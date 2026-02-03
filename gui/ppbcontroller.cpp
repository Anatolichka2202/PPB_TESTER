#include "ppbcontroller.h"
#include "../core/logger.h"
#include <QDebug>
#include <QThread>


void PPBController::connectCommunicationSignals()
{
    if (!m_communication) {
        LOG_WARNING("PPBController: попытка подключить сигналы к нулевой коммуникации");
        return;
    }

    LOG_DEBUG("PPBController: подключение сигналов к PPBCommunication");

    // === СИГНАЛЫ ОТ КОММУНИКАЦИИ К КОНТРОЛЛЕРУ ===

    // 1. Изменение состояния подключения
    connect(m_communication, &PPBCommunication::stateChanged,
            this, &PPBController::onConnectionStateChanged, Qt::QueuedConnection);

    // 2. Завершение выполнения команды
    connect(m_communication, &PPBCommunication::commandCompleted,
            this, &PPBController::onCommandCompleted, Qt::QueuedConnection);

    // 3. Прогресс выполнения команды
    connect(m_communication, &PPBCommunication::commandProgress,
            this, &PPBController::onCommandProgress, Qt::QueuedConnection);

    // 4. Получение статуса от ППБ
    connect(m_communication, &PPBCommunication::statusReceived,
            this, &PPBController::onStatusReceived, Qt::QueuedConnection);

    // 5. Ошибки при работе
    connect(m_communication, &PPBCommunication::errorOccurred,
            this, &PPBController::onErrorOccurred, Qt::QueuedConnection);

    // 6. Сообщения для лога
    connect(m_communication, &PPBCommunication::logMessage,
            this, &PPBController::logMessage, Qt::QueuedConnection);

    // 7. Сигналы подключения/отключения (специальные)
    connect(m_communication, &PPBCommunication::connected,
            this, [this]() {
                LOG_DEBUG("PPBController: получен сигнал connected от коммуникации");
                emit connectionStateChanged(PPBState::Ready);
            }, Qt::QueuedConnection);

    connect(m_communication, &PPBCommunication::disconnected,
            this, [this]() {
                LOG_DEBUG("PPBController: получен сигнал disconnected от коммуникации");
                emit connectionStateChanged(PPBState::Idle);
            }, Qt::QueuedConnection);

    // 8. Изменение состояния занятости
    connect(m_communication, &PPBCommunication::busyChange,
            this, &PPBController::onBusyChanged, Qt::QueuedConnection);

    // === СИГНАЛЫ ОТ КОНТРОЛЛЕРА К КОММУНИКАЦИИ ===

    // 9. Запрос на выполнение команды
    connect(this, &PPBController::executeCommandRequested,
            m_communication, &PPBCommunication::executeCommand, Qt::QueuedConnection);

    // 10. Подключение к ППБ
    connect(this, &PPBController::connectToPPBSignal,
            m_communication, &PPBCommunication::connectToPPB, Qt::QueuedConnection);

    // 11. Отключение от ППБ
    connect(this, &PPBController::disconnectSignal,
            m_communication, &PPBCommunication::disconnect, Qt::QueuedConnection);

    // 12. Команды ФУ (функционального управления)
    connect(this, &PPBController::sendFUTransmitSignal,
            m_communication, &PPBCommunication::sendFUTransmit, Qt::QueuedConnection);

    connect(this, &PPBController::sendFUReceiveSignal,
            m_communication, &PPBCommunication::sendFUReceive, Qt::QueuedConnection);

    LOG_INFO("PPBController: все сигналы успешно подключены");
}

PPBController::PPBController(PPBCommunication* communication, QObject *parent)
    : QObject(parent)
    , m_communication(communication)
    , m_autoPollTimer(nullptr)
    , m_autoPollEnabled(false)
    , m_currentAddress(0)
    , busy(false)  // Инициализируем busy
{
    LOG_DEBUG("PPBController: конструктор, thread=" +
              QString::number((qulonglong)QThread::currentThreadId()));

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
        LOG_DEBUG(QString("PPBController: начальное состояние коммуникации = %1")
                      .arg(static_cast<int>(initialState)));

        emit connectionStateChanged(initialState);

        LOG_INFO("PPBController: инициализация завершена");
    } else {
        LOG_WARNING("PPBController: коммуникация не передана, состояние = Idle");
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
    // Перенаправляем сигнал в UI
    emit busyChanged(busy);

    // Логируем для отладки
    if (busy) {
        LOG_INFO("PPBCommunication занят выполнением команды");
        emit logMessage("Система занята выполнением команды...");
    } else {
        LOG_INFO("PPBCommunication свободен");
        emit logMessage("Система готова к новым командам");
    }

}
void PPBController::connectToPPB(uint16_t address, const QString& ip, quint16 port)
{
    LOG_INFO(QString("PPBController::connectToPPB: address=0x%1, ip=%2, port=%3")
                 .arg(address, 4, 16, QChar('0')).arg(ip).arg(port));

    // Устанавливаем текущий адрес
    setCurrentAddress(address);

    emit connectToPPBSignal(address, ip, port);
    emit logMessage(QString("Подключение к ППБ %1...").arg(address));
}
void PPBController::disconnect()
{
    if (m_communication) {
        m_communication->disconnect();
        emit logMessage("Отключение от ППБ...");
    }
}

void PPBController::requestStatus(uint16_t address)
{
    // Устанавливаем текущий адрес для автоопроса
    setCurrentAddress(address);

    emit executeCommandRequested(TechCommand::TS, address);
    emit logMessage(QString("Запрос статуса ППБ %1").arg(address));
}
void PPBController::resetPPB(uint16_t address)
{
    emit executeCommandRequested(TechCommand::TC, address);
        emit logMessage(QString("Сброс ППБ %1").arg(address));

}

void PPBController::setGeneratorParameters(uint16_t address, uint32_t duration, uint8_t duty, uint32_t delay)
{
    // TODO: Реализовать через ФУ
    emit logMessage(QString("Параметры генератора для ППБ %1: Длительность=%2, Скважность=%3, Задержка=%4")
                        .arg(address).arg(duration).arg(duty).arg(delay));
}

void PPBController::setFUReceive(uint16_t address, uint8_t period)
{
    if (m_communication && !m_communication->isBusy()) {
        m_communication->sendFUReceive(address, period);
        emit logMessage(QString("Режим ФУ прием для ППБ %1").arg(address));
    }
}

void PPBController::setFUTransmit(uint16_t address)
{
    if (m_communication && !m_communication->isBusy()) {
        m_communication->sendFUTransmit(address);
        emit logMessage(QString("Режим ФУ передача для ППБ %1").arg(address));
    }
}

void PPBController::startPRBS_M2S(uint16_t address)
{
    emit executeCommandRequested(TechCommand::PRBS_M2S, address);
        emit logMessage(QString("Запуск PRBS_M2S для ППБ %1").arg(address));

}

void PPBController::startPRBS_S2M(uint16_t address)
{
    emit executeCommandRequested(TechCommand::PRBS_S2M, address);
        emit logMessage(QString("Запуск PRBS_S2M для ППБ %1").arg(address));

}

void PPBController::runFullTest(uint16_t address)
{
    //TODO
}
void PPBController::startAutoPoll(int intervalMs)
{
    m_autoPollEnabled = true;
    m_autoPollTimer->start(intervalMs);
    emit autoPollToggled(true);
    emit logMessage(QString("Автоопрос включен (интервал %1 мс)").arg(intervalMs));
}

void PPBController::stopAutoPoll()
{
    m_autoPollEnabled = false;
    m_autoPollTimer->stop();
    emit autoPollToggled(false);
    emit logMessage("Автоопрос выключен");
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
    // Проверяем, в каком потоке мы находимся
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "onStatusReceived",
                                  Qt::QueuedConnection,
                                  Q_ARG(uint16_t, address),
                                  Q_ARG(QVector<QByteArray>, data));
        return;
    }

    // Устанавливаем текущий адрес (если еще не установлен)
    if (m_currentAddress == 0) {
        setCurrentAddress(address);
    }

    // Обрабатываем данные
    processStatusData(address, data);

    // Сигнализируем о получении статуса
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
    emit logMessage(QString("%1: %2/%3").arg(operation).arg(current).arg(total));
}

void PPBController::onCommandCompleted(bool success, const QString& message, TechCommand command)
{
    QString logMsg = QString("Команда %1: %2")
                         .arg(commandToName(command))
                         .arg(message);

    if (success) {
        LOG_INFO(logMsg);
        emit logMessage(logMsg);

        if (command == TechCommand::TS) {
            emit connectionStateChanged(PPBState::Ready);
        }
    } else {
        LOG_WARNING(logMsg);
        emit errorOccurred(message);
        emit logMessage("Ошибка: " + message);
    }

    // Уведомляем UI о завершении операции
    emit operationCompleted(success, message);
}

void PPBController::onErrorOccurred(const QString& error)
{
    emit errorOccurred(error);
    emit logMessage("[ОШИБКА] " + error);
}

void PPBController::onAutoPollTimeout()
{
    if (m_autoPollEnabled && m_communication &&
        m_communication->state() == PPBState::Ready && m_currentAddress != 0) {

        requestStatus(m_currentAddress);
    }
}

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================

void PPBController::processStatusData(uint16_t address, const QVector<QByteArray>& data)
{
    // Определяем индекс ППБ из адреса
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
        emit logMessage("Недостаточно данных статуса");
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

    emit logMessage(QString("Статус ППБ%1 обновлен").arg(index + 1));
}

UIChannelState PPBController::parseChannelData(const QVector<QByteArray>& channelData)
{
    UIChannelState state;

    // TODO: Реальный парсинг данных
    // Пока заглушка с детерминированными значениями
    static int counter = 0;
    state.power = 1250.0f + (counter % 100 - 50);
    state.temperature = 45.0f + (counter % 10 - 5);
    state.vswr = 1.2f + (counter % 10) / 50.0f;
    state.isOk = (counter % 10) != 0; // 90% вероятность "OK"

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
//++++++++++++++++++++++ВЫЗОВЫ ДЯЛ КОМАНД
void PPBController::requestVersion(uint16_t address)
{
    if (m_communication ) {
        emit executeCommandRequested(TechCommand::VERS, address);
        emit logMessage(QString("Запрос версии ППБ %1").arg(address));
    }
}

void PPBController::requestVolume(uint16_t address)
{
    if (m_communication ) {
        emit executeCommandRequested(TechCommand::VOLUME, address);
        emit logMessage(QString("Запрос тома ПО ППБ %1").arg(address));
    }
}

void PPBController::requestChecksum(uint16_t address)
{
    if (m_communication ) {
        emit executeCommandRequested(TechCommand::CHECKSUM, address);
        emit logMessage(QString("Запрос контрольной суммы ППБ %1").arg(address));
    }
}

void PPBController::sendProgram(uint16_t address)
{
    if (m_communication ) {
       emit executeCommandRequested(TechCommand::PROGRAMM, address);
        emit logMessage(QString("Обновление ПО ППБ %1").arg(address));
    }
}

void PPBController::sendClean(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::CLEAN, address);
        emit logMessage(QString("Очистка временного файла ПО ППБ %1").arg(address));
    }
}

void PPBController::requestDroppedPackets(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::DROP, address);
        emit logMessage(QString("Запрос отброшенных пакетов ППБ %1").arg(address));
    }
}

void PPBController::requestBER_T(uint16_t address)
{
    if (m_communication ) {
        emit executeCommandRequested(TechCommand::BER_T, address);
        emit logMessage(QString("Запрос BER ТУ ППБ %1").arg(address));
    }
}

void PPBController::requestBER_F(uint16_t address)
{
    if (m_communication) {
        emit executeCommandRequested(TechCommand::BER_F, address);
        emit logMessage(QString("Запрос BER ФУ ППБ %1").arg(address));
    }
}
//++++++++++++++++++++++++++++++++++++++
void PPBController::setCommunication(PPBCommunication* communication)
{
    LOG_DEBUG("PPBController::setCommunication: новый объект = " +
              QString::number((qulonglong)communication) +
              ", текущий = " + QString::number((qulonglong)m_communication));

    // Если передается тот же объект - ничего не делаем
    if (m_communication == communication) {
        LOG_DEBUG("PPBController::setCommunication: тот же объект, игнорируем");
        return;
    }

    // ========== ОТКЛЮЧЕНИЕ СТАРОГО КОММУНИКАЦИОННОГО ОБЪЕКТА ==========
    if (m_communication) {
        LOG_DEBUG("PPBController::setCommunication: отключаем старый объект");

        // 1. Останавливаем автоопрос, если он активен
        if (m_autoPollTimer && m_autoPollTimer->isActive()) {
            LOG_DEBUG("PPBController::setCommunication: останавливаем автоопрос");
            m_autoPollTimer->stop();
        }

        // 2. ОТКЛЮЧАЕМ ВСЕ СИГНАЛЫ Qt МЕЖДУ контроллером и communication
        QObject::disconnect(m_communication, nullptr, this, nullptr);
        QObject::disconnect(this, nullptr, m_communication, nullptr);

        // 3. Вызываем физическое отключение от ППБ (если нужно)
        if (m_communication->state() == PPBState::Ready) {
            LOG_DEBUG("PPBController::setCommunication: отключаемся от ППБ");
            m_communication->disconnect();
        }

        // 4. Удаляем старый объект
        LOG_DEBUG("PPBController::setCommunication: удаляем старый объект");
        m_communication->deleteLater();
        m_communication = nullptr;

        // 5. Сбрасываем внутренние состояния
        m_currentAddress = 0;
        m_channel1States.clear();
        m_channel2States.clear();

        // 6. Уведомляем UI об отключении
        emit connectionStateChanged(PPBState::Idle);
        emit logMessage("Коммуникационный объект заменен");
    }

    // ========== УСТАНОВКА НОВОГО ОБЪЕКТА ==========
    m_communication = communication;

    if (m_communication) {
        LOG_DEBUG("PPBController::setCommunication: настраиваем новый объект");

        // 1. Подключаем сигналы (используем метод из ШАГА 1)
        connectCommunicationSignals();

        // 2. Проверяем текущее состояние нового communication
        PPBState newState = m_communication->state();
        LOG_DEBUG(QString("PPBController::setCommunication: состояние нового объекта = %1")
                      .arg(static_cast<int>(newState)));

        // 3. Уведомляем UI о текущем состоянии
        emit connectionStateChanged(newState);

        // 4. Если автоопрос был включен - перезапускаем
        if (m_autoPollEnabled && m_autoPollTimer) {
            LOG_DEBUG("PPBController::setCommunication: перезапускаем автоопрос");
            m_autoPollTimer->start();
        }

        LOG_INFO("PPBController: коммуникационный объект успешно заменен");
        emit logMessage("Новый коммуникационный объект установлен");
    } else {
        LOG_WARNING("PPBController::setCommunication: передан nullptr");
        emit logMessage("Коммуникационный объект удален");
    }
}
void PPBController::setCurrentAddress(uint16_t address)
{
    if (m_currentAddress != address) {
        LOG_DEBUG(QString("PPBController: изменение текущего адреса: 0x%1 -> 0x%2")
                      .arg(m_currentAddress, 4, 16, QChar('0'))
                      .arg(address, 4, 16, QChar('0')));
        m_currentAddress = address;
    }
}
