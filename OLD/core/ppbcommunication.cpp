#include "ppbcommunication.h"
#include <QHostAddress>
#include <QDateTime>
#include <QThread>
#include "../analyzer/testsequenceanalyzer.h"

// ===== КОНСТАНТЫ =====
const int OPERATION_TIMEOUT_MS = 5000;    // Таймаут операции 5 сек
const int PACKET_TIMEOUT_MS = 1000;       // Таймаут между пакетами 1 сек
const int TEST_PACKET_COUNT = 256;        // 256 тестовых пакетов
const int PACKET_INTERVAL_MS = 100;       // Интервал 10 Гц = 100 мс

enum class ExpectedDataType {
    None,
    StatusData,     // Данные техсостояния (7 пакетов)
    TestData        // Тестовые данные (256 пакетов)
};

// ===== РЕАЛИЗАЦИЯ =====

PPBCommunication::PPBCommunication(QObject* parent)
    : QObject(parent)
    , m_udpClient(new UDPClient(this))
    , m_state(PPBState::Disconnected)
    , m_operation(OperationType::None)
    , m_currentAddress(0)
    , m_currentPort(0)
    , m_timeoutTimer(new QTimer(this))
    , m_expectedPackets(0)
    , m_receivedPacketCount(0)
    , m_sentPacketCount(0)
    , m_waitingForOk(false)
    , m_testDataSent(false)
{
    // Настройка таймера
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &PPBCommunication::onOperationTimeout);

    // Подключение сигналов UDP
    connect(m_udpClient, &UDPClient::dataReceived,
            this, &PPBCommunication::onDataReceived);
    connect(m_udpClient, &UDPClient::errorOccurred,
            this, &PPBCommunication::onNetworkError);

    ExpectedDataType m_expectedDataType = ExpectedDataType::None;

    log("PPBCommunication инициализирован", "INFO");
}

PPBCommunication::~PPBCommunication()
{
    disconnect();
}


// Вспомогательная функция для преобразования OperationType в строку
QString operationToString(OperationType operation)
{
    switch (operation) {
    case OperationType::StatusRequest: return "Запрос состояния";
    case OperationType::Reset: return "Сброс";
    case OperationType::TransmitTest: return "Тестовая передача";
    case OperationType::ReceiveTest: return "Тестовый прием";
    case OperationType::FullTest: return "Полный тест";
    default: return "Неизвестная операция";
    }
}



// ===== ПУБЛИЧНЫЕ МЕТОДЫ =====

bool PPBCommunication::connectToPPB(uint16_t address, const QString& ip, quint16 port)
{
    // Простая проверка - если уже подключены, отключаем
    if (m_state == PPBState::Connected) {
        disconnect();
        QThread::msleep(100); // Даем время на отключение
    }

    m_currentAddress = address;
    m_currentIP = ip;
    m_currentPort = port;

    // Просто устанавливаем параметры и пробуем связаться
    setState(PPBState::Disconnected);

    // Очищаем очередь повторов
    clearRetryQueue();

    // Отправляем тестовый TS
    QByteArray packet = PacketBuilder::createStatusRequest(address);

    log(QString("Проверка связи с ППБ%1 на %2:%3")
            .arg(address).arg(ip).arg(port));

    sendPacket(packet, "Проверка связи (TS)");

    // Начинаем операцию
    startOperation(OperationType::StatusRequest, address);

    return true;
}
void PPBCommunication::disconnect()
{
    if (m_state == PPBState::Disconnected) {
        return;
    }

    // Останавливаем таймеры
    stopTimeoutTimer();

    // Сбрасываем состояние
    setState(PPBState::Disconnected);
    m_currentIP.clear();
    m_currentPort = 0;
    m_currentAddress = 0;

    log("Логическое отключение от ППБ", "INFO");
}



void PPBCommunication::sendStatusRequest(uint16_t address)
{
     m_lastTechCommand = TechCommand::TS;
    if (m_state != PPBState::Connected) {
        setError("Нет подключения к ППБ");
        return;
    }

    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }

    // Отправляем команду TS
    QByteArray packet = PacketBuilder::createStatusRequest(address);
    sendPacket(packet, QString("Запрос состояния ППБ%1").arg(address));

    // Начинаем операцию
    startOperation(OperationType::StatusRequest, address);
}

void PPBCommunication::sendReset(uint16_t address)
{
    if (m_state != PPBState::Connected) {
        setError("Нет подключения к ППБ");
        return;
    }

    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }

    QByteArray packet = PacketBuilder::createResetRequest(address);
    sendPacket(packet, QString("Сброс ППБ%1").arg(address));

    startOperation(OperationType::Reset, address);
}

void PPBCommunication::sendTransmitTest(uint16_t address)
{
     m_lastTechCommand = TechCommand::PRBS_M2S;
    if (m_state != PPBState::Connected) {
        setError("Нет подключения к ППБ");
        return;
    }

    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }

    // Отправляем команду PRBS_M2S
    QByteArray packet = PacketBuilder::createPRBSTransmitRequest(address);
    sendPacket(packet, QString("Запуск тестовой передачи ППБ%1").arg(address));

    startOperation(OperationType::TransmitTest, address);
}

void PPBCommunication::sendReceiveTest(uint16_t address)
{
     m_lastTechCommand = TechCommand::PRBS_S2M;
    if (m_state != PPBState::Connected) {
        setError("Нет подключения к ППБ");
        return;
    }

    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }

    // Отправляем команду PRBS_S2M
    QByteArray packet = PacketBuilder::createPRBSReceiveRequest(address);
    sendPacket(packet, QString("Запуск тестового приема ППБ%1").arg(address));

    startOperation(OperationType::ReceiveTest, address);
}

void PPBCommunication::sendFullTest(uint16_t address)
{
    if (m_state != PPBState::Connected) {
        setError("Нет подключения к ППБ");
        return;
    }

    if (isBusy()) {
        setError("Уже выполняется другая операция");
        return;
    }
m_lastTechCommand = TechCommand::PRBS_M2S;
    // Начинаем с команды PRBS_M2S
    QByteArray packet = PacketBuilder::createPRBSTransmitRequest(address);
    sendPacket(packet, QString("Начало полного теста ППБ%1").arg(address));

    startOperation(OperationType::FullTest, address);
}

void PPBCommunication::sendFUTransmit(uint16_t address)
{
    QByteArray packet = PacketBuilder::createFUTransmitRequest(address);
    sendPacket(packet, QString("ФУ передача ППБ%1").arg(address));
}

void PPBCommunication::sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3])
{
    QByteArray packet;
    if (fuData) {
        packet = PacketBuilder::createFUReceiveRequest(address, period, fuData);
    } else {
        packet = PacketBuilder::createFUReceiveRequest(address, period);
    }

    sendPacket(packet, QString("ФУ прием ППБ%1, период %2").arg(address).arg(period));
}

// ===== СЛОТЫ =====

void PPBCommunication::onDataReceived(const QByteArray& data,
                                      const QHostAddress& sender, quint16 port)
{
    // Логируем
    QString hex;
    for (int i = 0; i < data.size() && i < 8; ++i) {
        hex += QString::number(static_cast<uint8_t>(data[i]), 16).right(2) + " ";
    }
    log(QString("Получено %1 байт - 0x%2").arg(data.size()).arg(hex), "DEBUG");

    if (data.size() != 4) {
        log("Неверный размер пакета", "WARNING");
        return;
    }

    // Если мы ждем данные (техсостояние или тестовые пакеты)
    if (m_state == PPBState::WaitingData) {
        DataPacket packet;
        if (PacketBuilder::parseDataPacket(data, packet)) {
            processDataPacket(packet);
        } else {
            log("Не удалось распарсить пакет данных", "WARNING");
        }
        return;
    }

    // Если ждем ответ (m_waitingForOk), парсим как PPBResponse
    if (m_waitingForOk) {
        PPBResponse response;
        if (PacketBuilder::parsePPBResponse(data, response)) {
            processPPBResponse(response);
            return;
        }
        // Если не PPBResponse, может быть BridgeResponse
        BridgeResponse bridgeResponse;
        if (PacketBuilder::parseBridgeResponse(data, bridgeResponse)) {
            processBridgeResponse(bridgeResponse);
            return;
        }
        log("Не удалось распарсить ответ", "WARNING");
        return;
    }

    // Если не ждем ни ответ, ни данные - пробуем все варианты
    DataPacket dataPacket;
    if (PacketBuilder::parseDataPacket(data, dataPacket)) {
        log("Получен неожиданный пакет данных", "WARNING");
        return;
    }

    PPBResponse ppbResponse;
    if (PacketBuilder::parsePPBResponse(data, ppbResponse)) {
        log("Получен неожиданный ответ ППБ", "WARNING");
        return;
    }

    BridgeResponse bridgeResponse;
    if (PacketBuilder::parseBridgeResponse(data, bridgeResponse)) {
        log("Получен неожиданный ответ бриджа", "WARNING");
        return;
    }

    log("Не удалось распарсить пакет", "WARNING");
}

void PPBCommunication::onNetworkError(const QString& error)
{
    setError(QString("Сетевая ошибка: %1").arg(error));

    if (m_operation != OperationType::None) {
        completeOperation(false, error);
    }
}

void PPBCommunication::onOperationTimeout()
{
    QString operationName = operationToString(m_operation);

    log(QString("Таймаут операции: %1").arg(operationName), "WARNING");

    // Если это тестовая последовательность - анализируем что есть
    if (m_operation == OperationType::TransmitTest ||
        m_operation == OperationType::ReceiveTest ||
        m_operation == OperationType::FullTest) {

        if (m_receivedPacketCount > 0) {
            // Анализируем полученные данные
            TestSequenceAnalyzer analyzer;
            auto result = analyzer.analyzeTest(m_sentPackets, m_receivedPackets);

            // Добавляем информацию о таймауте
            result.report = QString("⚠️ ТЕСТ ПРЕРВАН ПО ТАЙМАУТУ (получено %1/%2 пакетов)\n%3")
                                .arg(m_receivedPacketCount)
                                .arg(m_expectedPackets)
                                .arg(result.report);

            emit testCompleted(result.success, result.report, m_operation);

            // Возвращаемся в Connected
            setState(PPBState::Connected);
            m_operation = OperationType::None;
            emit busyChanged(false);
            return;
        }
    }

    // Для других операций пробуем повторить
    if (!m_isProcessingRetry) {
        // Ставим в очередь повтор последней команды
        QByteArray retryPacket;
        QString description;

        if (m_lastTechCommand == TechCommand::TS) {
            retryPacket = PacketBuilder::createStatusRequest(m_currentAddress);
            description = "Повтор TS";
        } else if (m_lastTechCommand == TechCommand::PRBS_S2M) {
            retryPacket = PacketBuilder::createPRBSReceiveRequest(m_currentAddress);
            description = "Повтор PRBS_S2M";
        } else if (m_lastTechCommand == TechCommand::PRBS_M2S) {
            retryPacket = PacketBuilder::createPRBSTransmitRequest(m_currentAddress);
            description = "Повтор PRBS_M2S";
        }

        if (!retryPacket.isEmpty()) {
            enqueueRetry(m_lastTechCommand, retryPacket);
        } else {
            // Неизвестная команда - просто завершаем с ошибкой
            completeOperation(false, QString("Таймаут %1").arg(operationName));
        }
    } else {
        // Уже в процессе повтора - завершаем операцию
        completeOperation(false, QString("Таймаут после %1 попыток").arg(m_currentRetry));
    }
}
    void PPBCommunication::retryLastCommand()
    {
        static int retryCount = 0;
        const int MAX_RETRIES = 3;

        if (retryCount < MAX_RETRIES) {
            retryCount++;
            Logger::info(QString("Повтор команды %1/%2").arg(retryCount).arg(MAX_RETRIES));

            // Просто повторяем последнюю команду
            if (m_lastTechCommand == TechCommand::PRBS_S2M) {
                QByteArray packet = PacketBuilder::createPRBSReceiveRequest(m_currentAddress);
                sendPacket(packet, QString("Повтор PRBS_S2M (%1/%2)").arg(retryCount).arg(MAX_RETRIES));

                // Сбрасываем ожидание
                m_receivedPacketCount = 0;
                m_receivedPackets.clear();
                startTimeoutTimer(30000); // 30 секунд на новый таймаут
            }
        } else {
            Logger::error("Исчерпаны попытки повторных команд");
            retryCount = 0;
            setState(PPBState::Connected); // Просто возвращаемся в Connected
        }
    }
// ===== ПРИВАТНЫЕ МЕТОДЫ =====

void PPBCommunication::setState(PPBState state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    emit stateChanged(state);

    switch (state) {
    case PPBState::Connected:
        emit connected();
        break;
    case PPBState::Disconnected:
        emit disconnected();
        break;
    default:
        break;
    }
}

void PPBCommunication::setError(const QString& error)
{
    m_lastError = error;
    log(error, "ERROR");
    emit errorOccurred(error);
}



void PPBCommunication::startOperation(OperationType operation, uint16_t address)
{
    m_operation = operation;
    m_currentAddress = address;
    m_waitingForOk = true;
    m_testDataSent = false;
    m_receivedPacketCount = 0;
    m_sentPacketCount = 0;

    // Очищаем данные предыдущей операции
    m_sentPackets.clear();
    m_receivedPackets.clear();
    m_statusData.clear();

    // Запускаем таймер
    startTimeoutTimer(OPERATION_TIMEOUT_MS);
    m_operationTimer.start();

    setState(PPBState::SendingCommand);
    emit busyChanged(true);

    log(QString("Начало операции: %1 для ППБ%2")
            .arg(operationToString(operation)).arg(address));
}

void PPBCommunication::completeOperation(bool success, const QString& message)
{
    // Если операции нет - выходим
    if (m_operation == OperationType::None) {
        Logger::warning("completeOperation вызвана без активной операции");
        return;
    }

    // Сохраняем текущую операцию перед сбросом
    OperationType completedOperation = m_operation;
    uint16_t currentAddress = m_currentAddress;

    // Сбрасываем состояние
    m_operation = OperationType::None;
    m_waitingForOk = false;
    m_testDataSent = false;

    // Останавливаем таймер
    stopTimeoutTimer();

    // Всегда остаемся в Connected после тестовых операций
    // Для других операций - в зависимости от успеха
    if (completedOperation == OperationType::TransmitTest ||
        completedOperation == OperationType::ReceiveTest ||
        completedOperation == OperationType::FullTest) {
        // Тестовые операции НИКОГДА не отключают нас
        setState(PPBState::Connected);
    } else {
        // Для остальных операций - стандартная логика
        if (success) {
            setState(PPBState::Connected);
        } else {
            setState(PPBState::Disconnected);
        }
    }

    // Сообщаем о завершении занятости
    emit busyChanged(false);

    // Излучаем результат в зависимости от типа операции
    if (completedOperation == OperationType::StatusRequest) {
        if (success) {
            emit statusReceived(currentAddress, m_statusData);
        }
        // При ошибке не излучаем statusReceived
        Logger::info(QString("Операция опроса состояния завершена: %1")
                         .arg(success ? "УСПЕХ" : "ОШИБКА"));
    }
    else if (completedOperation == OperationType::TransmitTest ||
             completedOperation == OperationType::ReceiveTest ||
             completedOperation == OperationType::FullTest) {

        // ВСЕГДА анализируем тестовые данные, даже если success=false
        Logger::info("Запуск анализатора тестовой последовательности...");

        TestSequenceAnalyzer analyzer;
        auto result = analyzer.analyzeTest(m_sentPackets, m_receivedPackets);

        // Для тестовых операций всегда считаем success=true, чтобы показать анализ
        // Или используем result.success, если хотим отражать результат анализа
        emit testCompleted(true, result.report, completedOperation);

        Logger::info(QString("Анализ теста завершен. Получено пакетов: %1/%2")
                         .arg(m_receivedPackets.size()).arg(m_sentPackets.size()));
    }
    else if (completedOperation == OperationType::Reset) {
        // Для сброса просто эмитим результат
        emit testCompleted(success, message, completedOperation);
        Logger::info(QString("Операция сброса завершена: %1")
                         .arg(success ? "УСПЕХ" : "ОШИБКА"));
    }
    else {
        // Для всех остальных операций
        emit testCompleted(success, message, completedOperation);
    }

    // Очищаем данные операции (кроме сохраненных пакетов для анализа)
    m_statusData.clear();
    // m_sentPackets и m_receivedPackets не очищаем - они могут понадобиться для UI

    // Сбрасываем счетчики
    m_receivedPacketCount = 0;
    m_sentPacketCount = 0;
    m_expectedPackets = 0;

    Logger::info(QString("Операция %1 завершена: %2")
                     .arg(operationToString(completedOperation))
                     .arg(success ? "УСПЕХ" : "ОШИБКА: " + message));
}

void PPBCommunication::sendPacket(const QByteArray& packet, const QString& description)
{

    if (m_currentPort == 0) {
        setError("Не задан порт для отправки");
        return;
    }

    // Используем широковещательную отправку для ППБ
    // Но если указан конкретный IP - можно использовать его для отладки
    if (m_currentIP.isEmpty() || m_currentIP == "255.255.255.255") {
        m_udpClient->sendBroadcast(packet, m_currentPort);
        log(QString("Отправлен пакет (широковещательно): %1 на порт %2")
                .arg(description).arg(m_currentPort), "DEBUG");
    } else {
        // Для отладки можно использовать конкретный IP
        m_udpClient->sendTo(packet, m_currentIP, m_currentPort);
        log(QString("Отправлен пакет (unicast): %1 на %2:%3")
                .arg(description).arg(m_currentIP).arg(m_currentPort), "DEBUG");
    }
}

void PPBCommunication::sendTestDataPackets()
{
    if (m_testDataSent) {
        return;
    }

    log("Начало отправки тестовых пакетов (10 Гц)", "INFO");

    // Генерируем тестовые пакеты
    m_sentPackets = generateTestPackets(TEST_PACKET_COUNT);
    m_sentPacketCount = 0;

    // Используем таймер для отправки с частотой 10 Гц
    QTimer* sendTimer = new QTimer(this);
    sendTimer->setInterval(PACKET_INTERVAL_MS);

    connect(sendTimer, &QTimer::timeout, this, [this, sendTimer]() {
        if (m_sentPacketCount < m_sentPackets.size()) {
            DataPacket packet = m_sentPackets[m_sentPacketCount];
            QByteArray packetData(reinterpret_cast<const char*>(&packet), sizeof(packet));

            sendPacket(packetData, QString("Тестовый пакет %1/%2")
                                       .arg(m_sentPacketCount + 1)
                                       .arg(TEST_PACKET_COUNT));

            m_sentPacketCount++;

            emit testProgress(m_sentPacketCount, TEST_PACKET_COUNT, m_operation);

            // Если все пакеты отправлены
            if (m_sentPacketCount >= TEST_PACKET_COUNT) {
                sendTimer->stop();
                sendTimer->deleteLater();
                m_testDataSent = true;

                log("Все тестовые пакеты отправлены", "INFO");

                // Если это полный тест, переходим к приему
                if (m_operation == OperationType::FullTest) {
                    // Все пакеты отправлены, теперь отправляем PRBS_S2M
                    m_lastTechCommand = TechCommand::PRBS_S2M;  //
                    QByteArray packet = PacketBuilder::createPRBSReceiveRequest(m_currentAddress);
                    sendPacket(packet, "Запрос приема тестовых данных (PRBS_S2M)");
                    m_waitingForOk = true;
                    startTimeoutTimer(OPERATION_TIMEOUT_MS);
                } else {
                    // Только передача - завершаем операцию
                    completeOperation(true, "Тестовые данные отправлены");
                }
            }
        }
    });

    sendTimer->start();
}

void PPBCommunication::processOkResponse(const OkResponse& response)
{

    TechCommand cmd = static_cast<TechCommand>(response.command);

    switch (cmd) {
    case TechCommand::TS:
        if (m_waitingForOk) {
            stopTimeoutTimer();
            m_waitingForOk = false;

            if (response.status == 1) {
                // Успех!
                log("Получен OK на команду TS");
                setState(PPBState::WaitingData);
                m_expectedPackets = 2;  // Или 5 по эмулятору
                m_receivedPacketCount = 0;
                startTimeoutTimer(2000);
            } else {
                log("ППБ ответил с ошибкой на команду TS", "ERROR");
                // Явно переходим в Disconnected и завершаем операцию
                setState(PPBState::Disconnected);
                completeOperation(false, "Ошибка подключения: ППБ ответил с ошибкой");
            }
        }
        break;

    case TechCommand::PRBS_M2S:
        // Для PRBS_M2S начинаем отправку тестовых данных
        setState(PPBState::ProcessingTest);
        sendTestDataPackets();
        break;

    case TechCommand::PRBS_S2M:
        // Для PRBS_S2M ждем тестовые данные
        setState(PPBState::WaitingData);
        m_expectedPackets = TEST_PACKET_COUNT;
        startTimeoutTimer(PACKET_TIMEOUT_MS * m_expectedPackets);
        break;

    default:
        // Для других команд просто завершаем операцию
        completeOperation(true);
        break;
    }
}

void PPBCommunication::processDataPacket(const DataPacket& packet)
{
    // Проверяем CRC
    if (!PacketBuilder::checkDataPacketCRC(packet)) {
        log("Ошибка CRC в пакете данных", "WARNING");
        // Можно продолжить, но отметить
    }

    // В зависимости от текущей операции
    switch (m_operation) {
    case OperationType::StatusRequest:
        processStatusData(packet);
        break;

    case OperationType::TransmitTest:
    case OperationType::ReceiveTest:
    case OperationType::FullTest:
        processTestData(packet);
        break;

    default:
        log("Получен пакет данных без активной операции", "WARNING");
        break;
    }
}

void PPBCommunication::processStatusData(const DataPacket& packet)
{
    // Преобразуем в QByteArray
    QByteArray data(reinterpret_cast<const char*>(&packet), sizeof(packet));
    m_statusData.append(data);

    m_receivedPacketCount++;

    log(QString("Получен пакет техсостояния %1/%2").arg(m_receivedPacketCount).arg(m_expectedPackets));

    // Все пакеты получены?
    if (m_receivedPacketCount >= m_expectedPackets) {
        stopTimeoutTimer();
        completeOperation(true);
    } else {
        // Перезапускаем таймер для следующего пакета
        startTimeoutTimer(2000);
    }
}

void PPBCommunication::processTestData(const DataPacket& packet)
{
    // Сохраняем полученный пакет
    m_receivedPackets.append(packet);
    m_receivedPacketCount++;

    emit testPacketReceived(packet);
    emit testProgress(m_receivedPacketCount, m_expectedPackets, m_operation);

    log(QString("Получен тестовый пакет %1/%2").arg(m_receivedPacketCount).arg(m_expectedPackets), "DEBUG");

    // Проверяем, все ли пакеты получены
    if (m_receivedPacketCount >= m_expectedPackets) {
        stopTimeoutTimer();

        // Если это был полный тест, сравниваем отправленные и полученные пакеты
        if (m_operation == OperationType::FullTest) {
            QString report = PacketBuilder::compareSequences(m_sentPackets, m_receivedPackets);
            bool success = report.contains("Все пакеты совпадают");
            completeOperation(success, report);
        } else {
            // Только прием - завершаем операцию
            completeOperation(true, "Тестовые данные получены");
        }
    } else {
        // Перезапускаем таймер для следующего пакета
        startTimeoutTimer(PACKET_TIMEOUT_MS);
    }
}

QVector<DataPacket> PPBCommunication::generateTestPackets(int count)
{
    QVector<DataPacket> packets;
    packets.reserve(count);

    uint8_t lfsr = 0x7F; // Начальное значение (любое ненулевое)

    for (int i = 0; i < count; ++i) {
        DataPacket packet;

        // Генерируем 3 байта данных по полиному x^7 + x^6 + 1
        for (int j = 0; j < 3; ++j) {
            packet.data[j] = lfsrNext(lfsr);
        }

        // CRC считаем по тем же данным
        packet.crc = calculateCRC8(packet.data, 3);

        packets.append(packet);
    }

    return packets;
}

uint8_t PPBCommunication::lfsrNext(uint8_t &lfsr)
{
    // Полином: x^7 + x^6 + 1 (биты 7 и 6)
    uint8_t bit = ((lfsr >> 6) ^ (lfsr >> 5)) & 1;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}

void PPBCommunication::startTimeoutTimer(int ms)
{
    m_timeoutTimer->start(ms);
}

void PPBCommunication::stopTimeoutTimer()
{
    m_timeoutTimer->stop();
}

void PPBCommunication::log(const QString& message, const QString& level)
{
    // Только логируем, не излучаем сигналы
    if (level == "DEBUG") {
        Logger::debug(QString("[PPBComm] %1").arg(message));
    } else if (level == "INFO") {
        Logger::info(QString("[PPBComm] %1").arg(message));
    } else if (level == "WARNING") {
        Logger::warning(QString("[PPBComm] %1").arg(message));
    } else if (level == "ERROR") {
        Logger::error(QString("[PPBComm] %1").arg(message));
    }
}

void PPBCommunication::processPPBResponse(const PPBResponse& response)
{
    if (m_isProcessingRetry) {
        clearRetryQueue();
    }
    // Это ответ на ТУ команду
    log(QString("Ответ ППБ: адрес=%1, статус=%2, CRC=0x%3")
            .arg(response.address)
            .arg(response.status)
            .arg(QString::number(response.crc, 16)), "DEBUG");

    // Проверяем, ожидали ли мы этот ответ
    if (!m_waitingForOk) {
        log("Получен неожиданный ответ ППБ", "WARNING");
        return;
    }

    stopTimeoutTimer();
    m_waitingForOk = false;

    // Статус: 0x00 = ОК, 0x01 = ошибка
    if (response.status == 0x00) { // OK
        log("ППБ ответил OK");

        // Определяем, какая команда была отправлена
        // У нас есть m_operation и m_lastTechCommand
        if (m_operation == OperationType::StatusRequest && m_state != PPBState::Connected) {
            setState(PPBState::Connected);
            log("Связь установлена");
        }
        if (m_operation == OperationType::StatusRequest ||
            (m_operation == OperationType::None && m_lastTechCommand == TechCommand::TS)) {
            // Это запрос состояния (при подключении или ручном опросе)
            setState(PPBState::WaitingData);
            m_expectedPackets = 7;  // 5 пакетов данных
            m_receivedPacketCount = 0;
            m_statusData.clear();
            startTimeoutTimer(5000);
        }
        else if (m_operation == OperationType::TransmitTest ||
                 (m_operation == OperationType::FullTest && m_lastTechCommand == TechCommand::PRBS_M2S)) {
            // Это команда PRBS_M2S (отправка тестовых данных)
            setState(PPBState::ProcessingTest);
            sendTestDataPackets();
        }
        else if (m_operation == OperationType::ReceiveTest ||
                 (m_operation == OperationType::FullTest && m_lastTechCommand == TechCommand::PRBS_S2M)) {
            // Это команда PRBS_S2M (прием тестовых данных)
            setState(PPBState::WaitingData);
            m_expectedPackets = 256;  // 256 тестовых пакетов
            m_receivedPacketCount = 0;
            m_receivedPackets.clear();
            startTimeoutTimer(30000);  // 30 секунд
        }
        else {
            // Для других команд просто завершаем операцию
            completeOperation(true);
        }
    } else {
        // Ошибка
        log("ППБ ответил с ошибкой", "ERROR");
        completeOperation(false, "ППБ ответил с ошибкой");
    }
}

void PPBCommunication::processBridgeResponse(const BridgeResponse& response)
{
    // Это ответ на ФУ команду
    log(QString("Ответ бриджа: адрес=%1, команда=0x%2, статус=%3")
            .arg(response.address)
            .arg(QString::number(response.command, 16))
            .arg(response.status), "DEBUG");

    // Статус: 0 = ошибка, 1 = ОК
    if (response.status == 1) {
        log("Бридж ответил OK на ФУ команду");
    } else {
        log("Бридж ответил с ошибкой на ФУ команду", "ERROR");
        emit errorOccurred("Ошибка ФУ команды");
    }
}



// Дополнительные методы по необходимости...
