#include "ppbcontroller.h"
#include <QDebug>
#include <QThread>
#include "../core/logging/logging_unified.h"

// Подключение сигналов коммуникации
void PPBController::connectCommunicationSignals()
{
    if (!m_communication) {
        LOG_CONTROLLER_WARNING("попытка подключить сигналы к нулевой коммуникации");
        return;
    }

    LOG_CONTROLLER_DEBUG("PPBController: подключение сигналов к PPBCommunication");

    // Подключаем все необходимые сигналы
    connect(m_communication, &PPBCommunication::stateChanged,
            this, &PPBController::onConnectionStateChanged, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::commandCompleted,
            this, &PPBController::onCommandCompleted, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::commandProgress,
            this, &PPBController::onCommandProgress, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::statusReceived,
            this, &PPBController::onStatusReceived, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::errorOccurred,
            this, &PPBController::onErrorOccurred, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::connected,
            this, [this]() {
                LOG_CONTROLLER_DEBUG("PPBController: получен сигнал connected от коммуникации");
                emit connectionStateChanged(PPBState::Ready);
            }, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::disconnected,
            this, [this]() {
                LOG_CONTROLLER_DEBUG("PPBController: получен сигнал disconnected от коммуникации");
                emit connectionStateChanged(PPBState::Idle);
            }, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::busyChange,
            this, &PPBController::onBusyChanged, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::sentPacketsSaved,
            this, &PPBController::onSentPacketsSaved, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::receivedPacketsSaved,
            this, &PPBController::onReceivedPacketsSaved, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::clearPacketDataRequested,
            this, &PPBController::onClearPacketDataRequested, Qt::QueuedConnection);

    // Сигналы контроллера -> коммуникации
    connect(this, &PPBController::executeCommandRequested,
            m_communication, &PPBCommunication::executeCommand, Qt::QueuedConnection);

    connect(this, &PPBController::connectToPPBSignal,
            m_communication, &PPBCommunication::connectToPPB, Qt::QueuedConnection);

    connect(this, &PPBController::disconnectSignal,
            m_communication, &PPBCommunication::disconnect, Qt::QueuedConnection);

    connect(this, &PPBController::sendFUTransmitSignal,
            m_communication, &PPBCommunication::sendFUTransmit, Qt::QueuedConnection);

    connect(this, &PPBController::sendFUReceiveSignal,
            m_communication, &PPBCommunication::sendFUReceive, Qt::QueuedConnection);
}

PPBController::PPBController(PPBCommunication* communication, QObject *parent)
    : QObject(parent)
    , m_communication(communication)
    , m_communicationThread(nullptr)
    , m_autoPollTimer(nullptr)
    , m_autoPollEnabled(false)
    , m_currentAddress(0)
    , busy(false)
    , m_packetAnalyzer(nullptr)
{
    LOG_CONTROLLER_DEBUG("PPBController: конструктор");

    // Создаем анализатор через фабрику
    m_packetAnalyzer = AnalyzerFactory::createAnalyzer(this);

    // Подключаем сигналы анализатора
    if (m_packetAnalyzer) {
        connect(m_packetAnalyzer, &PacketAnalyzerInterface::analysisStarted,
                this, &PPBController::onAnalyzerAnalysisStarted);
        connect(m_packetAnalyzer, &PacketAnalyzerInterface::analysisProgress,
                this, &PPBController::onAnalyzerAnalysisProgress);
        connect(m_packetAnalyzer, &PacketAnalyzerInterface::analysisComplete,
                this, &PPBController::onAnalyzerAnalysisComplete);
        connect(m_packetAnalyzer, &PacketAnalyzerInterface::detailedResultsReady,
                this, &PPBController::onAnalyzerDetailedResultsReady);
    }

    // Инициализируем таймер автоопроса
    m_autoPollTimer = new QTimer(this);
    m_autoPollTimer->setInterval(5000);
    connect(m_autoPollTimer, &QTimer::timeout, this, &PPBController::onAutoPollTimeout);

    // Инициализируем карты состояний
    m_channel1States.clear();
    m_channel2States.clear();

    // Подключаем сигналы от переданной коммуникации
    if (m_communication) {
        connectCommunicationSignals();

        // Инициируем начальное состояние
        PPBState initialState = m_communication->state();
        LOG_CONTROLLER_DEBUG(QString("начальное состояние коммуникации = %1")
                                 .arg(static_cast<int>(initialState)));

        emit connectionStateChanged(initialState);
        LOG_CONTROLLER_INFO("инициализация завершена");
    } else {
        LOG_CONTROLLER_WARNING("коммуникация не передана, состояние = Idle");
        emit connectionStateChanged(PPBState::Idle);
    }
}

PPBController::~PPBController()
{
    if (m_autoPollTimer) {
        m_autoPollTimer->stop();
        delete m_autoPollTimer;
    }
}

void PPBController::onBusyChanged(bool busy)
{
    this->busy = busy;
    emit busyChanged(busy);
}

void PPBController::connectToPPB(uint16_t address, const QString& ip, quint16 port)
{
    LOG_CONTROLLER_INFO(QString("PPBController::connectToPPB: address=0x%1, ip=%2, port=%3")
                            .arg(address, 4, 16, QChar('0')).arg(ip).arg(port));

    setCurrentAddress(address);
    emit connectToPPBSignal(address, ip, port);
    LOG_CONTROLLER_INFO(QString("Подключение к ППБ %1...").arg(address));
}

void PPBController::disconnect()
{
    if (m_communication) {
        m_communication->disconnect();
        LOG_CONTROLLER_INFO("Отключение от ППБ...");
    }
}

void PPBController::requestStatus(uint16_t address)
{
    setCurrentAddress(address);
    emit executeCommandRequested(TechCommand::TS, address);
    LOG_CONTROLLER_INFO(QString("Запрос статуса ППБ %1").arg(address));
}

void PPBController::resetPPB(uint16_t address)
{
    emit executeCommandRequested(TechCommand::TC, address);
    LOG_CONTROLLER_INFO(QString("Сброс ППБ %1").arg(address));
}

void PPBController::setGeneratorParameters(uint16_t address, uint32_t duration, uint8_t duty, uint32_t delay)
{
    LOG_CONTROLLER_INFO( QString("Параметры генератора для ППБ %1: Длительность=%2, Скважность=%3, Задержка=%4")
                            .arg(address).arg(duration).arg(duty).arg(delay));
}

void PPBController::setFUReceive(uint16_t address, uint8_t period)
{
    if (m_communication && !m_communication->isBusy()) {
        m_communication->sendFUReceive(address, period);
        LOG_CONTROLLER_INFO(QString("Режим ФУ прием для ППБ %1").arg(address));
    }
}

void PPBController::setFUTransmit(uint16_t address)
{
    if (m_communication && !m_communication->isBusy()) {
        m_communication->sendFUTransmit(address);
        LOG_CONTROLLER_INFO(QString("Режим ФУ передача для ППБ %1").arg(address));
    }
}

void PPBController::startPRBS_M2S(uint16_t address)
{
    emit executeCommandRequested(TechCommand::PRBS_M2S, address);
    LOG_CONTROLLER_INFO(QString("Запуск PRBS_M2S для ППБ %1").arg(address));
}

void PPBController::startPRBS_S2M(uint16_t address)
{
    emit executeCommandRequested(TechCommand::PRBS_S2M, address);
    LOG_CONTROLLER_INFO(QString("Запуск PRBS_S2M для ППБ %1").arg(address));
}

void PPBController::runFullTest(uint16_t address)
{
    // TODO: реализовать полный тест
}

void PPBController::startAutoPoll(int intervalMs)
{
    m_autoPollEnabled = true;
    m_autoPollTimer->start(intervalMs);
    emit autoPollToggled(true);
    LOG_CONTROLLER_INFO(QString("Автоопрос включен (интервал %1 мс)").arg(intervalMs));
}

void PPBController::stopAutoPoll()
{
    m_autoPollEnabled = false;
    m_autoPollTimer->stop();
    emit autoPollToggled(false);
    LOG_CONTROLLER_INFO("Автоопрос выключен");
}

PPBState PPBController::connectionState() const
{
    return m_communication ? m_communication->state() : PPBState::Idle;
}

bool PPBController::isBusy() const
{
    return m_communication ? m_communication->isBusy() : false;
}

bool PPBController::isAutoPollEnabled() const
{
    return m_autoPollEnabled;
}

UIChannelState PPBController::getChannelState(uint8_t ppbIndex, int channel) const
{
    if (channel == 1) {
        return m_channel1States.value(ppbIndex);
    } else {
        return m_channel2States.value(ppbIndex);
    }
}

// ==================== СЛОТЫ ====================

void PPBController::onStatusReceived(uint16_t address, const QVector<QByteArray>& data)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "onStatusReceived",
                                  Qt::QueuedConnection,
                                  Q_ARG(uint16_t, address),
                                  Q_ARG(QVector<QByteArray>, data));
        return;
    }

    if (m_currentAddress == 0) {
        setCurrentAddress(address);
    }

    processStatusData(address, data);
    emit statusReceived(address, data);
}

void PPBController::onConnectionStateChanged(PPBState state)
{
    emit connectionStateChanged(state);
}

void PPBController::onCommandProgress(int current, int total, TechCommand command)
{
    QString operation = commandToName(command);
    emit operationProgress(current, total, operation);
    LOG_CONTROLLER_INFO(QString("%1: %2/%3").arg(operation).arg(current).arg(total));
}

void PPBController::onCommandCompleted(bool success, const QString& message, TechCommand command)
{
    QString logMsg = QString("Команда %1: %2")
                         .arg(commandToName(command))
                         .arg(message);

    if (success) {
        LOG_CONTROLLER_INFO(logMsg);
        if (command == TechCommand::TS) {
            emit connectionStateChanged(PPBState::Ready);
        }
    } else {
        LOG_CONTROLLER_WARNING(logMsg);
        emit errorOccurred(message);
        LOG_CONTROLLER_ERROR("Ошибка: " + message);
    }

    emit operationCompleted(success, message);
}

void PPBController::onErrorOccurred(const QString& error)
{
    emit errorOccurred(error);
    LOG_CONTROLLER_ERROR("[ОШИБКА] " + error);
}

void PPBController::onAutoPollTimeout()
{
    if (m_autoPollEnabled && m_communication &&
        m_communication->state() == PPBState::Ready && m_currentAddress != 0) {
        requestStatus(m_currentAddress);
    }
}

// ==================== АНАЛИЗ ПАКЕТОВ ====================

void PPBController::saveSentPackets(const QVector<DataPacket>& packets) {
    m_lastSentPackets = packets;
    if (m_packetAnalyzer) {
        m_packetAnalyzer->addSentPackets(packets);
    }
    LOG_CAT_DEBUG("CONTROLLER", QString("Сохранено %1 отправленных пакетов").arg(packets.size()));
}

void PPBController::saveReceivedPackets(const QVector<DataPacket>& packets) {
    m_lastReceivedPackets = packets;
    if (m_packetAnalyzer) {
        m_packetAnalyzer->addReceivedPackets(packets);
    }
    LOG_CAT_DEBUG("CONTROLLER", QString("Сохранено %1 полученных пакетов").arg(packets.size()));
}

void PPBController::onSentPacketsSaved(const QVector<DataPacket>& packets) {
    LOG_CAT_INFO("CONTROLLER", QString("Получены отправленные пакеты: %1 шт").arg(packets.size()));
    saveSentPackets(packets);
    LOG_UI_STATUS(QString("Сохранено %1 отправленных пакетов").arg(packets.size()));
}

void PPBController::onReceivedPacketsSaved(const QVector<DataPacket>& packets) {
    LOG_CAT_INFO("CONTROLLER", QString("Получены принятые пакеты: %1 шт").arg(packets.size()));
    saveReceivedPackets(packets);
    LOG_UI_STATUS(QString("Сохранено %1 полученных пакетов").arg(packets.size()));
}

void PPBController::onClearPacketDataRequested() {
    LOG_CAT_INFO("CONTROLLER", "Запрос на очистку данных пакетов");
    if (m_packetAnalyzer) {
        m_packetAnalyzer->clear();
    }
    m_lastSentPackets.clear();
    m_lastReceivedPackets.clear();
}

void PPBController::analize() {
    LOG_CAT_INFO("CONTROLLER", "=== АНАЛИЗ ПАКЕТОВ ===");

    if (!m_packetAnalyzer) {
        LOG_UI_STATUS("Анализатор не инициализирован");
        return;
    }

    int sentCount = m_packetAnalyzer->sentCount();
    int receivedCount = m_packetAnalyzer->receivedCount();

    if (sentCount == 0 && receivedCount == 0) {
        LOG_UI_STATUS("Нет данных для анализа");
        LOG_CAT_WARNING("ANALYSIS", "Отсутствуют отправленные и полученные пакеты");
        return;
    }

    if (sentCount == 0) {
        LOG_UI_STATUS("Нет отправленных пакетов");
        if (!m_lastReceivedPackets.isEmpty()) {
            showPacketsTable("Полученные пакеты", m_lastReceivedPackets);
        }
        return;
    }

    if (receivedCount == 0) {
        LOG_UI_STATUS("Нет полученных пакетов");
        if (!m_lastSentPackets.isEmpty()) {
            showPacketsTable("Отправленные пакеты", m_lastSentPackets);
        }
        return;
    }

    m_packetAnalyzer->analyze();
}

void PPBController::onAnalyzerAnalysisStarted() {
    LOG_CAT_INFO("CONTROLLER", "Анализатор начал работу");
    emit analysisStarted();
}

void PPBController::onAnalyzerAnalysisProgress(int percent) {
    emit analysisProgress(percent);
}

void PPBController::onAnalyzerAnalysisComplete(const QString& summary) {
    LOG_CAT_INFO("CONTROLLER", "Анализатор завершил работу");
}

void PPBController::onAnalyzerDetailedResultsReady(const QVariantMap& results) {
    LOG_CAT_INFO("CONTROLLER", "Получены детальные результаты анализа");
    QString summary = results.value("summary", "").toString();
    showAnalysisResults(summary, results);
    emit analysisComplete(summary, results);
}

void PPBController::showAnalysisResults(const QString& summary, const QVariantMap& details) {
    // Реализация показа результатов анализа
    CardData summaryCard;
    summaryCard.id = "analysis-summary";
    summaryCard.title = "📊 Результаты анализа";
    summaryCard.backgroundColor = QColor(240, 248, 255);

    summaryCard.addField("Отправлено", details["totalSent"].toString());
    summaryCard.addField("Получено", details["totalReceived"].toString());
    summaryCard.addField("Потери", details["lostPackets"].toString());

    if (details.contains("packetLossRate")) {
        double lossRate = details["packetLossRate"].toDouble();
        summaryCard.addField("Потери (%)", QString::number(lossRate * 100, 'f', 2) + "%");
    }

    summaryCard.addField("Ошибок CRC", details["crcErrors"].toString());
    summaryCard.addField("Битовых ошибок", details["bitErrors"].toString());

    if (details.contains("ber")) {
        double ber = details["ber"].toDouble();
        summaryCard.addField("BER", QString::number(ber, 'e', 6));
    }

    LOG_UI_CARD(summaryCard);

    // Детальная таблица сравнения
    if (details.contains("errorDetails")) {
        QVariantList errorDetails = details["errorDetails"].toList();
        if (!errorDetails.isEmpty()) {
            TableData detailsTable;
            detailsTable.id = "analysis-details";
            detailsTable.title = "Детали сравнения пакетов";
            detailsTable.headers = {"Индекс", "Отправлено", "Получено", "Статус", "Битовые ошибки"};

            for (const auto& item : errorDetails) {
                QVariantMap detail = item.toMap();
                QString status;
                if (detail["isLost"].toBool()) {
                    status = "🔴 ПОТЕРЯН";
                } else if (detail["hasCrcError"].toBool()) {
                    status = "⚠️ ОШИБКА CRC";
                } else if (detail["isOutOfOrder"].toBool()) {
                    status = "↕️ НЕ В ПОРЯДКЕ";
                } else if (detail["bitErrors"].toInt() > 0) {
                    status = QString("⚡ %1 бит").arg(detail["bitErrors"].toInt());
                } else {
                    status = "✅ OK";
                }

                detailsTable.addRow({
                    detail["index"].toString(),
                    detail["sentData"].toString(),
                    detail["receivedData"].toString(),
                    status,
                    detail["bitErrors"].toString()
                });
            }

            LOG_UI_TABLE(detailsTable);
        }
    }

    if (!summary.isEmpty()) {
        LOG_CAT_INFO("ANALYSIS", summary);
    }
}

void PPBController::showPacketsTable(const QString& title, const QVector<DataPacket>& packets) {
    TableData table;
    table.id = "packets-table";
    table.title = title;
    table.headers = {"Индекс", "Данные [0]", "Данные [1]", "CRC", "HEX представление"};
    table.columnFormats[0] = "hex";
    table.columnFormats[1] = "hex";
    table.columnFormats[2] = "hex";
    table.columnFormats[3] = "hex";

    for (const DataPacket& packet : packets) {
        table.addRow({
            QString::number(packet.counter),
            QString::number(packet.data[0], 16).rightJustified(2, '0').toUpper(),
            QString::number(packet.data[1], 16).rightJustified(2, '0').toUpper(),
            QString::number(packet.crc, 16).rightJustified(2, '0').toUpper(),
            QString("[%1 %2] idx:%3 crc:%4")
                .arg(packet.data[0], 2, 16, QChar('0'))
                .arg(packet.data[1], 2, 16, QChar('0'))
                .arg(packet.counter, 3, 10, QChar('0'))
                .arg(packet.crc, 2, 16, QChar('0'))
        });
    }

    LOG_UI_TABLE(table);
    LOG_UI_STATUS(QString("Показано %1 пакетов").arg(packets.size()));
}

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================

void PPBController::processStatusData(uint16_t address, const QVector<QByteArray>& data)
{
    int index = -1;
    switch (address) {
    case 0x0001: index = 0; break;
    case 0x0002: index = 1; break;
    case 0x0004: index = 2; break;
    case 0x0008: index = 3; break;
    case 0x0010: index = 4; break;
    case 0x0020: index = 5; break;
    case 0x0040: index = 6; break;
    case 0x0080: index = 7; break;
    default: break;
    }

    if (index == -1) return;

    if (data.size() < 8) {
        LOG_CONTROLLER_WARNING("Недостаточно данных статуса");
        return;
    }

    // Парсим канал 1 (первые 4 пакета)
    UIChannelState channel1 = parseChannelData(data.mid(0, 4));
    // Парсим канал 2 (следующие 4 пакета)
    UIChannelState channel2 = parseChannelData(data.mid(4, 4));

    // Обновляем состояния
    m_channel1States[index] = channel1;
    m_channel2States[index] = channel2;

    emit channelStateUpdated(index, 1, channel1);
    emit channelStateUpdated(index, 2, channel2);

    LOG_CONTROLLER_INFO(QString("Статус ППБ%1 обновлен").arg(index + 1));
}

UIChannelState PPBController::parseChannelData(const QVector<QByteArray>& channelData)
{
    UIChannelState state;

    // TODO: Реальный парсинг данных
    static int counter = 0;
    state.power = 1250.0f + (counter % 100 - 50);
    state.temperature = 45.0f + (counter % 10 - 5);
    state.vswr = 1.2f + (counter % 10) / 50.0f;
    state.isOk = (counter % 10) != 0;

    counter++;
    return state;
}

QString PPBController::commandToName(TechCommand command) const
{
    static QMap<TechCommand, QString> names = {
        {TechCommand::TS, "Опрос состояния"},
        {TechCommand::TC, "Сброс"},
        {TechCommand::PRBS_M2S, "PRBS передача"},
        {TechCommand::PRBS_S2M, "PRBS приём"}
    };

    return names.value(command, "Неизвестная команда");
}

void PPBController::requestVersion(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::VERS, address);
        LOG_CONTROLLER_INFO(QString("Запрос версии ППБ %1").arg(address));
    }
}

void PPBController::requestVolume(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::VOLUME, address);
        LOG_CONTROLLER_INFO(QString("Запрос тома ПО ППБ %1").arg(address));
    }
}

void PPBController::requestChecksum(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::CHECKSUM, address);
        LOG_CONTROLLER_INFO(QString("Запрос контрольной суммы ППБ %1").arg(address));
    }
}

void PPBController::sendProgram(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::PROGRAMM, address);
        LOG_CONTROLLER_INFO(QString("Обновление ПО ППБ %1").arg(address));
    }
}

void PPBController::sendClean(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::CLEAN, address);
        LOG_CONTROLLER_INFO(QString("Очистка временного файла ПО ППБ %1").arg(address));
    }
}

void PPBController::requestDroppedPackets(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::DROP, address);
        LOG_CONTROLLER_INFO(QString("Запрос отброшенных пакетов ППБ %1").arg(address));
    }
}

void PPBController::requestBER_T(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::BER_T, address);
        LOG_CONTROLLER_INFO(QString("Запрос BER ТУ ППБ %1").arg(address));
    }
}

void PPBController::requestBER_F(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::BER_F, address);
        LOG_CONTROLLER_INFO(QString("Запрос BER ФУ ППБ %1").arg(address));
    }
}

void PPBController::setCommunication(PPBCommunication* communication)
{
    LOG_CONTROLLER_DEBUG("PPBController::setCommunication");

    if (m_communication == communication) {
        LOG_CONTROLLER_DEBUG("PPBController::setCommunication: тот же объект, игнорируем");
        return;
    }

    // Очищаем старое соединение
    if (m_communication) {
        if (m_autoPollTimer && m_autoPollTimer->isActive()) {
            m_autoPollTimer->stop();
        }

        QObject::disconnect(m_communication, nullptr, this, nullptr);
        QObject::disconnect(this, nullptr, m_communication, nullptr);

        if (m_communication->state() == PPBState::Ready) {
            m_communication->disconnect();
        }

        m_communication->deleteLater();
        m_communication = nullptr;

        m_currentAddress = 0;
        m_channel1States.clear();
        m_channel2States.clear();

        emit connectionStateChanged(PPBState::Idle);
        LOG_CONTROLLER_INFO("Коммуникационный объект заменен");
    }

    m_communication = communication;

    if (m_communication) {
        LOG_CONTROLLER_DEBUG("PPBController::setCommunication: настраиваем новый объект");
        connectCommunicationSignals();

        PPBState newState = m_communication->state();
        LOG_CONTROLLER_DEBUG(QString("PPBController::setCommunication: состояние нового объекта = %1")
                                 .arg(static_cast<int>(newState)));

        emit connectionStateChanged(newState);

        if (m_autoPollEnabled && m_autoPollTimer) {
            m_autoPollTimer->start();
        }

        LOG_CONTROLLER_INFO("PPBController: коммуникационный объект успешно заменен");
    } else {
        LOG_CONTROLLER_WARNING("PPBController::setCommunication: передан nullptr");
    }
}

void PPBController::setCurrentAddress(uint16_t address)
{
    if (m_currentAddress != address) {
        LOG_CONTROLLER_DEBUG(QString("PPBController: изменение текущего адреса: 0x%1 -> 0x%2")
                                 .arg(m_currentAddress, 4, 16, QChar('0'))
                                 .arg(address, 4, 16, QChar('0')));
        m_currentAddress = address;
    }
}
