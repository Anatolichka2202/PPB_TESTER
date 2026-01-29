#include "communicationengine.h"
#include <QMutex>
#include <QThread>
#include "../logger.h"

// Определения методов для Internal::StateManager
namespace Internal {

StateManager::StateManager(QObject* parent) : QObject(parent) {}

PPBState StateManager::getState(uint16_t address) const {
    QMutexLocker locker(&m_mutex);
    return m_states.value(address, PPBState::Idle);
}

void StateManager::setState(uint16_t address, PPBState state) {
    {
        QMutexLocker locker(&m_mutex);
        PPBState oldState = m_states.value(address, PPBState::Idle);
        if (oldState == state) return;
        m_states[address] = state;
    }
    emit stateChanged(address, state);
}

void StateManager::clear() {
    QMutexLocker locker(&m_mutex);
    m_states.clear();
}

CommandQueue::CommandQueue(QObject* parent) : QObject(parent) {}

CommandQueue::~CommandQueue() {
    clear(); // Очищаем все команды при уничтожении
}

void Internal::CommandQueue::enqueue(uint16_t address, std::unique_ptr<PPBCommand> cmd) {
    QMutexLocker locker(&m_mutex);
    m_queues[address].push_back(std::move(cmd));
    emit queueChanged(address, m_queues[address].size());
}

std::unique_ptr<PPBCommand> Internal::CommandQueue::dequeue(uint16_t address) {
    QMutexLocker locker(&m_mutex);

    auto it = m_queues.find(address);
    if (it != m_queues.end() && !it->second.empty()) {
        // Берем первый элемент из deque
        std::unique_ptr<PPBCommand> cmd = std::move(it->second.front());
        it->second.pop_front();

        if (it->second.empty()) {
            m_queues.erase(it);
        }

        emit queueChanged(address, m_queues.count(address) ? m_queues[address].size() : 0);
        return cmd;
    }

    return nullptr;
}
bool Internal::CommandQueue::isEmpty(uint16_t address) const {
    QMutexLocker locker(&m_mutex);
    auto it = m_queues.find(address);
    return it == m_queues.end() || it->second.empty();
}

void Internal::CommandQueue::clear() {
    QMutexLocker locker(&m_mutex);
    m_queues.clear();
}

QList<uint16_t> Internal::CommandQueue::addresses() const {
    QMutexLocker locker(&m_mutex);
    QList<uint16_t> keys;
    for (const auto& pair : m_queues) {
        keys.append(pair.first);
    }
    return keys;
}

} // namespace Internal

communicationengine::communicationengine(UDPClient* udpClient, QObject* parent)
    : QObject(parent)
    , m_udpClient(udpClient)
    , m_queueTimer(nullptr)
    , m_commandInterface(nullptr)
    , m_timeoutTimer(nullptr)
    , m_currentAddress(0)
    , m_currentPort(0)
    , m_stateManager(new Internal::StateManager(this))
    , m_commandQueue(new Internal::CommandQueue(this))
{

    LOG_INFO("communicationengine создан");

    if (m_udpClient) {
        connect(m_udpClient, &UDPClient::dataReceived,
                this, &communicationengine::onDataReceived, Qt::DirectConnection);
        connect(m_udpClient, &UDPClient::errorOccurred,
                this, &communicationengine::onNetworkError, Qt::DirectConnection);
    }

    m_queueTimer = new QTimer(this);
    m_queueTimer->setInterval(100);
    connect(m_queueTimer, &QTimer::timeout, this, &communicationengine::processCommandQueue);
    m_queueTimer->start();

}



communicationengine::~communicationengine() {
    m_queueTimer->stop();

    // Очищаем все контексты и их таймеры
    QMutexLocker locker(&m_contextsMutex);
    for (auto& pair : m_contexts) {
        PPBContext& context = pair.second;
        if (context.operationTimer) {
            context.operationTimer->stop();
            delete context.operationTimer;
        }
    }
    m_contexts.clear();
}


// Получаем контекст для адреса
communicationengine::PPBContext* communicationengine::getContext(uint16_t address) {
    QMutexLocker locker(&m_contextsMutex);
    return &m_contexts[address];  // std::unordered_map автоматически создает элемент
}

// Очищаем контекст
void communicationengine::clearContext(uint16_t address) {
    QMutexLocker locker(&m_contextsMutex);
    auto it = m_contexts.find(address);
    if (it != m_contexts.end()) {
        if (it->second.operationTimer) {
            it->second.operationTimer->stop();
            it->second.operationTimer->deleteLater();
        }
        m_contexts.erase(it);
    }
}

bool communicationengine::connectToPPB(uint16_t address, const QString& ip, quint16 port) {
    LOG_INFO(QString("communicationengine::connectToPPB: адрес=0x%1, IP=%2, порт=%3")
                 .arg(address, 4, 16, QChar('0'))
                 .arg(ip).arg(port));

    m_currentAddress = address;
    m_currentIP = ip;
    m_currentPort = port;

    auto tsCommand = CommandFactory::create(TechCommand::TS);
    if (!tsCommand) {
        emit errorOccurred("Не удалось создать команду TS");
        return false;
    }

    m_commandQueue->enqueue(address, std::move(tsCommand));
    return true;
}

void communicationengine::disconnect() {
    LOG_INFO("communicationengine::disconnect");
    m_commandQueue->clear();
    m_stateManager->clear();
    emit disconnected();
}

void communicationengine::executeCommand(TechCommand cmd, uint16_t address) {
    LOG_INFO(QString("communicationengine::executeCommand: команда=%1, адрес=%2")
                 .arg(static_cast<int>(cmd)).arg(address, 4, 16, QChar('0')));

    auto command = CommandFactory::create(cmd);
    if (!command) {
        emit errorOccurred(QString("Неизвестная команда: %1").arg(static_cast<int>(cmd)));
        return;
    }

    PPBState currentState = m_stateManager->getState(address);
    if (currentState != PPBState::Ready && currentState != PPBState::Idle) {
        m_commandQueue->enqueue(address, std::move(command));
        emit logMessage(QString("Команда %1 для адреса 0x%2 поставлена в очередь")
                            .arg(command->name())
                            .arg(address, 4, 16, QChar('0')));
    } else {
        executeCommandImmediately(address, std::move(command));
    }
}

void communicationengine::sendFUTransmit(uint16_t address) {
    QByteArray packet = PacketBuilder::createFUTransmitRequest(address);
    sendPacketInternal(packet, "ФУ передача");
}

void communicationengine::sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3]) {
    QByteArray packet;
    if (fuData) {
        packet = PacketBuilder::createFUReceiveRequest(address, period, fuData);
    } else {
        packet = PacketBuilder::createFUReceiveRequest(address, period);
    }
    sendPacketInternal(packet, "ФУ прием");
}

// ===== ПРИВАТНЫЕ МЕТОДЫ =====

void communicationengine::executeCommandImmediately(uint16_t address, std::unique_ptr<PPBCommand> command) {
    if (!command) return;

    PPBContext* context = getContext(address);
    if (!context) return;

    // Сохраняем команду
    context->currentCommand = std::move(command);
    context->waitingForOk = true;
    context->packetsExpected = context->currentCommand->expectedResponsePackets();
    context->packetsReceived = 0;
    context->receivedData.clear();

    // Обновляем состояние
    m_stateManager->setState(address, PPBState::SendingCommand);

    // Отправляем запрос
    QByteArray request = context->currentCommand->buildRequest(address);
    sendPacketInternal(request, context->currentCommand->name());

    // Создаем и настраиваем таймер
    if (context->operationTimer) {
        context->operationTimer->stop();
        delete context->operationTimer;
    }

    context->operationTimer = new QTimer(this);
    context->operationTimer->setSingleShot(true);

    // Используем лямбда-функцию вместо прямого подключения метода
    connect(context->operationTimer, &QTimer::timeout,
            this, [this, address]() {
                onOperationTimeout(address);
            });

    context->operationTimer->start(context->currentCommand->timeoutMs());

    emit logMessage(QString("Выполняется команда: %1 для адреса 0x%2")
                        .arg(context->currentCommand->name())
                        .arg(address, 4, 16, QChar('0')));
}

void communicationengine::processCommandQueue() {
    auto addresses = m_commandQueue->addresses();

    for (uint16_t address : addresses) {
        PPBState state = m_stateManager->getState(address);

        if ((state == PPBState::Ready || state == PPBState::Idle) &&
            !m_commandQueue->isEmpty(address)) {
            auto command = m_commandQueue->dequeue(address);
            if (command) {
                executeCommandImmediately(address, std::move(command));
            }
        }
    }
}

void communicationengine::onDataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port) {
    LOG_DEBUG(QString("communicationengine::onDataReceived: размер=%1, от=%2:%3")
                  .arg(data.size())
                  .arg(sender.toString())
                  .arg(port));

    // Определяем тип пакета по размеру
    if (data.size() == sizeof(PPBResponse)) {
        // Это ответ от ППБ
        PPBResponse response;
        if (PacketBuilder::parsePPBResponse(data, response)) {
            processPPBResponse(response);
        } else {
            LOG_WARNING("Не удалось распарсить ответ ППБ");
        }
    } else if (data.size() == sizeof(BridgeResponse)) {
        // Это ответ от бриджа
        BridgeResponse response;
        if (PacketBuilder::parseBridgeResponse(data, response)) {
            processBridgeResponse(response);
        } else {
            LOG_WARNING("Не удалось распарсить ответ бриджа");
        }
    } else if (data.size() == sizeof(DataPacket)) {
        // Это пакет данных
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(data, packet)) {
            processDataPacket(packet);
        } else {
            LOG_WARNING("Не удалось распарсить пакет данных");
        }
    } else {
        LOG_WARNING(QString("Неизвестный размер пакета: %1 байт").arg(data.size()));
    }
}

void communicationengine::onNetworkError(const QString& error) {
    emit errorOccurred(error);
}

void communicationengine::onOperationTimeout(uint16_t address) {
    PPBContext* context = getContext(address);
    if (context && context->currentCommand) {
        // Используем без перемещения
        context->currentCommand->onTimeout(m_commandInterface);
        clearContext(address);
    }
}

void communicationengine::sendPacketInternal(const QByteArray& packet, const QString& description) {
    if (!m_udpClient) {
        emit errorOccurred("UDPClient не инициализирован");
        return;
    }

    if (m_currentIP.isEmpty() || m_currentIP == "255.255.255.255") {
        m_udpClient->sendBroadcast(packet, m_currentPort);
    } else {
        m_udpClient->sendTo(packet, m_currentIP, m_currentPort);
    }

    LOG_INFO(QString("Отправлен пакет: %1").arg(description));
}

void communicationengine::processPPBResponse(const PPBResponse& response) {
    uint16_t address = response.address;

    QMutexLocker locker(&m_contextsMutex);

    // Ищем контекст по адресу
    auto it = m_contexts.find(address);
    if (it != m_contexts.end()) {
        PPBContext& context = it->second;

        if (context.waitingForOk && context.currentCommand) {
            if (response.status == 0x00) { // OK
                // Временно перемещаем команду для вызова метода
                std::unique_ptr<PPBCommand> tempCmd = std::move(context.currentCommand);
                tempCmd->onOkReceived(m_commandInterface, address);
                context.waitingForOk = false;
                // Возвращаем команду обратно
                context.currentCommand = std::move(tempCmd);
            } else {
                std::unique_ptr<PPBCommand> tempCmd = std::move(context.currentCommand);
                tempCmd->onTimeout(m_commandInterface);
                // Очищаем контекст при ошибке
                m_contexts.erase(it);
                // tempCmd будет уничтожен при выходе из области видимости
            }
        }
    }
}

void communicationengine::processBridgeResponse(const BridgeResponse& response) {
    LOG_DEBUG(QString("Ответ бриджа: адрес=%1, команда=0x%2, статус=%3")
                  .arg(response.address)
                  .arg(QString::number(response.command, 16))
                  .arg(response.status));

    // Просто логируем
    if (response.status == 1) {
        LOG_INFO("Бридж ответил OK");
        emit commandCompleted(true, "ФУ команда выполнена", TechCommand::TS);
    } else {
        LOG_WARNING("Бридж ответил с ошибкой");
        emit commandCompleted(false, "ФУ команда ошибка", TechCommand::TS);
    }

    // ФУ команды не меняют состояние
}

void communicationengine::processDataPacket(const DataPacket& packet) {
    // Конвертируем в QByteArray
    QByteArray packetData(reinterpret_cast<const char*>(&packet), sizeof(DataPacket));

    QMutexLocker locker(&m_contextsMutex);

    // Ищем контекст, ожидающий данных
    for (auto& pair : m_contexts) {
        PPBContext& context = pair.second;

        if (context.currentCommand && context.packetsExpected > 0) {
            context.receivedData.append(packetData);
            context.packetsReceived++;

            // Если получили все пакеты
            if (context.packetsReceived >= context.packetsExpected) {
                // Вызываем обработку в команде
                context.currentCommand->onDataReceived(m_commandInterface, context.receivedData);

                // Очищаем для следующей команды
                context.receivedData.clear();
                context.packetsReceived = 0;
                context.packetsExpected = 0;
            }
            break;
        }
    }
}
