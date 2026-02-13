#ifndef PPBCONTROLLER_H
#define PPBCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QVariant>
#include "../analyzer/packetanalyzer_interface.h"
#include "../analyzer/analyzer_factory.h"
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
    explicit PPBController(PPBCommunication* communication, QObject *parent = nullptr);
    ~PPBController();
    PPBCommunication* m_communication;
    QThread* m_communicationThread;
    // API для TesterWindow
    Q_INVOKABLE void connectToPPB(uint16_t address, const QString& ip, quint16 port);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void requestStatus(uint16_t address);
    Q_INVOKABLE void resetPPB(uint16_t address);
    Q_INVOKABLE void setGeneratorParameters(uint16_t address, uint32_t duration, uint8_t duty, uint32_t delay);
    Q_INVOKABLE void setFUReceive(uint16_t address, uint8_t period = 0);
    Q_INVOKABLE void setFUTransmit(uint16_t address);

    Q_INVOKABLE void startPRBS_M2S(uint16_t address);
    Q_INVOKABLE void startPRBS_S2M(uint16_t address);
    Q_INVOKABLE void runFullTest(uint16_t address);

    // Автоопрос
    Q_INVOKABLE void startAutoPoll(int intervalMs = 5000);
    Q_INVOKABLE void stopAutoPoll();
    Q_INVOKABLE void setCurrentAddress(uint16_t address);

    // Геттеры состояния
    Q_INVOKABLE PPBState connectionState() const;
    Q_INVOKABLE bool isBusy() const;
    Q_INVOKABLE bool isAutoPollEnabled() const;
    Q_INVOKABLE UIChannelState getChannelState(uint8_t ppbIndex, int channel) const;

    // Команды для пульта
    Q_INVOKABLE void requestVersion(uint16_t address);
    Q_INVOKABLE void requestVolume(uint16_t address);
    Q_INVOKABLE void requestChecksum(uint16_t address);
    Q_INVOKABLE void sendProgram(uint16_t address);
    Q_INVOKABLE void sendClean(uint16_t address);
    Q_INVOKABLE void requestDroppedPackets(uint16_t address);
    Q_INVOKABLE void requestBER_T(uint16_t address);
    Q_INVOKABLE void requestBER_F(uint16_t address);
    Q_INVOKABLE void analize();

    void saveReceivedPackets(const QVector<DataPacket>& packets);
    void saveSentPackets(const QVector<DataPacket>& packets);
    void setCommunication(PPBCommunication* communication);

signals:
    // Сигналы Cont->Com
    void executeCommandRequested(TechCommand cmd, uint16_t address);
    void connectionStateChanged(PPBState state);
    void busyChanged(bool busy);
    void statusReceived(uint16_t address, const QVector<QByteArray>& data);
    void errorOccurred(const QString& error);
    void channelStateUpdated(uint8_t ppbIndex, int channel, const UIChannelState& state);
    void autoPollToggled(bool enabled);
    void connectToPPBSignal(uint16_t address, const QString& ip, quint16 port);
    void disconnectSignal();
    void sendFUReceiveSignal(uint16_t address, uint8_t period, const uint8_t fuData[3]);
    void sendFUTransmitSignal(uint16_t address);

    // Сигналы прогресса для UI
    void operationProgress(int current, int total, const QString& operation);
    void operationCompleted(bool success, const QString& message);

    // Сигналы анализа
    void analysisStarted();
    void analysisProgress(int percent);
    void analysisComplete(const QString& summary, const QVariantMap& details);

private slots:
    void onStatusReceived(uint16_t address, const QVector<QByteArray>& data);
    void onConnectionStateChanged(PPBState state);
    void onCommandProgress(int current, int total, TechCommand command);
    void onCommandCompleted(bool success, const QString& message, TechCommand command);
    void onErrorOccurred(const QString& error);
    void onAutoPollTimeout();
    void onBusyChanged(bool busy);

    // Слоты анализа
    void onSentPacketsSaved(const QVector<DataPacket>& packets);
    void onReceivedPacketsSaved(const QVector<DataPacket>& packets);
    void onClearPacketDataRequested();

    // Слоты для сигналов анализатора
    void onAnalyzerAnalysisStarted();
    void onAnalyzerAnalysisProgress(int percent);
    void onAnalyzerAnalysisComplete(const QString& summary);
    void onAnalyzerDetailedResultsReady(const QVariantMap& results);

private:
    void initializeCommunication();
    void initializeTimers();
    void processStatusData(uint16_t address, const QVector<QByteArray>& data);
    UIChannelState parseChannelData(const QVector<QByteArray>& channelData);
    QString commandToName(TechCommand command) const;

    // Методы анализа
    void connectCommunicationSignals();
    void showAnalysisResults(const QString& summary, const QVariantMap& details);
    void showPacketsTable(const QString& title, const QVector<DataPacket>& packets);


    QTimer* m_autoPollTimer;
    bool m_autoPollEnabled;

    // Хранение состояний каналов
    QMap<uint8_t, UIChannelState> m_channel1States;
    QMap<uint8_t, UIChannelState> m_channel2States;

    // Текущий адрес для автоопроса
    uint16_t m_currentAddress;

    bool busy;

    PacketAnalyzerInterface* m_packetAnalyzer;
    QVector<DataPacket> m_lastSentPackets;
    QVector<DataPacket> m_lastReceivedPackets;
};

#endif // PPBCONTROLLER_H
