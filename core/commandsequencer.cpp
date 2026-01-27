#include "commandsequencer.h"
#include "udpclient.h"
#include "../analyzer/testsequenceanalyzer.h"
#include <QHostAddress>
#include <QDateTime>

// Константы
const int COMMAND_TIMEOUT_MS = 5000;
const int PACKET_TIMEOUT_MS = 1000;
const int TEST_PACKET_COUNT = 256;
const int PACKET_INTERVAL_MS = 100; // 10 Гц
const int STATUS_PACKET_COUNT = 7;

CommandSequencer::CommandSequencer(UDPClient* udpClient, QObject* parent)
    : QObject(parent)
    , m_udpClient(udpClient)
    , m_currentSequence(SequenceType::None)
    , m_currentAddress(0)
    , m_currentPort(0)
    , m_currentStep(0)
    , m_totalSteps(0)
    , m_isRunning(false)
    , m_expectedPackets(0)
    , m_packetsSent(0)
    , m_packetsReceived(0)
    , m_waitingForResponse(false)
    , m_lastTUCommand(TechCommand::TS)
{
    // Настройка таймеров
    m_stepTimer.setSingleShot(true);
    connect(&m_stepTimer, &QTimer::timeout, this, &CommandSequencer::onTimeout);

    m_testPacketTimer.setSingleShot(false);
    connect(&m_testPacketTimer, &QTimer::timeout, this, &CommandSequencer::onSendNextTestPacket);

    // Подключение к UDP клиенту
    if (m_udpClient) {
        connect(m_udpClient, &UDPClient::dataReceived,
                this, &CommandSequencer::onDataReceived);
        connect(m_udpClient, &UDPClient::errorOccurred,
                this, &CommandSequencer::sequenceError);
    }

    Logger::info("CommandSequencer инициализирован");
}

CommandSequencer::~CommandSequencer()
{
    stopCurrentSequence();
}

// === ПУБЛИЧНЫЕ МЕТОДЫ ===

void CommandSequencer::executeConnectionTest(uint16_t address, const QString& ip, quint16 port)
{
    startSequence(SequenceType::ConnectionTest, "Проверка связи", address, ip, port);
}

void CommandSequencer::executeFullTest(uint16_t address, const QString& ip, quint16 port)
{
    startSequence(SequenceType::FullTest, "Полный тест", address, ip, port);
}

void CommandSequencer::executeTransmitTest(uint16_t address, const QString& ip, quint16 port)
{
    startSequence(SequenceType::TransmitTest, "Тест передачи", address, ip, port);
}

void CommandSequencer::executeReceiveTest(uint16_t address, const QString& ip, quint16 port)
{
    startSequence(SequenceType::ReceiveTest, "Тест приема", address, ip, port);
}

void CommandSequencer::executeStatusRequest(uint16_t address, const QString& ip, quint16 port)
{
    startSequence(SequenceType::StatusRequest, "Опрос состояния", address, ip, port);
}

void CommandSequencer::executeReset(uint16_t address, const QString& ip, quint16 port)
{
    startSequence(SequenceType::Reset, "Сброс ППБ", address, ip, port);
}

void CommandSequencer::executeTUCommand(TechCommand cmd, uint16_t address,
                                        const QString& ip, quint16 port)
{
    m_lastTUCommand = cmd;
    QString cmdName = "ТУ команда 0x" + QString::number(static_cast<uint8_t>(cmd), 16);
    startSequence(SequenceType::TUCommand, cmdName, address, ip, port);
}

void CommandSequencer::executeFUCommand(uint16_t address, bool isTransmit, uint8_t period,
                                        const uint8_t fuData[3], const QString& ip, quint16 port)
{
    QString cmdName = isTransmit ? "ФУ передача" : "ФУ прием";
    startSequence(SequenceType::FUCommand, cmdName, address, ip, port);
    // TODO: Сохранить параметры ФУ
}

void CommandSequencer::stopCurrentSequence()
{
    if (!m_isRunning) return;

    m_testPacketTimer.stop();
    stopStepTimer();

    Logger::warning(QString("Последовательность '%1' принудительно остановлена")
                        .arg(m_sequenceName));

    completeSequence(false, "Операция прервана пользователем");
}

// === СЛОТЫ ===

void CommandSequencer::onDataReceived(const QByteArray& data,
                                      const QHostAddress& sender, quint16 port)
{
    if (!m_isRunning || !m_waitingForResponse) {
        Logger::debug("Получены данные без активной операции");
        return;
    }

    // Логируем получение
    Logger::debug(QString("Получено %1 байт от %2:%3")
                      .arg(data.size()).arg(sender.toString()).arg(port));

    // Пытаемся парсить как разные типы пакетов
    PPBResponse ppbResponse;
    if (PacketBuilder::parsePPBResponse(data, ppbResponse)) {
        handlePPBResponse(ppbResponse);
        return;
    }

    BridgeResponse bridgeResponse;
    if (PacketBuilder::parseBridgeResponse(data, bridgeResponse)) {
        handleBridgeResponse(bridgeResponse);
        return;
    }

    DataPacket dataPacket;
    if (PacketBuilder::parseDataPacket(data, dataPacket)) {
        handleDataPacket(dataPacket);
        return;
    }

    Logger::warning("Не удалось распарсить пакет");
}

void CommandSequencer::onTimeout()
{
    if (!m_isRunning) return;

    QString errorMsg = QString("Таймаут операции: шаг %1/%2")
                           .arg(m_currentStep).arg(m_totalSteps);

    Logger::warning(errorMsg);
    emit timeoutOccurred(errorMsg);

    // В зависимости от типа последовательности, решаем что делать
    switch (m_currentSequence) {
    case SequenceType::ConnectionTest:
    case SequenceType::StatusRequest:
        completeSequence(false, "Таймаут приема данных статуса");
        break;
    case SequenceType::ReceiveTest:
    case SequenceType::FullTest:
        // Для тестовых последовательностей частичный результат - это нормально
        if (m_packetsReceived > 0) {
            // Продолжаем обработку с тем, что есть
            m_currentStep++;
            emit sequenceStep("Анализ полученных данных", m_currentStep, m_totalSteps);
            processReceiveTest();
        } else {
            completeSequence(false, "Не получено ни одного пакета");
        }
        break;
    default:
        completeSequence(false, "Таймаут операции");
        break;
    }
}

void CommandSequencer::onSendNextTestPacket()
{
    if (m_packetsSent >= m_testPackets.size()) {
        m_testPacketTimer.stop();
        Logger::info("Все тестовые пакеты отправлены");

        // Следующий шаг зависит от типа последовательности
        m_currentStep++;
        emit sequenceStep("Ожидание подтверждения", m_currentStep, m_totalSteps);

        if (m_currentSequence == SequenceType::TransmitTest) {
            // Для теста передачи - завершаем
            completeSequence(true, "Тестовые данные отправлены");
        } else if (m_currentSequence == SequenceType::FullTest) {
            // Для полного теста - переходим к приему
            sendTUCommandWithResponse(TechCommand::PRBS_S2M);
        }
        return;
    }

    // Отправляем следующий пакет
    TestPacket& testPacket = m_testPackets[m_packetsSent];
    QByteArray packetData(reinterpret_cast<const char*>(&testPacket.packet),
                          sizeof(DataPacket));

    sendPacket(packetData, QString("Тестовый пакет %1/%2")
                               .arg(m_packetsSent + 1).arg(m_testPackets.size()));

    testPacket.sent = true;
    m_packetsSent++;

    // Обновляем прогресс
    int progress = (m_packetsSent * 100) / m_testPackets.size();
    emit sequenceProgress(progress);
    emit sequenceStep(QString("Отправка пакетов (%1/%2)")
                          .arg(m_packetsSent).arg(m_testPackets.size()),
                      m_currentStep, m_totalSteps);
}

// === ПРИВАТНЫЕ МЕТОДЫ ===

void CommandSequencer::startSequence(SequenceType type, const QString& name,
                                     uint16_t address, const QString& ip, quint16 port)
{
    if (m_isRunning) {
        emit sequenceError("Уже выполняется другая операция");
        return;
    }

    m_currentSequence = type;
    m_sequenceName = name;
    m_currentAddress = address;
    m_currentIP = ip;
    m_currentPort = port;
    m_currentStep = 1;
    m_isRunning = true;
    m_waitingForResponse = false;

    // Очищаем предыдущие данные
    m_testPackets.clear();
    m_statusPackets.clear();
    m_packetsSent = 0;
    m_packetsReceived = 0;

    // Запускаем таймер последовательности
    m_sequenceTimer.start();

    Logger::info(QString("Начало последовательности: %1 для адреса 0x%2")
                     .arg(name).arg(address, 4, 16, QChar('0')));

    emit sequenceStarted(name);
    emit sequenceStep("Инициализация", 1, 10); // Начальное значение

    // Запускаем соответствующую обработку
    switch (type) {
    case SequenceType::ConnectionTest:
        m_totalSteps = 4;
        processConnectionTest();
        break;
    case SequenceType::FullTest:
        m_totalSteps = 8;
        processFullTest();
        break;
    case SequenceType::TransmitTest:
        m_totalSteps = 5;
        processTransmitTest();
        break;
    case SequenceType::ReceiveTest:
        m_totalSteps = 4;
        processReceiveTest();
        break;
    case SequenceType::StatusRequest:
        m_totalSteps = 3;
        processStatusRequest();
        break;
    case SequenceType::Reset:
        m_totalSteps = 2;
        processReset();
        break;
    case SequenceType::TUCommand:
        m_totalSteps = 2;
        processTUCommand();
        break;
    case SequenceType::FUCommand:
        m_totalSteps = 2;
        processFUCommand();
        break;
    default:
        completeSequence(false, "Неизвестный тип последовательности");
        break;
    }
}

void CommandSequencer::completeSequence(bool success, const QString& result)
{
    if (!m_isRunning) return;

    // Останавливаем все таймеры
    m_testPacketTimer.stop();
    stopStepTimer();

    // Логируем завершение
    qint64 elapsed = m_sequenceTimer.elapsed();
    QString logMsg = QString("Последовательность '%1' завершена за %2 мс: %3")
                         .arg(m_sequenceName).arg(elapsed)
                         .arg(success ? "УСПЕХ" : "ОШИБКА");

    if (success) {
        Logger::info(logMsg);
    } else {
        Logger::warning(logMsg + " - " + result);
    }

    m_isRunning = false;
    m_currentSequence = SequenceType::None;

    emit sequenceCompleted(success, result.isEmpty() ? logMsg : result);
}

// === РЕАЛИЗАЦИЯ ЦЕПОЧЕК ===

void CommandSequencer::processConnectionTest()
{
    // Шаг 1: Отправка TS команды
    emit sequenceStep("Отправка команды TS", 1, m_totalSteps);
    sendTUCommandWithResponse(TechCommand::TS);
}

void CommandSequencer::processFullTest()
{
    // Шаг 1: Отправка PRBS_M2S
    emit sequenceStep("Отправка PRBS_M2S", 1, m_totalSteps);
    sendTUCommandWithResponse(TechCommand::PRBS_M2S);
}

void CommandSequencer::processTransmitTest()
{
    // Шаг 1: Отправка PRBS_M2S
    emit sequenceStep("Отправка PRBS_M2S", 1, m_totalSteps);
    sendTUCommandWithResponse(TechCommand::PRBS_M2S);
}

void CommandSequencer::processReceiveTest()
{
    // Шаг 1: Отправка PRBS_S2M
    emit sequenceStep("Отправка PRBS_S2M", 1, m_totalSteps);
    sendTUCommandWithResponse(TechCommand::PRBS_S2M);
}

void CommandSequencer::processStatusRequest()
{
    // Шаг 1: Отправка TS команды
    emit sequenceStep("Отправка команды TS", 1, m_totalSteps);
    sendTUCommandWithResponse(TechCommand::TS);
}

void CommandSequencer::processReset()
{
    // Шаг 1: Отправка TC команды
    emit sequenceStep("Отправка команды сброса", 1, m_totalSteps);
    sendTUCommandWithResponse(TechCommand::TC);
}

void CommandSequencer::processTUCommand()
{
    // Шаг 1: Отправка команды
    emit sequenceStep(QString("Отправка команды 0x%1")
                          .arg(static_cast<uint8_t>(m_lastTUCommand), 2, 16, QChar('0')),
                      1, m_totalSteps);
    sendTUCommandWithResponse(m_lastTUCommand);
}

void CommandSequencer::processFUCommand()
{
    // TODO: Реализовать ФУ команды
    emit sequenceStep("Отправка ФУ команды", 1, m_totalSteps);
    completeSequence(false, "ФУ команды не реализованы");
}

void CommandSequencer::sendTUCommandWithResponse(TechCommand cmd)
{
    QByteArray packet = PacketBuilder::createTURequest(m_currentAddress, cmd);
    sendPacket(packet, QString("%1 команда").arg(static_cast<int>(cmd)));

    m_waitingForResponse = true;
    m_lastTUCommand = cmd;
    startStepTimer(COMMAND_TIMEOUT_MS);
}

void CommandSequencer::waitForDataPackets(int expectedCount, int timeoutPerPacketMs)
{
    m_expectedPackets = expectedCount;
    m_packetsReceived = 0;
    m_waitingForResponse = true;
    startStepTimer(timeoutPerPacketMs * expectedCount);
}

void CommandSequencer::generateTestPackets(int count)
{
    m_testPackets.clear();
    m_testPackets.reserve(count);

    uint8_t lfsr = 0x7F;

    for (int i = 0; i < count; ++i) {
        TestPacket testPacket;

        // Генерация данных
        for (int j = 0; j < 3; ++j) {
            testPacket.packet.data[j] = lfsrNext(lfsr);
        }

        // Расчет CRC
        testPacket.packet.crc = calculateCRC8(testPacket.packet.data, 3);

        testPacket.sent = false;
        testPacket.received = false;
        testPacket.matched = false;

        m_testPackets.append(testPacket);
    }

    m_packetsSent = 0;
    Logger::info(QString("Сгенерировано %1 тестовых пакетов").arg(count));
}

void CommandSequencer::sendTestPackets()
{
    if (m_testPackets.isEmpty()) {
        generateTestPackets(TEST_PACKET_COUNT);
    }

    // Начинаем отправку с интервалом
    m_testPacketTimer.start(PACKET_INTERVAL_MS);
    Logger::info("Начало отправки тестовых пакетов (10 Гц)");
}

// === ОБРАБОТКА ОТВЕТОВ ===

void CommandSequencer::handlePPBResponse(const PPBResponse& response)
{
    stopStepTimer();
    m_waitingForResponse = false;

    if (response.status != 0x00) {
        emit sequenceError("ППБ ответил с ошибкой");
        completeSequence(false, "Ошибка ППБ");
        return;
    }

    Logger::info("Получен OK от ППБ");

    // Определяем что делать дальше в зависимости от команды
    switch (m_lastTUCommand) {
    case TechCommand::TS:
        if (m_currentSequence == SequenceType::ConnectionTest) {
            m_currentStep++;
            emit sequenceStep("Прием данных статуса", m_currentStep, m_totalSteps);
            waitForDataPackets(STATUS_PACKET_COUNT);
        } else if (m_currentSequence == SequenceType::StatusRequest) {
            m_currentStep++;
            emit sequenceStep("Прием данных статуса", m_currentStep, m_totalSteps);
            waitForDataPackets(STATUS_PACKET_COUNT);
        }
        break;

    case TechCommand::PRBS_M2S:
        m_currentStep++;
        emit sequenceStep("Генерация тестовых данных", m_currentStep, m_totalSteps);
        generateTestPackets(TEST_PACKET_COUNT);

        m_currentStep++;
        emit sequenceStep("Отправка тестовых данных", m_currentStep, m_totalSteps);
        sendTestPackets();
        break;

    case TechCommand::PRBS_S2M:
        m_currentStep++;
        emit sequenceStep("Ожидание тестовых данных", m_currentStep, m_totalSteps);
        waitForDataPackets(TEST_PACKET_COUNT, 100);
        break;

    case TechCommand::TC:
        // Сброс завершен
        m_currentStep++;
        emit sequenceStep("Завершение", m_currentStep, m_totalSteps);
        completeSequence(true, "ППБ сброшен");
        break;

    default:
        // Простая команда завершена
        m_currentStep++;
        emit sequenceStep("Завершение", m_currentStep, m_totalSteps);
        completeSequence(true, "Команда выполнена");
        break;
    }
}

void CommandSequencer::handleBridgeResponse(const BridgeResponse& response)
{
    stopStepTimer();
    m_waitingForResponse = false;

    if (response.status == 1) {
        Logger::info("Бридж ответил OK");
        completeSequence(true, "ФУ команда выполнена");
    } else {
        emit sequenceError("Бридж ответил с ошибкой");
        completeSequence(false, "Ошибка ФУ команды");
    }
}

void CommandSequencer::handleDataPacket(const DataPacket& packet)
{
    // Проверяем CRC
    if (!PacketBuilder::checkDataPacketCRC(packet)) {
        Logger::warning("Получен пакет с ошибкой CRC");
        // Продолжаем, но отмечаем
    }

    // В зависимости от типа последовательности
    switch (m_currentSequence) {
    case SequenceType::ConnectionTest:
    case SequenceType::StatusRequest:
        // Сохраняем пакет статуса
        m_statusPackets.append(QByteArray(reinterpret_cast<const char*>(&packet),
                                          sizeof(DataPacket)));
        m_packetsReceived++;

        if (m_packetsReceived >= m_expectedPackets) {
            stopStepTimer();
            m_currentStep++;
            emit sequenceStep("Обработка данных", m_currentStep, m_totalSteps);

            // Эмитируем полученные данные
            emit statusDataReceived(m_currentAddress, m_statusPackets);

            m_currentStep++;
            emit sequenceStep("Завершение", m_currentStep, m_totalSteps);
            completeSequence(true, "Данные статуса получены");
        }
        break;

    case SequenceType::ReceiveTest:
    case SequenceType::FullTest:
        // Сохраняем тестовый пакет
        if (m_packetsReceived < m_testPackets.size()) {
            // Ищем соответствующий отправленный пакет (по CRC или данным)
            // Упрощенная версия: сравниваем CRC
            bool foundMatch = false;
            for (TestPacket& testPacket : m_testPackets) {
                if (!testPacket.received && testPacket.packet.crc == packet.crc) {
                    testPacket.received = true;
                    testPacket.matched = true;
                    foundMatch = true;
                    break;
                }
            }

            if (!foundMatch) {
                // Сохраняем как дополнительный пакет
                Logger::debug("Получен неожиданный пакет");
            }
        }

        m_packetsReceived++;

        // Обновляем прогресс
        int progress = (m_packetsReceived * 100) / m_expectedPackets;
        emit sequenceProgress(progress);
        emit sequenceStep(QString("Прием данных (%1/%2)")
                              .arg(m_packetsReceived).arg(m_expectedPackets),
                          m_currentStep, m_totalSteps);

        if (m_packetsReceived >= m_expectedPackets) {
            stopStepTimer();
            m_currentStep++;
            emit sequenceStep("Анализ данных", m_currentStep, m_totalSteps);

            // Анализируем результаты
            analyzeTestResults();
        }
        break;

    default:
        Logger::warning("Получен пакет данных вне ожидаемой последовательности");
        break;
    }
}

void CommandSequencer::analyzeTestResults()
{
    if (m_currentSequence != SequenceType::ReceiveTest &&
        m_currentSequence != SequenceType::FullTest) {
        return;
    }

    // Подготавливаем векторы для анализатора
    QVector<DataPacket> sentPackets;
    QVector<DataPacket> receivedPackets;

    sentPackets.reserve(m_testPackets.size());
    for (const TestPacket& tp : m_testPackets) {
        sentPackets.append(tp.packet);
    }

    // Полученные пакеты (только те, что совпали)
    for (const TestPacket& tp : m_testPackets) {
        if (tp.received && tp.matched) {
            receivedPackets.append(tp.packet);
        }
    }

    // Используем анализатор
    TestSequenceAnalyzer analyzer;
    auto result = analyzer.analyzeTest(sentPackets, receivedPackets);

    m_currentStep++;
    emit sequenceStep("Завершение", m_currentStep, m_totalSteps);

    emit testDataResult(result.report);
    completeSequence(result.success, result.report);
}

// === УТИЛИТЫ ===

void CommandSequencer::sendPacket(const QByteArray& packet, const QString& description)
{
    if (!m_udpClient) {
        emit sequenceError("UDP клиент не инициализирован");
        return;
    }

    Logger::debug(QString("Отправка: %1 (%2 байт)").arg(description).arg(packet.size()));

    if (m_currentIP.isEmpty() || m_currentIP == "255.255.255.255") {
        m_udpClient->sendBroadcast(packet, m_currentPort);
    } else {
        m_udpClient->sendTo(packet, m_currentIP, m_currentPort);
    }
}

void CommandSequencer::startStepTimer(int ms)
{
    m_stepTimer.start(ms);
}

void CommandSequencer::stopStepTimer()
{
    if (m_stepTimer.isActive()) {
        m_stepTimer.stop();
    }
}

QString CommandSequencer::sequenceTypeToString(SequenceType type) const
{
    switch (type) {
    case SequenceType::ConnectionTest: return "Проверка связи";
    case SequenceType::FullTest: return "Полный тест";
    case SequenceType::TransmitTest: return "Тест передачи";
    case SequenceType::ReceiveTest: return "Тест приема";
    case SequenceType::StatusRequest: return "Опрос состояния";
    case SequenceType::Reset: return "Сброс ППБ";
    case SequenceType::TUCommand: return "ТУ команда";
    case SequenceType::FUCommand: return "ФУ команда";
    default: return "Неизвестная";
    }
}

uint8_t CommandSequencer::lfsrNext(uint8_t &lfsr)
{
    uint8_t bit = ((lfsr >> 6) ^ (lfsr >> 5)) & 1;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}
