#ifndef PPBCONTROLLER_H
#define PPBCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include "../core/communication/ppbcommunication.h"
#include "../core/utilits/dataconverter.h"

struct UIChannelState {
    float power;
    float temperature;
    float vswr;
    bool isOk;
};

class PPBController : public QObject
{
    Q_OBJECT

public:
    explicit PPBController(PPBCommunication* communication , QObject *parent = nullptr);
    ~PPBController();
    PPBCommunication* m_communication;
    QThread* m_communicationThread;  // Добавляем указатель на поток

    // API для TesterWindow
   Q_INVOKABLE  void connectToPPB(uint16_t address, const QString& ip, quint16 port);
   Q_INVOKABLE  void disconnect();
   Q_INVOKABLE  void requestStatus(uint16_t address);
   Q_INVOKABLE  void resetPPB(uint16_t address);
   Q_INVOKABLE  void setGeneratorParameters(uint16_t address, uint32_t duration, uint8_t duty, uint32_t delay);
   Q_INVOKABLE  void setFUReceive(uint16_t address, uint8_t period = 0);
   Q_INVOKABLE  void setFUTransmit(uint16_t address);

   Q_INVOKABLE  void startPRBS_M2S(uint16_t address);
   Q_INVOKABLE  void startPRBS_S2M(uint16_t address);
   Q_INVOKABLE  void runFullTest(uint16_t address);

    // Автоопрос
   Q_INVOKABLE  void startAutoPoll(int intervalMs = 5000);
   Q_INVOKABLE  void stopAutoPoll();

    // Геттеры состояния
   Q_INVOKABLE  PPBState connectionState() const;
   Q_INVOKABLE  bool isBusy() const;
   Q_INVOKABLE   bool isAutoPollEnabled() const;
   Q_INVOKABLE   UIChannelState getChannelState(uint8_t ppbIndex, int channel) const;

    // Команды для пульта
   Q_INVOKABLE  void requestVersion(uint16_t address);          // VERS
   Q_INVOKABLE  void requestVolume(uint16_t address);           // VOLUME
   Q_INVOKABLE  void requestChecksum(uint16_t address);         // CHECKSUM
   Q_INVOKABLE  void sendProgram(uint16_t address);             // PROGRAMM
   Q_INVOKABLE  void sendClean(uint16_t address);               // CLEAN
   Q_INVOKABLE  void requestDroppedPackets(uint16_t address);   // DROP
   Q_INVOKABLE  void requestBER_T(uint16_t address);            // BER_T
   Q_INVOKABLE  void requestBER_F(uint16_t address);            // BER_F

   void setCommunication(PPBCommunication* communication);
signals:
   //сигналы Cont->Com
    void executeCommandRequested(TechCommand cmd, uint16_t address);
    void connectionStateChanged(PPBState state);
    void busyChanged(bool busy);
    void statusReceived(uint16_t address, const QVector<QByteArray>& data);
    void errorOccurred(const QString& error);
    void logMessage(const QString& message);
    void channelStateUpdated(uint8_t ppbIndex, int channel, const UIChannelState& state);
    void autoPollToggled(bool enabled);
    void connectToPPBSignal(uint16_t address, const QString& ip, quint16 port);
    void disconnectSignal();
    void sendFUReceiveSignal(uint16_t address, uint8_t period, const uint8_t fuData[3]);
    void sendFUTransmitSignal(uint16_t addres);

    // Сигналы прогресса для UI
    void operationProgress(int current, int total, const QString& operation);
    void operationCompleted(bool success, const QString& message);
private slots:
    void onStatusReceived(uint16_t address, const QVector<QByteArray>& data);
    void onConnectionStateChanged(PPBState state);
    void onCommandProgress(int current, int total, TechCommand command);
    void onCommandCompleted(bool success, const QString& message, TechCommand command);
    void onErrorOccurred(const QString& error);
    void onAutoPollTimeout();
    void onBusyChanged(bool busy);  // Новый слот для busyChanged
private:
    void initializeCommunication();
    void initializeTimers();
    void processStatusData(uint16_t address, const QVector<QByteArray>& data);
    UIChannelState parseChannelData(const QVector<QByteArray>& channelData);
    QString commandToName(TechCommand command) const;

    //PPBCommunication* m_communication;
    QTimer* m_autoPollTimer;
    bool m_autoPollEnabled;

    // Хранение состояний каналов
    QMap<uint8_t, UIChannelState> m_channel1States;
    QMap<uint8_t, UIChannelState> m_channel2States;

    // Текущий адрес для автоопроса
    uint16_t m_currentAddress;

    bool busy=false;
};

#endif // PPBCONTROLLER_H
