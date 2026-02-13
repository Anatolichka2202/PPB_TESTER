#ifndef AKIP_MANAGER_H
#define AKIP_MANAGER_H

#include <QObject>
#include <QMutex>
#include <QScopedPointer>
#include "usbinterface.h"
#include "akip_scpi.h"

class AKIPManager : public QObject
{
    Q_OBJECT

public:
    explicit AKIPManager(QObject *parent = nullptr);
    ~AKIPManager();

    // ---------- Состояние и ошибки ----------
    bool isConnected() const;
    QString lastError() const;

public slots:
    // ---------- Управление подключением ----------
    void initialize();      // подключение + заводской пресет
    void shutdown();        // отключение

    // ---------- Высокоуровневые команды (основной API) ----------
    void setDutyCycle(double percent);               // канал B, скважность, %
    void setChannelAParameters(double powerDBm, double frequencyMHz); // мощность, дБм; частота, МГц

    // ---------- Прямые SCPI команды (для отладки / расширенного контроля) ----------
    void sendCommand(const QString &scpi);
    QString queryCommand(const QString &scpi);

signals:
    void connected();
    void disconnected();
    void error(const QString &message);
    void initialized();                 // после успешного применения заводского пресета
    void commandExecuted(const QString &command, bool success);
    void parameterChanged(const QString &param, double value); // например "freq_a", "duty_b"

private slots:
    void onDeviceError(const QString &error); // обработчик ошибок UsbInterface

private:
    // Внутренние служебные методы
    bool openConnection();
    void closeConnection();
    bool applyFactoryPreset();          // заводской пресет (при инициализации)
    void log(const QString &msg) const;

    // Кэш состояния устройства
    struct DeviceState {
        // Канал A
        bool chAEnabled = false;
        double chAFreqMHz = 500.0;
        double chAPowerDBm = 0.0;
        bool amEnabled = true;
        double amFreqHz = 20.0;
        double amDepth = 100.0;

        // Канал B
        bool chBEnabled = false;
        double chBAmplitudeVpp = 4.0;
        double chBDutyPercent = 33.333;

        // Дополнительно: нагрузка, синхронизация...
    };

    // Объекты
    UsbInterface *m_usb;
    mutable QMutex m_mutex;
    QString m_lastError;
    bool m_initialized;     // флаг, что пресет был применён хотя бы раз
    DeviceState m_state;
};

#endif // AKIP_MANAGER_H
