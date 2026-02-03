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
    , m_activeDataAddress(0)
    , m_waitingForData(false)
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
     LOG_DEBUG(QString("Очистка контекста для адреса 0x%1").arg(address, 4, 16, QChar('0')));
    QMutexLocker locker(&m_contextsMutex);
    auto it = m_contexts.find(address);
    if (it != m_contexts.end()) {
        if (it->second.operationTimer) {
            LOG_DEBUG("Остановка таймера операции");
            it->second.operationTimer->stop();
            it->second.operationTimer->deleteLater();
        }
        m_contexts.erase(it);
          LOG_DEBUG("Контекст удален");
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

    // Сбрасываем активный диалог
    {
        QMutexLocker locker(&m_activeDataMutex);
        m_activeDataAddress = 0;
        m_waitingForData = false;
    }

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

void communicationengine::setCommandParseResult(uint16_t address, bool success, const QString& message) {
    PPBContext* context = getContext(address);
    if (!context) {
        LOG_WARNING(QString("setCommandParseResult: нет контекста для адреса 0x%1")
                        .arg(address, 4, 16, QChar('0')));
        return;
    }

    LOG_DEBUG(QString("setCommandParseResult для 0x%1: успех=%2, сообщение='%3'")
                  .arg(address, 4, 16, QChar('0'))
                  .arg(success)
                  .arg(message));

    context->parsedSuccess = success;
    context->parsedMessage = message;
}

void communicationengine::setCommandParseData(uint16_t address, const QVariant& data) {
    PPBContext* context = getContext(address);
    if (!context) {
        LOG_WARNING(QString("setCommandParseData: нет контекста для адреса 0x%1")
                        .arg(address, 4, 16, QChar('0')));
        return;
    }

    LOG_DEBUG(QString("setCommandParseData для 0x%1: тип данных=%2")
                  .arg(address, 4, 16, QChar('0'))
                  .arg(data.typeName()));

    context->parsedData = data;
}

// ===== ПРИВАТНЫЕ МЕТОДЫ =====

void communicationengine::executeCommandImmediately(uint16_t address, std::unique_ptr<PPBCommand> command) {
    if (!command) return;

    if (!canExecuteCommand(address, command.get())) {
        LOG_WARNING(QString("Не могу выполнить команду %1 для 0x%2: активен диалог с данными для 0x%3")
                        .arg(command->name())
                        .arg(address, 4, 16, QChar('0'))
                        .arg(m_activeDataAddress, 4, 16, QChar('0')));

        // Возвращаем команду в очередь
        m_commandQueue->enqueue(address, std::move(command));
        return;
    }


    // Проверяем, можем ли выполнить команду
    PPBState currentState = m_stateManager->getState(address);
    if (currentState != PPBState::Ready && currentState != PPBState::Idle) {
        LOG_WARNING(QString("Нельзя выполнить команду %1 для 0x%2: состояние %3")
                        .arg(command->name())
                        .arg(address, 4, 16, QChar('0'))
                        .arg(stateToString(currentState)));
        return;
    }

    // Создаём/очищаем контекст
    PPBContext* context = getContext(address);
    *context = PPBContext(); // Полная очистка

    // Настраиваем контекст
    context->currentCommand = std::move(command);
    context->waitingForOk = true;
    context->operationCompleted = false;
    context->packetsExpected = 0;
    context->packetsReceived = 0;
    context->receivedData.clear();

    // Переходим в состояние отправки команды
    transitionState(address, PPBState::SendingCommand,
                    QString("Начинаем %1").arg(context->currentCommand->name()));

    // Отправляем запрос
    QByteArray request = context->currentCommand->buildRequest(address);
    sendPacketInternal(request, context->currentCommand->name());

    // Настраиваем таймер
    context->operationTimer = new QTimer(this);
    context->operationTimer->setSingleShot(true);
    connect(context->operationTimer, &QTimer::timeout,
            this, [this, address]() { onOperationTimeout(address); });

    context->operationTimer->start(context->currentCommand->timeoutMs());

    LOG_INFO(QString("Выполняется %1 для 0x%2 (таймаут: %3 мс)")
                 .arg(context->currentCommand->name())
                 .arg(address, 4, 16, QChar('0'))
                 .arg(context->currentCommand->timeoutMs()));
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

    QString hexData = data.toHex(' ').toUpper();
    LOG_DEBUG(QString("Данные: %1").arg(hexData));

    // Проверяем, находимся ли мы в состоянии ожидания данных
    bool waitingForData = false;
    uint16_t activeAddress = 0;
    {
        QMutexLocker locker(&m_activeDataMutex);
        waitingForData = m_waitingForData;
        activeAddress = m_activeDataAddress;
    }

    // Определяем тип пакета по размеру и состоянию
    if (waitingForData && data.size() == sizeof(DataPacket)) {
        // В состоянии ожидания данных пробуем сначала разобрать как DataPacket
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(data, packet)) {
            LOG_DEBUG(QString("Пакет данных в состоянии ожидания: counter=%1")
                          .arg(packet.counter));
            processDataPacket(packet);
            return;
        }
    }

    // Обычная логика определения типа пакета
    if (data.size() == sizeof(PPBResponse)) {
        // Это ответ от ППБ
        PPBResponse response;
        if (PacketBuilder::parsePPBResponse(data, response)) {
            LOG_DEBUG(QString("Ответ ППБ: адрес=0x%1, статус=0x%2")
                          .arg(response.address, 4, 16, QChar('0'))
                          .arg(response.status, 2, 16, QChar('0')));

            processPPBResponse(response);
        } else {
            LOG_WARNING("Не удалось распарсить ответ ППБ");
        }
    } else if (data.size() == sizeof(BridgeResponse)) {
        // Это ответ от бриджа
        BridgeResponse response;
        if (PacketBuilder::parseBridgeResponse(data, response)) {
            LOG_DEBUG(QString("Ответ бриджа: адрес=0x%1, статус=%2")
                          .arg(response.address)
                          .arg(response.status));

            processBridgeResponse(response);
        } else {
            LOG_WARNING("Не удалось распарсить ответ бриджа");
        }
    } else if (data.size() == sizeof(DataPacket)) {
        // Это пакет данных (но не в состоянии ожидания)
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(data, packet)) {
            LOG_DEBUG(QString("Пакет данных вне состояния ожидания: counter=%1")
                          .arg(packet.counter));
            // Если не в состоянии ожидания, игнорируем (или обрабатываем иначе)
            LOG_WARNING("Получен пакет данных вне состояния ожидания");
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
    if (!context || !context->currentCommand) {
        // Нет контекста → просто переходим в Ready
        transitionState(address, PPBState::Ready, "Таймаут без активной команды");

        {
            QMutexLocker locker(&m_activeDataMutex);
            if (address == m_activeDataAddress) {
                m_activeDataAddress = 0;
                m_waitingForData = false;
            }
        }

        return;
    }

    LOG_WARNING(QString("Таймаут для %1 (0x%2)")
                    .arg(context->currentCommand->name())
                    .arg(address, 4, 16, QChar('0')));

    {
        QMutexLocker locker(&m_activeDataMutex);
        if (address == m_activeDataAddress) {
            m_activeDataAddress = 0;
            m_waitingForData = false;
        }
    }
    // Проверяем, получили ли мы часть данных
    if (context->packetsReceived > 0) {
        // Были получены частичные данные - даем команде их обработать
        QString partialMessage = QString("Таймаут операции. Получено %1 из %2 пакетов")
                                     .arg(context->packetsReceived)
                                     .arg(context->packetsExpected);

        // Вызываем обработку частичных данных
        if (!context->receivedData.isEmpty() && m_commandInterface) {
            try {
                context->currentCommand->onDataReceived(m_commandInterface, context->receivedData);
            } catch (...) {
                // Игнорируем исключения при обработке частичных данных
            }
        }

        completeOperation(address, false, partialMessage);
    } else {
        // Данных не было совсем
        completeOperation(address, false, "Таймаут операции");
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
    PPBContext* context = getContext(address);

    if (!context || !context->currentCommand) {
        LOG_WARNING(QString("Ответ от неизвестного адреса 0x%1").arg(address, 4, 16, QChar('0')));
        return;
    }

    // Если операция уже завершена, игнорируем
    if (context->operationCompleted) {
        LOG_DEBUG("Операция уже завершена, игнорируем ответ");
        return;
    }

    if (response.status == 0x00) { // OK
        LOG_INFO(QString("OK от 0x%1 для %2")
                     .arg(address, 4, 16, QChar('0'))
                     .arg(context->currentCommand->name()));

        // Вызываем логику команды для обработки OK
        context->currentCommand->onOkReceived(m_commandInterface, address);

        // Если команда не ожидает данных, завершаем операцию
        if (context->currentCommand->expectedResponsePackets() == 0) {
            completeOperation(address, true, "Команда выполнена");
        } else {
            // Команда ожидает данные - устанавливаем глобальный активный диалог
            {
                QMutexLocker locker(&m_activeDataMutex);
                m_activeDataAddress = address;
                m_waitingForData = true;
            }

            LOG_INFO(QString("Установлен активный диалог с данными для адреса 0x%1")
                         .arg(address, 4, 16, QChar('0')));

            // Команда ожидает данные
            context->waitingForOk = false;
            context->packetsExpected = context->currentCommand->expectedResponsePackets();
            context->packetsReceived = 0;
            context->receivedData.clear();

            // Переходим в состояние ожидания данных
            transitionState(address, PPBState::WaitingData,
                            QString("Ожидание %1 пакетов").arg(context->packetsExpected));

            // Перезапускаем таймер на время ожидания данных
            if (context->operationTimer) {
                context->operationTimer->start(context->currentCommand->timeoutMs());
            }
        }
    } else { // ОШИБКА от ППБ
        LOG_ERROR(QString("Ошибка ППБ 0x%1: статус=0x%2")
                      .arg(address, 4, 16, QChar('0'))
                      .arg(response.status, 2, 16, QChar('0')));

        // Если это был активный диалог - сбрасываем
        {
            QMutexLocker locker(&m_activeDataMutex);
            if (address == m_activeDataAddress) {
                m_activeDataAddress = 0;
                m_waitingForData = false;
            }
        }

        // Завершаем с ошибкой
        completeOperation(address, false,
                          QString("Ошибка ППБ: 0x%1").arg(response.status, 2, 16, QChar('0')));
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
    QByteArray packetData(reinterpret_cast<const char*>(&packet), sizeof(DataPacket));

    QMutexLocker dataLocker(&m_activeDataMutex);

    // Если нет активного диалога с данными - игнорируем пакет
    if (!m_waitingForData || m_activeDataAddress == 0) {
        LOG_DEBUG("Получен пакет данных, но нет активного диалога, игнорируем");
        return;
    }

    uint16_t activeAddress = m_activeDataAddress;
    dataLocker.unlock(); // Разблокируем мьютекс

    // Проверяем, что контекст все еще существует
    {
        QMutexLocker locker(&m_contextsMutex);
        if (m_contexts.find(activeAddress) == m_contexts.end()) {
            LOG_WARNING(QString("Контекст для адреса 0x%1 уже удален, игнорируем пакет")
                            .arg(activeAddress, 4, 16, QChar('0')));
            return;
        }
    }

    // Получаем контекст для активного адреса
    PPBContext* context = getContext(activeAddress);
    if (!context || !context->currentCommand || context->operationCompleted) {
        LOG_WARNING(QString("Получен пакет данных для адреса 0x%1, но нет активной команды")
                        .arg(activeAddress, 4, 16, QChar('0')));
        return;
    }

    // Проверяем, что команда действительно ожидает данные
    if (context->packetsExpected == 0) {
        LOG_WARNING(QString("Получен пакет данных для адреса 0x%1, но команда не ожидает данные")
                        .arg(activeAddress, 4, 16, QChar('0')));
        return;
    }

    // Проверяем, что операция еще не завершена
    if (context->operationCompleted) {
        LOG_DEBUG(QString("Операция для адреса 0x%1 уже завершена, игнорируем пакет")
                      .arg(activeAddress, 4, 16, QChar('0')));
        return;
    }

    // Сохраняем пакет
    context->receivedData.append(packetData);
    context->packetsReceived++;

    LOG_DEBUG(QString("Пакет %1/%2 для активного адреса 0x%3")
                  .arg(context->packetsReceived)
                  .arg(context->packetsExpected)
                  .arg(activeAddress, 4, 16, QChar('0')));

    // Если получили все пакеты
    if (context->packetsReceived >= context->packetsExpected) {
        // Сбрасываем активный диалог
        {
            QMutexLocker locker(&m_activeDataMutex);
            m_activeDataAddress = 0;
            m_waitingForData = false;
        }

        completeOperation(activeAddress, true,
                          QString("Получены все %1 пакетов").arg(context->packetsExpected));
    }
}

// +++++++++++++++++++++++++++++++++++++++++++++++++ МАШИНА СОСТОЯНИЯ +++++++++++++++++++++++++++++++++++++
QString communicationengine::stateToString(PPBState state)  const {
    switch (state) {
    case PPBState::Idle: return "Idle";
    case PPBState::Ready: return "Ready";
    case PPBState::SendingCommand: return "SendingCommand";
    case PPBState::WaitingData: return "WaitingData";
    default: return "Unknown";
    }
}

void communicationengine::transitionState(uint16_t address, PPBState newState, const QString& reason) {
    // Получаем текущее состояние
    PPBState oldState = m_stateManager->getState(address);

    // Если состояние не изменилось - выходим
    if (oldState == newState) {
        LOG_DEBUG(QString("Состояние не изменилось для 0x%1: %2 [%3]")
                      .arg(address, 4, 16, QChar('0'))
                      .arg(stateToString(oldState))
                      .arg(reason));
        return;
    }

    // Логируем переход
    LOG_INFO(QString("Переход состояния для 0x%1: %2 -> %3 [%4]")
                 .arg(address, 4, 16, QChar('0'))
                 .arg(stateToString(oldState))
                 .arg(stateToString(newState))
                 .arg(reason));

    // Дополнительные действия при определенных переходах
    switch (newState) {
    case PPBState::Idle:
        // При переходе в Idle очищаем контекст
        clearContext(address);
        break;

    case PPBState::Ready:
        // При готовности проверяем очередь команд
        QTimer::singleShot(0, this, [this, address]() {
            processNextCommandForAddress(address);
        });
        break;

    default:
        break;
    }

    // Устанавливаем новое состояние
    m_stateManager->setState(address, newState);

    // Отправляем сигнал (если нужно)
    if (address == m_currentAddress) {
        emit stateChanged(address, newState);
    }
}

void communicationengine::completeOperation(uint16_t address, bool success, const QString& message) {
    // Получаем контекст
    PPBContext* context = getContext(address);
    if (!context) {
        LOG_WARNING(QString("Нет контекста для завершения операции для 0x%1")
                        .arg(address, 4, 16, QChar('0')));
        return;
    }

    // Проверяем, не завершена ли уже операция
    if (context->operationCompleted) {
        LOG_DEBUG(QString("Операция уже завершена для 0x%1, игнорируем")
                      .arg(address, 4, 16, QChar('0')));
        return;
    }

    // Устанавливаем флаг завершения
    context->operationCompleted = true;

    // Останавливаем таймер
    if (context->operationTimer) {
        context->operationTimer->stop();
    }

    // Сбрасываем активный диалог, если это наш адрес
    {
        QMutexLocker locker(&m_activeDataMutex);
        if (address == m_activeDataAddress) {
            m_activeDataAddress = 0;
            m_waitingForData = false;
            LOG_DEBUG(QString("Сброшен активный диалог для адреса 0x%1").arg(address, 4, 16, QChar('0')));
        }
    }

    // =====  ЛОГИКА: ИСПОЛЬЗОВАНИЕ РЕЗУЛЬТАТОВ ПАРСИНГА =====
    QString finalMessage = message;
    bool finalSuccess = success;

    // Если команда предоставила результат парсинга - используем его
    if (!context->parsedMessage.isEmpty()) {
        finalMessage = context->parsedMessage;
        finalSuccess = context->parsedSuccess && success; // Учитываем и успех операции, и успех парсинга

        LOG_DEBUG(QString("Используем результат парсинга команды: успех=%1, сообщение='%2'")
                      .arg(context->parsedSuccess)
                      .arg(context->parsedMessage));
    }

    // Логируем завершение
    LOG_INFO(QString("Завершение операции для 0x%1: %2 - %3")
                 .arg(address, 4, 16, QChar('0'))
                 .arg(finalSuccess ? "УСПЕХ" : "ОШИБКА")
                 .arg(finalMessage));

    // ===== ВЫЗОВ ОБРАБОТКИ ДАННЫХ КОМАНДОЙ =====
    // Если есть необработанные данные и команда - обрабатываем их
    // Обратите внимание: теперь команда в onDataReceived сама устанавливает
    // результаты парсинга через CommandInterface
    if (!context->receivedData.isEmpty() && context->currentCommand && m_commandInterface) {
        LOG_DEBUG(QString("Вызываем onDataReceived команды для обработки %1 пакетов")
                      .arg(context->receivedData.size()));
        if (m_commandInterface) {
            try {
        context->currentCommand->onDataReceived(m_commandInterface, context->receivedData);
            } catch (const std::exception& e) {
                LOG_ERROR(QString("Исключение в onDataReceived: %1").arg(e.what()));
            } catch (...) {
                LOG_ERROR("Неизвестное исключение в onDataReceived");
            }
        }
    }

    // ===== ОПРЕДЕЛЕНИЕ СЛЕДУЮЩЕГО СОСТОЯНИЯ =====
    PPBState nextState;

    if (finalSuccess) {
        // Успех -> Ready
        nextState = PPBState::Ready;
    } else {
        // Ошибка операции
        if (context->currentCommand) {
            // Различаем тип команды
            switch (context->currentCommand->commandId()) {
            case TechCommand::TS:
                // Ошибка TS (подключения) -> Idle
                nextState = PPBState::Idle;
                break;

            case TechCommand::VERS:
            case TechCommand::CHECKSUM:
            case TechCommand::DROP:
            case TechCommand::BER_T:
            case TechCommand::BER_F:
            case TechCommand::PRBS_S2M:
                // Ошибки команд с данными -> остаемся Ready
                // (соединение может быть живым, просто не получили данные)
                nextState = PPBState::Ready;
                break;

            default:
                // Для остальных команд - по умолчанию Ready
                nextState = PPBState::Ready;
                break;
            }
        } else {
            // Нет команды -> Idle
            nextState = PPBState::Idle;
        }
    }

    // ===== СПЕЦИАЛЬНАЯ ОБРАБОТКА ДЛЯ TS =====
    if (success && context->currentCommand &&
        context->currentCommand->commandId() == TechCommand::TS) {
        emit connected();
    }

    if (!success && context->currentCommand &&
        context->currentCommand->commandId() == TechCommand::TS) {
        emit errorOccurred(QString("Ошибка подключения: %1").arg(finalMessage));
    }

    // ===== ОТПРАВКА СИГНАЛОВ =====
    // Отправляем сигнал о завершении команды
    if (context->currentCommand) {
        emit commandCompleted(finalSuccess, finalMessage, context->currentCommand->commandId());
    } else {
        emit commandCompleted(finalSuccess, finalMessage, TechCommand::TS); // Дефолтная команда
    }

    // Отправляем сигнал с распарсенными данными, если они есть
    if (context->parsedData.isValid()) {
        emit commandDataParsed(address, context->parsedData, context->currentCommand ?
                                                                 context->currentCommand->commandId() : TechCommand::TS);
    }

    // ===== ПЕРЕХОД В НОВОЕ СОСТОЯНИЕ =====
    transitionState(address, nextState,
                    QString("Завершение операции: %1").arg(finalMessage));

    // ===== ОЧИСТКА КОНТЕКСТА =====
    // Не очищаем контекст полностью, только поля парсинга для следующей операции
    context->clearParseResults();
}

void communicationengine::processNextCommandForAddress(uint16_t address) {
    // Проверяем, что мы в состоянии Ready
    if (m_stateManager->getState(address) != PPBState::Ready) {
        return;
    }

    // Проверяем, есть ли команды в очереди
    if (m_commandQueue->isEmpty(address)) {
        return;
    }

    // Берем следующую команду из очереди
    auto command = m_commandQueue->dequeue(address);
    if (!command) {
        return;
    }

    LOG_INFO(QString("Берем команду из очереди для 0x%1: %2")
                 .arg(address, 4, 16, QChar('0'))
                 .arg(command->name()));

    // Выполняем команду немедленно
    executeCommandImmediately(address, std::move(command));
}

bool communicationengine::canExecuteCommand(uint16_t address, const PPBCommand* command) const {
    QMutexLocker locker(&m_activeDataMutex);

    // Если команда НЕ ожидает данных — можно выполнять всегда
    if (command->expectedResponsePackets() == 0) {
        return true;
    }

    // Если команда ожидает данных:
    // 1. Не должно быть активного диалога с данными
    // 2. Или если есть активный диалог, то это должен быть тот же адрес
    //    (хотя это рискованно — данные могут перемешаться)

    if (!m_waitingForData) {
        return true; // Нет активного диалога — можно
    }

    // Есть активный диалог — можно только если тот же адрес
    // (Но лучше запретить вообще, чтобы не перемешивать данные)
    return false; // Пока запрещаем все параллельные диалоги с данными
}
