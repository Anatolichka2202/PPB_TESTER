#include "ppbcommunication.h"
#include <QHostAddress>
#include "ppbprotocol.h"
#include "crc.h"
#include "commandandoperation.h"
#include <QThread>


// ===== РЕАЛИЗАЦИЯ =====
PPBCommunication::PPBCommunication(QObject* parent)
    : CommandInterface(parent)
    , m_state(PPBState::Idle)
    , m_currentAddress(0)
    , m_currentPort(0)
    , m_expectedPackets(0)
    , m_receivedPacketCount(0)
    , m_waitingForOk(false)
    , m_currentPacketIndex(0)
    , m_timeoutTimer(nullptr)  // Явная инициализация
    , m_packetTimer(nullptr)   // Явная инициализация
    , m_udpClient(nullptr)
    , m_processingTask(false)
{
     LOG_INFO("PPBCommunication конструктор вызван");
}

PPBCommunication::~PPBCommunication()

{
    LOG_INFO("~PPBCommunication: начало");

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
        delete m_timeoutTimer;
        m_timeoutTimer = nullptr;
    }

    if (m_packetTimer) {
        m_packetTimer->stop();
        delete m_packetTimer;
        m_packetTimer = nullptr;
    }

    LOG_INFO("~PPBCommunication: завершение");
}
void PPBCommunication::initializeInThread()
{
    // Создаем таймеры (они должны создаваться в потоке объекта)
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &PPBCommunication::onOperationTimeout);

    m_packetTimer = new QTimer(this);
    m_packetTimer->setSingleShot(false);
    connect(m_packetTimer, &QTimer::timeout, this, &PPBCommunication::sendNextPacket);

    LOG_INFO(QString("PPBCommunication инициализирован в потоке: %1")
                 .arg((qulonglong)QThread::currentThreadId()));
}
void PPBCommunication::setUDPClient(UDPClient* udpClient)
{
    // Проверяем, что метод вызывается в правильном потоке
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "setUDPClient",
                                  Qt::QueuedConnection,
                                  Q_ARG(UDPClient*, udpClient));
        return;
    }

    if (m_udpClient == udpClient) {
        return;
    }

    // Отключаем старый UDPClient
    if (m_udpClient) {
        m_udpClient->disconnect(this);
        LOG_INFO("Отключен старый UDPClient");
    }

    m_udpClient = udpClient;

    if (m_udpClient) {
        // Подключаем сигналы нового UDPClient
        connect(m_udpClient, &UDPClient::dataReceived,
                this, &PPBCommunication::onDataReceived, Qt::QueuedConnection);
        connect(m_udpClient, &UDPClient::errorOccurred,
                this, &PPBCommunication::onNetworkError, Qt::QueuedConnection);

        LOG_INFO(QString("UDPClient установлен для PPBCommunication (порт: %1)")
                     .arg(m_udpClient->boundPort()));
    } else {
        LOG_WARNING("Передан nullptr для UDPClient");
    }
}

// ===== ПУБЛИЧНЫЕ МЕТОДЫ =====

bool PPBCommunication::connectToPPB(uint16_t address, const QString& ip, quint16 port)
{
    LOG_INFO(QString("PPBCommunication::connectToPPB: address=0x%1, ip=%2, port=%3, thread=%4")
                 .arg(address, 4, 16, QChar('0'))
                 .arg(ip).arg(port)
                 .arg((qulonglong)QThread::currentThreadId()));
    // Проверяем, что мы в правильном потоке
    if (QThread::currentThread() != this->thread()) {
        LOG_WARNING("connectToPPB вызван из другого потока");
    }

    m_currentAddress = address;
    m_currentIP = ip;
    m_currentPort = port;
LOG_INFO(QString("Параметры установлены: IP=%1, Port=%2").arg(ip).arg(port));
    // Отправляем TS для проверки связи
    executeCommand(TechCommand::TS, address);

    return true;
}

void PPBCommunication::disconnect()
{
    // Сбрасываем всё
    stopTimeoutTimer();
    setState(PPBState::Idle);
    m_currentAddress = 0;
    m_currentIP.clear();
    m_currentPort = 0;
    m_waitingForOk = false;
    m_currentCommand.reset();

    LOG_INFO("Отключено");
     emit disconnected(); //  уведомляем об отключении
}

// ====================================================МЕТОДЫ КОМАНД====================================================

void PPBCommunication::executeCommand(TechCommand cmd, uint16_t address)
{
    LOG_INFO(QString("PPBCommunication::executeCommand: cmd=%1, address=0x%2, thread=%3")
                 .arg(static_cast<int>(cmd))
                 .arg(address, 4, 16, QChar('0'))
                 .arg((qulonglong)QThread::currentThreadId()));

    emit busyChange(true);
    // Проверяем поток
    if (QThread::currentThread() != this->thread()) {
        LOG_ERROR("executeCommand вызван из другого потока!");
        return;
    }

    LOG_INFO(QString("Запрос на выполнение команды: %1").arg(static_cast<int>(cmd)));

    if (isBusy()) {
           LOG_WARNING("Уже выполняется другая операция");
        setError("Уже выполняется другая операция");
        return;
    }

    m_currentCommand = CommandFactory::create(cmd);//создаём команду
    if (!m_currentCommand) {
          LOG_ERROR("Неизвестная команда");
        setError("Неизвестная команда");
        return;
    }
 LOG_INFO(QString("Команда создана: %1").arg(m_currentCommand->name()));
    m_expectedPackets = m_currentCommand->expectedResponsePackets();//сохраняем изменения
    m_receivedPacketCount = 0;

    QByteArray packet = m_currentCommand->buildRequest(address);
    sendPacket(packet, m_currentCommand->name());

    setState(PPBState::SendingCommand);
    m_waitingForOk = true;
    startTimeoutTimer(m_currentCommand->timeoutMs());
     LOG_INFO("Команда отправлена, ожидание ответа...");
}



void PPBCommunication::sendFUTransmit(uint16_t address)
{
    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }

    // Создаем пакет ФУ передачи
    QByteArray packet = PacketBuilder::createFUTransmitRequest(address);

    // Отправляем пакет
    sendPacket(packet, "ФУ передача");

    LOG_INFO(QString("Отправлен запрос ФУ передача для адреса %1").arg(address));
}

void PPBCommunication::sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3])
{
    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }

    // Создаем пакет ФУ приема
    QByteArray packet;
    if (fuData) {
        packet = PacketBuilder::createFUReceiveRequest(address, period, fuData);
    } else {
        packet = PacketBuilder::createFUReceiveRequest(address, period);
    }

    // Отправляем пакет
    sendPacket(packet, "ФУ прием");

    LOG_INFO(QString("Отправлен запрос ФУ прием для адреса %1, период %2").arg(address).arg(period));
}

// ===== СЛОТЫ =====

void PPBCommunication::onDataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port)
{
    LOG_DEBUG(QString("PPBCommunication: Получены данные от %1:%2, размер: %3 байт")
                  .arg(sender.toString()).arg(port).arg(data.size()));

    // Логируем
    QString hex;
    for (int i = 0; i < data.size() && i < 8; ++i) {
        hex += QString::number(static_cast<uint8_t>(data[i]), 16).right(2) + " ";
    }
    LOG_DEBUG(QString("Получено %1 байт - 0x%2").arg(data.size()).arg(hex));

    if (data.size() != 4) {
        LOG_WARNING("Неверный размер пакета");
        return;
    }

    // Если ждем данные (статус или тестовые)
    if (m_state == PPBState::WaitingData) {
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(data, packet)) {
            processDataPacket(packet);
        } else {
            LOG_WARNING("Не удалось распарсить пакет данных");
        }
        return;
    }

    // Если ждем OK ответ
    if (m_waitingForOk) {
        PPBResponse response;
        if (PacketBuilder::parsePPBResponse(data, response)) {
            processPPBResponse(response);
            return;
        }

        BridgeResponse bridgeResponse;
        if (PacketBuilder::parseBridgeResponse(data, bridgeResponse)) {
            processBridgeResponse(bridgeResponse);
            return;
        }

        LOG_WARNING("Не удалось распарсить ответ");
        return;
    }

    // Если ничего не ждем, но пришел пакет - игнорируем
    LOG_WARNING("Получен неожиданный пакет");
}

void PPBCommunication::onNetworkError(const QString& error)
{
    setError(QString("Сетевая ошибка: %1").arg(error));
    completeOperation(false, error);
}

void PPBCommunication::onOperationTimeout()
{
    LOG_WARNING("Таймаут операции");

    if (m_currentCommand) {
        // Уведомляем команду о таймауте
        m_currentCommand->onTimeout(this);
    } else {
        setError("Таймаут операции");
        completeOperation(false, "Таймаут операции");
    }
}
void PPBCommunication::completeCurrentOperation(bool success, const QString& message)
{
    // Останавливаем таймер
    stopTimeoutTimer();

    // Определяем, какая команда выполнялась
    TechCommand cmd = m_currentCommand ? m_currentCommand->commandId() : TechCommand::TS;

    // Сбрасываем состояние команды
    m_currentCommand.reset();
    m_waitingForOk = false;
    m_receivedPacketCount = 0;
    m_expectedPackets = 0;

    // Устанавливаем соответствующее состояние
    if (success) {
        // Успешная команда TS переводит в состояние Ready
        if (cmd == TechCommand::TS) {
            setState(PPBState::Ready);
            emit connected();  // Уведомляем о подключении
        } else {
            // Другие успешные команды возвращают в Ready
            setState(PPBState::Ready);
        }
    } else {
        // Ошибка - возвращаемся в Idle
        setState(PPBState::Idle);
    }

    // Эмитируем сигнал о завершении команды
    emit commandCompleted(success, message, cmd);
    emit busyChange(false);
    QTimer::singleShot(0, this, &PPBCommunication::processNextTask);
}

void PPBCommunication::initialize(UDPClient* udpClient)
{
    LOG_INFO("PPBCommunication::initialize начат в потоке: " +
             QString::number((qulonglong)QThread::currentThreadId()));

    // Этот метод должен выполняться в потоке объекта!
    if (QThread::currentThread() != this->thread()) {
        LOG_WARNING("PPBCommunication::initialize: вызван не из своего потока");
        QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection,
                                  Q_ARG(UDPClient*, udpClient));
        return;
    }

    try {
        LOG_INFO("PPBCommunication::initialize: создание таймеров...");

        // 1. Создаем таймеры БЕЗ родителя
        m_timeoutTimer = new QTimer();
        m_timeoutTimer->setSingleShot(true);

        m_packetTimer = new QTimer();
        m_packetTimer->setSingleShot(false);

        LOG_INFO("Таймеры созданы");

        // Подключаем сигналы с Qt::DirectConnection (так как объекты в одном потоке)
        LOG_INFO("Подключение сигналов таймеров...");
        connect(m_timeoutTimer, &QTimer::timeout, this, &PPBCommunication::onOperationTimeout, Qt::DirectConnection);
        connect(m_packetTimer, &QTimer::timeout, this, &PPBCommunication::sendNextPacket, Qt::DirectConnection);
        LOG_INFO("Сигналы таймеров подключены");

        // 2. Устанавливаем UDPClient
        LOG_INFO("Установка UDPClient...");
        setUDPClient(udpClient);

        LOG_INFO("PPBCommunication::initialize: инициализация завершена успешно");
        emit initialized();

    } catch (const std::exception& e) {
        LOG_ERROR("PPBCommunication::initialize: исключение: " + QString(e.what()));
        emit errorOccurred(QString("Ошибка инициализации: %1").arg(e.what()));
    } catch (...) {
        LOG_ERROR("PPBCommunication::initialize: неизвестное исключение");
        emit errorOccurred("Неизвестная ошибка инициализации");
    }
}

void PPBCommunication::stop()
{
    LOG_INFO("PPBCommunication::stop() - остановка");

    // Останавливаем все таймеры
    if (m_timeoutTimer && m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }

    if (m_packetTimer && m_packetTimer->isActive()) {
        m_packetTimer->stop();
    }

    // Очищаем очереди
    m_taskQueue.clear();
    m_packetsQueue.clear();
    m_receivedData.clear();

    // Отключаемся от UDPClient
    if (m_udpClient) {
        CommandInterface::disconnect(m_udpClient, nullptr, this, nullptr);
    }

    // Сбрасываем состояние
    setState(PPBState::Idle);
    m_currentCommand.reset();

    LOG_INFO("PPBCommunication остановлен");
}

// ===== ПРИВАТНЫЕ МЕТОДЫ =====

void PPBCommunication::setState(PPBState state)
{
    QMutexLocker locker(&m_stateMutex);
    if (m_state == state) return;

    m_state = state;
    emit stateChanged(state);

    LOG_DEBUG(QString("Состояние изменено: %1").arg(static_cast<int>(state)));

    // При переходе в Ready уведомляем о подключении
    if (state == PPBState::Ready) {
        emit connected();
    }



}

void PPBCommunication::setError(const QString& error)
{
    m_lastError = error;
    LOG_ERROR(error);
    emit errorOccurred(error);
}

void PPBCommunication::completeOperation(bool success, const QString& message)
{
    stopTimeoutTimer();

    TechCommand cmd = m_currentCommand ? m_currentCommand->commandId() : TechCommand::TS;

    // Сбрасываем команду
    m_currentCommand.reset();
    m_waitingForOk = false;
    m_receivedPacketCount = 0;

    // Возвращаемся в Ready, если была связь
    if(success && cmd == TechCommand::TS){
        setState(PPBState::Ready);
    }else{
        setState(PPBState::Idle);

    }

    emit commandCompleted(success, message, cmd);
}



// Обработка входящих данных
void PPBCommunication::processDataPacket(const DataPacket& packet)
{
    LOG_DEBUG(QString("Пакет данных #%1: [0x%2 0x%3] CRC=0x%4")
                  .arg(packet.counter)
                  .arg(packet.data[0], 2, 16, QChar('0'))
                  .arg(packet.data[1], 2, 16, QChar('0'))
                  .arg(packet.crc, 2, 16, QChar('0')));
    LOG_INFO("Обработка пакета данных");

    // Проверяем CRC
    if (!PacketBuilder::checkDataPacketCRC(packet)) {
        LOG_WARNING("Ошибка CRC в пакете данных");
    }

    if (packet.counter != m_receivedPacketCount) {
        LOG_WARNING(QString("Неверный порядок пакетов: ожидался #%1, получен #%2")
                        .arg(m_receivedPacketCount).arg(packet.counter));
    }



    // Сохраняем
    QByteArray packetBytes(reinterpret_cast<const char*>(&packet), sizeof(DataPacket));
    m_receivedData.append(packetBytes);
    m_receivedPacketCount++;

    // Прогресс
    emit commandProgress(m_receivedPacketCount, m_expectedPackets,
                         m_currentCommand ? m_currentCommand->commandId() : TechCommand::TS);

    // Все ли пакеты получены?
    if (m_receivedPacketCount >= m_expectedPackets) {
        // Команда завершена
        if (m_currentCommand) {
            m_currentCommand->onDataReceived(this, m_receivedData);
            m_receivedData.clear();
                            }
    }
    else {
        // Ждем следующий пакет
        startTimeoutTimer(PPBConstants::PACKET_TIMEOUT_MS);
        }

}
void PPBCommunication::processBridgeResponse(const BridgeResponse& response)
{
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

// ===== ОБРАБОТКА КОМАНД =====

void PPBCommunication::processPPBResponse(const PPBResponse& response)
{
    if (!m_waitingForOk) {
        LOG_WARNING("Неожиданный ответ ППБ");
        return;
    }

    stopTimeoutTimer();
    m_waitingForOk = false;

    if (response.status == 0x00) { // OK
        LOG_INFO("Получен OK от ППБ");

        // Проверяем, какая команда
        if (!m_currentCommand) {
            setError("Нет текущей команды");
            return;
        }

        // Действия после OK определяет сама команда
         m_currentCommand->onOkReceived(this, m_currentAddress);

    } else {
        // Ошибка от ППБ
        LOG_ERROR("ППБ ответил с ошибкой");
        completeOperation(false, "ППБ ответил с ошибкой");
    }
}

// ===== УПРАВЛЕНИЕ ТАЙМЕРАМИ =====

void PPBCommunication::startTimeoutTimer(int ms)
{
    if (QThread::currentThread() != this->thread()) {
        // Если вызвано из другого потока - ставим в очередь
        QMetaObject::invokeMethod(this, "startTimeoutTimer",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, ms));
        return;
    }
    m_timeoutTimer->start(ms);
}

void PPBCommunication::stopTimeoutTimer()
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "stopTimeoutTimer",
                                  Qt::QueuedConnection);
        return;
    }
    m_timeoutTimer->stop();
}

//======================Отправка

void PPBCommunication::sendDataPackets(const QVector<DataPacket>& packets)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "sendDataPackets",
                                  Qt::QueuedConnection,
                                  Q_ARG(QVector<DataPacket>, packets));
        return;
    }
    static bool isSending = false;
    if (isSending) {
        LOG_WARNING("Отправка уже выполняется");
        return;
    }

    isSending = true;

    try{
    if (packets.isEmpty()) {
        LOG_WARNING("Пустой вектор пакетов для отправки");
        return;
    }

    if (m_packetTimer->isActive()) {
        LOG_WARNING("Отправка пакетов уже выполняется");
        return;
    }

    LOG_INFO(QString("Начало отправки %1 тестовых пакетов").arg(packets.size()));

    // Сохраняем для будущего сравнения
    m_generatedPackets = packets;

    // Инициализируем очередь отправки
    m_packetsQueue = packets;
    m_currentPacketIndex = 0;

    // Запускаем таймер для отправки пакетов
    m_packetTimer->start(PPBConstants::PACKET_INTERVAL_MS);

    // Немедленно отправляем первый пакет
    QTimer::singleShot(0, this, &PPBCommunication::sendNextPacket);
    } catch(...)
    {
        isSending = false;
        throw;
    }
    isSending = false;
}                //  ОТПРАВКА ПАКЕТОВ(ПОТОЧНАЯ)

void PPBCommunication::sendNextPacket()
{
    if (m_currentPacketIndex >= m_packetsQueue.size()) {
        // Все пакеты отправлены
        m_packetTimer->stop();
        LOG_INFO("Все тестовые пакеты отправлены");

        // Завершаем операцию
        completeCurrentOperation(true,
                                 QString("Отправлено %1 тестовых пакетов").arg(m_packetsQueue.size()));
        return;
    }

    const DataPacket& packet = m_packetsQueue[m_currentPacketIndex];
    QByteArray packetBytes(reinterpret_cast<const char*>(&packet), sizeof(DataPacket));

    // Отправляем текущий пакет
    sendPacket(packetBytes, QString("Тестовый пакет #%1").arg(m_currentPacketIndex));

    // Прогресс
    emit commandProgress(m_currentPacketIndex + 1, m_packetsQueue.size(), TechCommand::PRBS_M2S);

    m_currentPacketIndex++;
}



void PPBCommunication::sendPacket(const QByteArray& packet, const QString& description)
{
    LOG_INFO(QString("PPBCommunication::sendPacket: %1, размер: %2 байт")
                 .arg(description).arg(packet.size()));

    if (m_currentPort == 0) {
        LOG_ERROR("Не задан порт для отправки");
        setError("Не задан порт для отправки");
        return;
    }

    if (!m_udpClient) {
        LOG_ERROR("UDPClient не установлен");
        setError("UDPClient не установлен");
        return;
    }

    // Широковещательная отправка
    if (m_currentIP.isEmpty() || m_currentIP == "255.255.255.255") {
          LOG_INFO(QString("Отправка широковещательно на порт %1").arg(m_currentPort));
    QMetaObject::invokeMethod(m_udpClient, "sendBroadcast",
                              Qt::QueuedConnection,
                              Q_ARG(QByteArray, packet),
                              Q_ARG(QString, m_currentIP),
                              Q_ARG(quint16, m_currentPort));
        LOG_INFO(QString("Запланирована отправка пакета: %1").arg(description));
        LOG_DEBUG(QString("Отправлен пакет (широковещательно): %1 на порт %2")
                      .arg(description).arg(m_currentPort));
    } else {
        // unicast
         LOG_INFO(QString("Отправка unicast на %1:%2").arg(m_currentIP).arg(m_currentPort));
        QMetaObject::invokeMethod(m_udpClient, "sendTo",
                                  Qt::QueuedConnection,
                                  Q_ARG(QByteArray, packet),
                                  Q_ARG(QString, m_currentIP),
                                  Q_ARG(quint16, m_currentPort));

        LOG_INFO(QString("Запланирована отправка пакета: %1").arg(description));
        LOG_DEBUG(QString("Отправлен пакет (unicast): %1 на %2:%3")
                      .arg(description).arg(m_currentIP).arg(m_currentPort));
    }

}                           //ОТПРАВКА ПАКЕТА(ОДИНОЧНЫЙ, КОМАНДА)

void PPBCommunication::enqueueCommand(TechCommand cmd, uint16_t address)
{
    CommandTask task{cmd, address, QDateTime::currentDateTime()};
    m_taskQueue.enqueue(task);

    emit logMessage(QString("Команда поставлена в очередь. В очереди: %1")
                        .arg(m_taskQueue.size()));

    if (!m_processingTask) {
        processNextTask();
    }
}

void PPBCommunication::processNextTask()
{
    if (m_taskQueue.isEmpty()) {
        m_processingTask = false;
        emit busyChange(false);
        return;
    }

    m_processingTask = true;
    emit busyChange(true);

    CommandTask task = m_taskQueue.dequeue();
    executeCommand(task.cmd, task.address);
}
// Дополнительные методы по необходимости...
