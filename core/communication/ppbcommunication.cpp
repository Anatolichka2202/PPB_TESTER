#include "ppbcommunication.h"
#include <QHostAddress>
#include "ppbprotocol.h"
#include "../utilits/crc.h"
#include "commandandoperation.h"
#include <QThread>


// ===== РЕАЛИЗАЦИЯ =====
PPBCommunication::PPBCommunication(QObject* parent)
    : CommandInterface(parent)
    , m_state(PPBState::Idle)
    , m_udpClient(nullptr)
    , m_currentAddress(0)
    , m_currentPort(0)
    , m_processingTask(false)
{
    LOG_INFO("PPBCommunication конструктор вызван");

    // Таймер для обработки очереди (для обратной совместимости)
    m_taskTimer = new QTimer(this);
    m_taskTimer->setInterval(100);
    connect(m_taskTimer, &QTimer::timeout, this, &PPBCommunication::processNextTask);
}

PPBCommunication::~PPBCommunication()
{
    LOG_INFO("~PPBCommunication: начало");

    // Останавливаем движок
    if (m_engine) {
        m_engine->disconnect();
    }

    LOG_INFO("~PPBCommunication: завершение");
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
        // Сохраняем UDP клиент
        m_udpClient = udpClient;

        // Создаем движок обработки команд
        m_engine = std::make_unique<communicationengine>(udpClient);
        m_engine->setCommandInterface(this);

        if (m_engine) {
        // Подключаем сигналы движка к сигналам PPBCommunication
            connect(m_engine.get(), &communicationengine::stateChanged,
                    this, &PPBCommunication::onEngineStateChanged);

            connect(m_engine.get(), &communicationengine::connected,
                    this, &PPBCommunication::connected);

            connect(m_engine.get(), &communicationengine::disconnected,
                    this, &PPBCommunication::disconnected);

            connect(m_engine.get(), &communicationengine::commandCompleted,
                    this, &PPBCommunication::onEngineCommandCompleted);

            connect(m_engine.get(), &communicationengine::errorOccurred,
                    this, &PPBCommunication::onEngineErrorOccurred);

            connect(m_engine.get(), &communicationengine::logMessage,
                    this, &PPBCommunication::onEngineLogMessage);
        }


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



bool PPBCommunication::connectToPPB(uint16_t address, const QString& ip, quint16 port)
{
    LOG_INFO(QString("PPBCommunication::connectToPPB (фасад): address=0x%1, ip=%2, port=%3")
                 .arg(address, 4, 16, QChar('0'))
                 .arg(ip).arg(port));

    m_currentAddress = address;
    m_currentIP = ip;
    m_currentPort = port;

    if (m_engine) {
         m_engine->setCommandInterface(this);
        return m_engine->connectToPPB(address, ip, port);

    } else {
        LOG_ERROR("communicationengine не инициализирован");
        emit errorOccurred("Движок обработки команд не инициализирован");
        return false;
    }
}

void PPBCommunication::disconnect()
{
    LOG_INFO("PPBCommunication::disconnect (фасад)");

    if (m_engine) {
        m_engine->disconnect();
    }

    // Локальное состояние
    setStateInternal(PPBState::Idle);
}

void PPBCommunication::executeCommand(TechCommand cmd, uint16_t address)
{
    LOG_INFO(QString("PPBCommunication::executeCommand (фасад): cmd=%1, address=0x%2")
                 .arg(static_cast<int>(cmd))
                 .arg(address, 4, 16, QChar('0')));

    if (m_engine) {
        m_engine->executeCommand(cmd, address);
    } else {
        LOG_ERROR("communicationengine не инициализирован");
        emit errorOccurred("Движок обработки команд не инициализирован");
    }
}

void PPBCommunication::sendFUTransmit(uint16_t address)
{
    LOG_INFO(QString("PPBCommunication::sendFUTransmit (фасад): address=0x%1")
                 .arg(address, 4, 16, QChar('0')));

    if (m_engine) {
        m_engine->sendFUTransmit(address);
    } else {
        LOG_ERROR("communicationengine не инициализирован");
        emit errorOccurred("Движок обработки команд не инициализирован");
    }
}

void PPBCommunication::sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3])
{
    LOG_INFO(QString("PPBCommunication::sendFUReceive (фасад): address=0x%1, period=%2")
                 .arg(address, 4, 16, QChar('0')).arg(period));

    if (m_engine) {
        m_engine->sendFUReceive(address, period, fuData);
    } else {
        LOG_ERROR("communicationengine не инициализирован");
        emit errorOccurred("Движок обработки команд не инициализирован");
    }
}

// ===== РЕАЛИЗАЦИЯ ИНТЕРФЕЙСА COMMANDINTERFACE =====

void PPBCommunication::setState(PPBState state)
{
    // Для обратной совместимости - просто устанавливаем состояние
    setStateInternal(state);
}

void PPBCommunication::startTimeoutTimer(int ms)
{
    // Больше не используется напрямую, т.к. таймеры в communicationengine
    Q_UNUSED(ms);
    LOG_DEBUG("PPBCommunication::startTimeoutTimer - вызов игнорируется (управляется в движке)");
}

void PPBCommunication::stopTimeoutTimer()
{
    // Больше не используется напрямую
    LOG_DEBUG("PPBCommunication::stopTimeoutTimer - вызов игнорируется (управляется в движке)");
}

void PPBCommunication::completeCurrentOperation(bool success, const QString& message) {
    // Передаем завершение операции в UI
    emit commandCompleted(success, message, TechCommand::TS);
}

void PPBCommunication::sendPacket(const QByteArray& packet, const QString& description) {
    // Отправляем через движок
    if (m_engine) {
        m_engine->sendPacketInternal(packet, description);
    }
}

void PPBCommunication::sendDataPackets(const QVector<DataPacket>& packets) {
    // Сохраняем для возможного сравнения
    m_generatedPackets = packets;

    // Отправляем через движок
    if (m_engine) {
        // Здесь нужно реализовать метод в движке для отправки массива пакетов
        // Пока отправляем по одному
        for (const DataPacket& packet : packets) {
            QByteArray data(reinterpret_cast<const char*>(&packet), sizeof(DataPacket));
            sendPacket(data, "Data packet");
        }
    }
}

QVector<DataPacket> PPBCommunication::getGeneratedPackets() const {
    return m_generatedPackets;
}

// ===== СЛОТЫ ДЛЯ ОБРАБОТКИ СИГНАЛОВ ОТ ДВИЖКА =====

void PPBCommunication::onEngineStateChanged(uint16_t address, PPBState state)
{
    // Если состояние относится к текущему адресу, обновляем наше состояние
    if (address == m_currentAddress) {
        setStateInternal(state);

        // Дополнительные действия при изменении состояния
        if (state == PPBState::Ready) {
            emit connected();
        } else if (state == PPBState::Idle) {
            emit disconnected();
        }
    }
}

void PPBCommunication::onEngineCommandCompleted(bool success, const QString& report, TechCommand command)
{
    emit commandCompleted(success, report, command);
}

void PPBCommunication::onEngineErrorOccurred(const QString& error)
{
    setError(error);
    emit errorOccurred(error);
}

void PPBCommunication::onEngineLogMessage(const QString& message)
{
    emit logMessage(message);
}

// ===== ПРИВАТНЫЕ МЕТОДЫ =====

void PPBCommunication::setStateInternal(PPBState state)
{
    QMutexLocker locker(&m_stateMutex);
    if (m_state == state) return;

    PPBState oldState = m_state;
    m_state = state;

    LOG_DEBUG(QString("Состояние изменено: %1 -> %2")
                  .arg(static_cast<int>(oldState))
                  .arg(static_cast<int>(state)));

    locker.unlock();
    emit stateChanged(state);

    // Сигнал о занятости
    bool busy = (state == PPBState::SendingCommand || state == PPBState::WaitingData);
    emit busyChange(busy);
}

void PPBCommunication::setError(const QString& error)
{
    m_lastError = error;
    LOG_ERROR(error);
}

// ===== МЕТОДЫ ДЛЯ ОБРАТНОЙ СОВМЕСТИМОСТИ =====

void PPBCommunication::enqueueCommand(TechCommand cmd, uint16_t address)
{
    CommandTask task{cmd, address, QDateTime::currentDateTime()};
    m_taskQueue.enqueue(task);

    if (!m_processingTask) {
        m_taskTimer->start();
    }
}

void PPBCommunication::processNextTask()
{
    if (m_taskQueue.isEmpty()) {
        m_processingTask = false;
        m_taskTimer->stop();
        emit busyChange(false);
        return;
    }

    m_processingTask = true;
    emit busyChange(true);

    CommandTask task = m_taskQueue.dequeue();
    executeCommand(task.cmd, task.address);
}

void PPBCommunication::setParseResult(bool success, const QString& message) {
    LOG_DEBUG(QString("PPBCommunication::setParseResult: %1 - %2")
                  .arg(success ? "УСПЕХ" : "ОШИБКА")
                  .arg(message));

    if (m_engine) {
        // Передаем результат парсинга в движок
        // Нужно будет добавить соответствующий метод в communicationengine
        m_engine->setCommandParseResult(m_currentAddress, success, message);
    } else {
        LOG_WARNING("setParseResult: движок не инициализирован");
    }
}

void PPBCommunication::setParseData(const QVariant& parsedData) {
    LOG_DEBUG(QString("PPBCommunication::setParseData: тип данных: %1")
                  .arg(parsedData.typeName()));

    if (m_engine) {
        // Передаем дополнительные данные парсинга в движок
        m_engine->setCommandParseData(m_currentAddress, parsedData);
    } else {
        LOG_WARNING("setParseData: движок не инициализирован");
    }
}

void PPBCommunication::stop()
{
    LOG_INFO("PPBCommunication::stop() - остановка");

    // Останавливаем движок
    if (m_engine) {
        m_engine->disconnect();
    }

    // Останавливаем таймер
    if (m_taskTimer) {
        m_taskTimer->stop();
    }

    // Очищаем очередь
    m_taskQueue.clear();
    m_processingTask = false;

    // Сбрасываем состояние
    setStateInternal(PPBState::Idle);

    LOG_INFO("PPBCommunication остановлен");
}
