#ifndef PPBCOMMUNICATION_H
#define PPBCOMMUNICATION_H

#include "udpclient.h"
#include "packetbuilder.h"
#include "logger.h"
#include <QObject>
#include <QTimer>
#include "commandinterface.h"
#include <QDateTime>
#include <QQueue>
#include <QSharedPointer>
#include <QWeakPointer>

class PPBCommand;

class PPBCommunication : public CommandInterface
{
    Q_OBJECT
    Q_PROPERTY(PPBState state READ state NOTIFY stateChanged)

public:
    explicit PPBCommunication(QObject* parent = nullptr);
    ~PPBCommunication();
    Q_INVOKABLE void initializeInThread();


    // === ОСНОВНЫЕ МЕТОДЫ ===
   Q_INVOKABLE  bool connectToPPB(uint16_t address, const QString& ip, quint16 port);
   Q_INVOKABLE  void disconnect();

   Q_INVOKABLE  void executeCommand(TechCommand cmd, uint16_t address); // основаная команда отпрвки

    // ФУ команды
   Q_INVOKABLE  void sendFUTransmit(uint16_t address);
   Q_INVOKABLE  void sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr);


    // Состояние
    PPBState state() const {
        QMutexLocker locker(&m_stateMutex);
        return m_state; }
    bool isBusy() const {
        return m_state == PPBState::SendingCommand ||
               m_state == PPBState::WaitingData;
    }


    //реализация интерфейса команд
    void setState(PPBState state) override;
    void startTimeoutTimer(int ms) override;
    void stopTimeoutTimer() override;
    void completeCurrentOperation(bool success, const QString& message = "") override;
    void sendPacket(const QByteArray& packet, const QString& description) override;



   public slots:
    void sendDataPackets(const QVector<DataPacket>& packets) override;
    QVector<DataPacket> getGeneratedPackets() const override { return m_generatedPackets; }
void stop();
      void enqueueCommand(TechCommand cmd, uint16_t address);
    void initialize(UDPClient* udpClient); // Объединенный метод инициализации
    //void startAsyncPacketSending(); //асинхронная отправка
    //void sendDataPacketsSync(const QVector<DataPacket>& packets); //синхронная отправка (теееееееест)
signals:

    // Сигналы состояния
    void stateChanged(PPBState state);
    void connected();
    void disconnected();
    void busyChange(bool busy);
    // Сигналы данных
    void statusReceived(uint16_t address, const QVector<QByteArray>& data);
    void commandProgress(int current, int total,  TechCommand command);
    void commandCompleted(bool success, const QString& report, TechCommand command);

    // Сигналы ошибок
    void errorOccurred(const QString& error);

    // Сигналы для логов
    void logMessage(const QString& message);

    void initialized(); // Новый сигнал

private slots:
    void onDataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);
    void onNetworkError(const QString& error);
    void onOperationTimeout();
    void sendNextPacket();  // Для отправки последовательности пакетов

     void setUDPClient(UDPClient* udpClient);

    void processNextTask();

private:


    // === ВНУТРЕННИЕ МЕТОДЫ ===

    // Управление состоянием

    void setError(const QString& error);
    void completeOperation(bool success, const QString& message = "");

    // Обработка входящих данных
    void processDataPacket(const DataPacket& packet);
    void processBridgeResponse(const BridgeResponse& response);
    void processPPBResponse(const PPBResponse& response);

    void startPacketSending();


private:
   QWeakPointer<UDPClient> m_udpClientWeak; // Слабая ссылка
    struct CommandTask {
        TechCommand cmd;
        uint16_t address;
        QDateTime timestamp;
    };
    QQueue<CommandTask> m_taskQueue;
    bool m_processingTask = false;

    UDPClient* m_udpClient;

    //состояние
    PPBState m_state;

    // Текущая команда (ОБЪЕКТ)
    std::unique_ptr<PPBCommand> m_currentCommand;

    // Текущая данные о ппб
    uint16_t m_currentAddress;
    QString m_currentIP;
    quint16 m_currentPort;

    // Таймеры
    QTimer* m_timeoutTimer;
    QTimer* m_packetTimer;

    // Счетчики для тестов
    int m_expectedPackets;
    int m_receivedPacketCount;

    // Флаги
    bool m_waitingForOk;          // Ждем OK на обычную команду

    //ДАННЫЕ
     QVector<DataPacket> m_generatedPackets;  // Для хранения сгенерированных данных
     QVector<QByteArray> m_receivedData;     //Экземпляр полученных пакетов

    // Ошибки
    QString m_lastError;

    //поля асинхронной отправки пакетов
    QVector<DataPacket> m_packetsQueue;
    int m_currentPacketIndex;

    mutable QMutex m_mutex;
    mutable QMutex m_stateMutex;
};

#endif // PPBCOMMUNICATION_H
