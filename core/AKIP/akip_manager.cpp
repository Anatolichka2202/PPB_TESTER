#include "akip_manager.h"
#include <QThread>
#include <QElapsedTimer>

#include "../logging/logging_unified.h"

#define AKIP_LOG(msg)   LOG_INFO("[AKIP] " + QString(msg))
#define AKIP_WARN(msg)  LOG_WARNING("[AKIP] " + QString(msg))
#define AKIP_ERR(msg)   LOG_ERROR("[AKIP] " + QString(msg))

AKIPManager::AKIPManager(QObject *parent)
    : QObject(parent)
    , m_usb(new UsbInterface(this))
    , m_initialized(false)
{
    // Подключаем сигнал ошибки от USB-интерфейса
    connect(m_usb, &UsbInterface::errorOccurred,
            this, &AKIPManager::onDeviceError);
}

AKIPManager::~AKIPManager()
{
    shutdown();
}

bool AKIPManager::isConnected() const
{
    QMutexLocker locker(&m_mutex);
    return m_usb->isOpen();
}

QString AKIPManager::lastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

// ---------- Приватные методы ----------

bool AKIPManager::openConnection()
{
    QMutexLocker locker(&m_mutex);
    if (m_usb->isOpen()) {
        return true;
    }

    AKIP_LOG("Opening connection to AKIP-3417...");
    if (!m_usb->open(0)) {
        m_lastError = "Cannot open USB device";
        return false;
    }

    // Проверяем связь запросом IDN
    QString idn = m_usb->getIdentity();
    if (idn.isEmpty()) {
        m_lastError = "Device not responding to *IDN?";
        m_usb->close();
        return false;
    }

    AKIP_LOG("Connected to: " + idn);
    emit connected();
    return true;
}

void AKIPManager::closeConnection()
{
    QMutexLocker locker(&m_mutex);
    if (m_usb->isOpen()) {
        // Перед закрытием выключаем оба канала (безопасность)
        m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::A, false));
        m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::B, false));
        QThread::msleep(20); // даём время командам уйти
        m_usb->close();
        m_initialized = false;
        AKIP_LOG("Device disconnected");
        emit disconnected();
    }
}

bool AKIPManager::applyFactoryPreset()
{
    AKIP_LOG("Applying factory preset...");

    // 1. Выключаем выходы (чтобы избежать артефактов)
    m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::A, false));
    m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::B, false));
    QThread::msleep(50);

    // ----- Канал B (прямоугольные импульсы) -----
    // Форма сигнала: SQUARE
    if (!m_usb->sendScpiCommand(AkipSCPI::function(AkipSCPI::Channel::B, "SQUARE"))) {
        AKIP_ERR("Failed to set CHB function to SQUARE");
        return false;
    }
    // Амплитуда: 4.0 Vpp
    if (!m_usb->sendScpiCommand(AkipSCPI::voltage(AkipSCPI::Channel::B, 4.0, "VPP"))) {
        AKIP_ERR("Failed to set CHB amplitude");
        return false;
    }
    // Скважность: 33.333%
    if (!m_usb->sendScpiCommand(AkipSCPI::dutyCycleB(33.333))) {
        AKIP_ERR("Failed to set CHB duty cycle");
        return false;
    }

    // ----- Канал A (синус + AM) -----
    // Форма сигнала: SIN
    if (!m_usb->sendScpiCommand(AkipSCPI::function(AkipSCPI::Channel::A, "SIN"))) {
        AKIP_ERR("Failed to set CHA function to SIN");
        return false;
    }
    // Частота: 500 МГц
    if (!m_usb->sendScpiCommand(AkipSCPI::frequency(AkipSCPI::Channel::A, 500.0, "MHz"))) {
        AKIP_ERR("Failed to set CHA frequency");
        return false;
    }
    // Мощность: 0 dBm
    if (!m_usb->sendScpiCommand(AkipSCPI::voltage(AkipSCPI::Channel::A, 0.0, "DBM"))) {
        AKIP_ERR("Failed to set CHA power");
        return false;
    }

    // Настройка AM модуляции
    if (!m_usb->sendScpiCommand(AkipSCPI::amSource("INT"))) {
        AKIP_ERR("Failed to set AM source");
        return false;
    }
    if (!m_usb->sendScpiCommand(AkipSCPI::amInternalFrequency(20.0))) {
        AKIP_ERR("Failed to set AM internal frequency");
        return false;
    }
    if (!m_usb->sendScpiCommand(AkipSCPI::amDepth(100.0))) {
        AKIP_ERR("Failed to set AM depth");
        return false;
    }
    // Форма модуляции: SIN (по умолчанию) – можно отправить для гарантии
    m_usb->sendScpiCommand(AkipSCPI::amInternalFunction("SIN"));

    // Включаем выход канала A (канал B остаётся выключен)
    if (!m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::A, true))) {
        AKIP_ERR("Failed to enable CHA output");
        return false;
    }

    // ----- Обновляем кэш состояния -----
    {
        QMutexLocker locker(&m_mutex);
        m_state.chAEnabled = true;
        m_state.chAFreqMHz = 500.0;
        m_state.chAPowerDBm = 0.0;
        m_state.amEnabled = true;
        m_state.amFreqHz = 20.0;
        m_state.amDepth = 100.0;

        m_state.chBEnabled = false;
        m_state.chBAmplitudeVpp = 4.0;
        m_state.chBDutyPercent = 33.333;
    }

    AKIP_LOG("Factory preset applied successfully");
    return true;
}

void AKIPManager::log(const QString &msg) const
{
    // Здесь можно использовать qDebug() или ваш логгер
    qDebug().noquote() << "[AKIP]" << msg;
}

// ---------- Публичные слоты ----------

void AKIPManager::initialize()
{
    if (!openConnection()) {
        emit error(m_lastError);
        return;
    }

    if (applyFactoryPreset()) {
        m_initialized = true;
        emit initialized();
    } else {
        emit error("Failed to apply factory preset");
    }
}

void AKIPManager::shutdown()
{
    closeConnection();
}

void AKIPManager::setDutyCycle(double percent)
{
    QMutexLocker locker(&m_mutex);
    if (!m_usb->isOpen()) {
        AKIP_WARN("Cannot set duty cycle: device not connected");
        return;
    }

    // Ограничим разумными пределами
    percent = qBound(0.0, percent, 100.0);

    // Если значение не изменилось – пропускаем
    if (qFuzzyCompare(m_state.chBDutyPercent, percent)) {
        AKIP_LOG("Duty cycle already set to " + QString::number(percent) + "%, skipping");
        return;
    }

    AKIP_LOG("Setting CHB duty cycle to " + QString::number(percent) + "%");

    // Последовательность: выключить -> изменить -> включить (если был включён)
    bool wasEnabled = m_state.chBEnabled;
    if (wasEnabled) {
        m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::B, false));
        QThread::msleep(20);
    }

    bool ok = m_usb->sendScpiCommand(AkipSCPI::dutyCycleB(percent));
    emit commandExecuted(AkipSCPI::dutyCycleB(percent), ok);

    if (ok) {
        m_state.chBDutyPercent = percent;
        emit parameterChanged("duty_b", percent);
    }

    if (wasEnabled) {
        m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::B, true));
        QThread::msleep(20);
    }

    if (!ok) {
        AKIP_ERR("Failed to set duty cycle");
    }
}

void AKIPManager::setChannelAParameters(double powerDBm, double frequencyMHz)
{
    QMutexLocker locker(&m_mutex);
    if (!m_usb->isOpen()) {
        AKIP_WARN("Cannot set CHA parameters: device not connected");
        return;
    }

    // Проверка границ (можно подстроить под реальные ограничения прибора)
    powerDBm = qBound(-136.0, powerDBm, 30.0);   // примерный диапазон
    frequencyMHz = qBound(0.01, frequencyMHz, 500.0); // до 500 МГц

    bool powerChanged = !qFuzzyCompare(m_state.chAPowerDBm, powerDBm);
    bool freqChanged  = !qFuzzyCompare(m_state.chAFreqMHz, frequencyMHz);

    if (!powerChanged && !freqChanged) {
        AKIP_LOG("CHA parameters already set to requested values, skipping");
        return;
    }

    AKIP_LOG(QString("Setting CHA: power=%1 dBm, frequency=%2 MHz")
                 .arg(powerDBm).arg(frequencyMHz));

    bool wasEnabled = m_state.chAEnabled;
    if (wasEnabled) {
        m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::A, false));
        QThread::msleep(20);
    }

    bool success = true;
    if (freqChanged) {
        QString cmd = AkipSCPI::frequency(AkipSCPI::Channel::A, frequencyMHz, "MHz");
        success &= m_usb->sendScpiCommand(cmd);
        emit commandExecuted(cmd, success);
        if (success) {
            m_state.chAFreqMHz = frequencyMHz;
            emit parameterChanged("freq_a", frequencyMHz);
        }
    }
    if (powerChanged) {
        QString cmd = AkipSCPI::voltage(AkipSCPI::Channel::A, powerDBm, "DBM");
        success &= m_usb->sendScpiCommand(cmd);
        emit commandExecuted(cmd, success);
        if (success) {
            m_state.chAPowerDBm = powerDBm;
            emit parameterChanged("power_a", powerDBm);
        }
    }

    if (wasEnabled) {
        m_usb->sendScpiCommand(AkipSCPI::output(AkipSCPI::Channel::A, true));
        QThread::msleep(20);
    }

    if (!success) {
        AKIP_ERR("Failed to set CHA parameters");
    }
}

void AKIPManager::sendCommand(const QString &scpi)
{
    QMutexLocker locker(&m_mutex);
    if (!m_usb->isOpen()) {
        AKIP_WARN("Cannot send command: device not connected");
        return;
    }
    bool ok = m_usb->sendScpiCommand(scpi);
    emit commandExecuted(scpi, ok);
}

QString AKIPManager::queryCommand(const QString &scpi)
{
    QMutexLocker locker(&m_mutex);
    if (!m_usb->isOpen()) {
        AKIP_WARN("Cannot query: device not connected");
        return QString();
    }
    QString response = m_usb->queryScpiCommand(scpi);
    emit commandExecuted(scpi, !response.isEmpty());
    return response;
}

void AKIPManager::onDeviceError(const QString &error)
{
    AKIP_ERR("USB error: " + error);
    m_lastError = error;
    emit this->error(error);

    // При серьёзной ошибке можно принудительно сбросить состояние
    if (error.contains("not open", Qt::CaseInsensitive) ||
        error.contains("failed", Qt::CaseInsensitive)) {
        QMutexLocker locker(&m_mutex);
        // Отмечаем, что устройство более не открыто (UsbInterface сам закроет хендл)
        // Ничего не делаем, isOpen() вернёт false
    }
}
