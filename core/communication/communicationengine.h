#ifndef COMMUNICATIONENGINE_H
#define COMMUNICATIONENGINE_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMap>
#include <deque>
#include <list>
#include <memory>

#include "udpclient.h"
#include "packetbuilder.h"
#include "commandandoperation.h"

namespace Internal {
class StateManager : public QObject {
    Q_OBJECT



public:
    explicit StateManager(QObject* parent = nullptr);
    PPBState getState(uint16_t address) const;
    void setState(uint16_t address, PPBState state);

    void clear();

    QMap<uint16_t, PPBState> states() const {
        QMutexLocker locker(&m_mutex);
        return m_states;
    }

signals:
    void stateChanged(uint16_t address, PPBState state);

private:
    mutable QMutex m_mutex;
    QMap<uint16_t, PPBState> m_states;
};

class CommandQueue : public QObject {
    Q_OBJECT
public:
    explicit CommandQueue(QObject* parent = nullptr);
    ~CommandQueue();

    void enqueue(uint16_t address, std::unique_ptr<PPBCommand> cmd);
    std::unique_ptr<PPBCommand> dequeue(uint16_t address);
    bool isEmpty(uint16_t address) const;
    void clear();

    // Получить список всех адресов, для которых есть очереди
    QList<uint16_t> addresses() const;

signals:
    void queueChanged(uint16_t address, int size);

private:
    mutable QMutex m_mutex;

    // Используем std::unordered_map
    std::unordered_map<uint16_t, std::deque<std::unique_ptr<PPBCommand>>> m_queues;
};

} // namespace Internal

class communicationengine : public QObject
{
    Q_OBJECT

private:

    struct PPBContext {
        std::unique_ptr<PPBCommand> currentCommand;
        QVector<QByteArray> receivedData;
        QVector<DataPacket> generatedPackets;
        QVector<DataPacket> receivedPackets;
        int packetsExpected = 0;
        int packetsReceived = 0;
        bool waitingForOk = false;
        QTimer* operationTimer = nullptr;

        // Конструкторы и операторы
        PPBContext() = default;
        PPBContext(const PPBContext&) = delete;
        PPBContext& operator=(const PPBContext&) = delete;
        PPBContext(PPBContext&& other) noexcept
            : currentCommand(std::move(other.currentCommand))
            , receivedData(std::move(other.receivedData))
            , generatedPackets(std::move(other.generatedPackets))
            , receivedPackets(std::move(other.receivedPackets))
            , packetsExpected(other.packetsExpected)
            , packetsReceived(other.packetsReceived)
            , waitingForOk(other.waitingForOk)
            , operationTimer(other.operationTimer)
        {
            other.operationTimer = nullptr;
        }

        PPBContext& operator=(PPBContext&& other) noexcept {
            if (this != &other) {
                if (operationTimer) {
                    delete operationTimer;
                }

                currentCommand = std::move(other.currentCommand);
                receivedData = std::move(other.receivedData);
                generatedPackets = std::move(other.generatedPackets);
                receivedPackets = std::move(other.receivedPackets);
                packetsExpected = other.packetsExpected;
                packetsReceived = other.packetsReceived;
                waitingForOk = other.waitingForOk;
                operationTimer = other.operationTimer;
                other.operationTimer = nullptr;
            }
            return *this;
        }

        ~PPBContext() {
            if (operationTimer) {
                operationTimer->stop();
                delete operationTimer;
            }
        }
    };

public:
    explicit communicationengine(UDPClient* udpClient, QObject* parent = nullptr);
    ~communicationengine();

    // Основные методы
    bool connectToPPB(uint16_t address, const QString& ip, quint16 port);
    void disconnect();
    void executeCommand(TechCommand cmd, uint16_t address);

    // ФУ команды
    void sendFUTransmit(uint16_t address);
    void sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr);

public:
    void setCommandInterface(CommandInterface* cmdInterface) {
        m_commandInterface = cmdInterface;
    }
    void sendPacketInternal(const QByteArray& packet, const QString& description);
public slots:
   // void forceCompleteOperation(bool success, const QString& message);
signals:
    void stateChanged(uint16_t address, PPBState state);
    void connected();
    void disconnected();
    void commandCompleted(bool success, const QString& report, TechCommand command);
    void errorOccurred(const QString& error);
    void logMessage(const QString& message);

    void statusDataReady(const QVector<QByteArray>& data);
    void testDataReady(const QVector<DataPacket>& data);
    void commandProgress(int current, int total);

private slots:
    void onDataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);
    void onNetworkError(const QString& error);
    void onOperationTimeout(uint16_t address);
    void processCommandQueue();

private:
    void executeCommandImmediately(uint16_t address, std::unique_ptr<PPBCommand> command);
    void processPPBResponse(const PPBResponse& response);
    void processBridgeResponse(const BridgeResponse& response);
    void processDataPacket(const DataPacket& packet);
    void clearContext(uint16_t address);
    PPBContext* getContext(uint16_t address);
private:

    UDPClient* m_udpClient;
    QTimer* m_queueTimer;
    QTimer* m_timeoutTimer;
     std::unordered_map<uint16_t, PPBContext> m_contexts;



    QMutex m_contextsMutex;
    uint16_t m_currentAddress;
    QString m_currentIP;
    quint16 m_currentPort;
    mutable QMutex m_mutex;
    Internal::StateManager* m_stateManager;
    Internal::CommandQueue* m_commandQueue;
    CommandInterface* m_commandInterface;


};

#endif // COMMUNICATIONENGINE_H
