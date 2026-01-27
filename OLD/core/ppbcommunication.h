#ifndef PPBCOMMUNICATION_H
#define PPBCOMMUNICATION_H

#include "udpclient.h"
#include "packetbuilder.h"
#include "logger.h"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

// Простые состояния
enum class PPBState {
    Disconnected,      // Нет подключения
    Connected,         // Подключено к ППБ
    Connecting,        // Подключение...
    SendingCommand,    // Отправлена команда, ждем OK
    WaitingData,       // Получили OK, ждем данные
    ProcessingTest,    // Выполняется тестовая последовательность
    Error              // Ошибка
};

// Типы операций
enum class OperationType {
    None,
    StatusRequest,     // Команда TS
    Reset,             // Команда TC
    TransmitTest,      // PRBS_M2S + отправка данных
    ReceiveTest,       // PRBS_S2M + прием данных
    FullTest           // Полный тестовый цикл
};

class PPBCommunication : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PPBState state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)

public:
    explicit PPBCommunication(QObject* parent = nullptr);
    ~PPBCommunication();

    // === ОСНОВНЫЕ МЕТОДЫ ===
    bool connectToPPB(uint16_t address, const QString& ip, quint16 port);
    void disconnect();

    // Простые команды (каждая запускает свою операцию)
    void sendStatusRequest(uint16_t address);
    void sendReset(uint16_t address);
    void sendTransmitTest(uint16_t address);
    void sendReceiveTest(uint16_t address);
    void sendFullTest(uint16_t address);

    // ФУ команды
    void sendFUTransmit(uint16_t address);
    void sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr);

    // Состояние
    PPBState state() const { return m_state; }
    bool isBusy() const {
        return m_state != PPBState::Disconnected &&
               m_state != PPBState::Connected;
    }
    QString lastError() const { return m_lastError; }

    // Данные
    QVector<DataPacket> sentTestPackets() const { return m_sentPackets; }
    QVector<DataPacket> receivedTestPackets() const { return m_receivedPackets; }
    QVector<QByteArray> statusData() const { return m_statusData; }

signals:
    // Сигналы состояния
    void stateChanged(PPBState state);
    void busyChanged(bool busy);
    void connected();
    void disconnected();

    // Сигналы данных
    void statusReceived(uint16_t address, const QVector<QByteArray>& data);
    void testProgress(int current, int total, OperationType operation);
    void testCompleted(bool success, const QString& report, OperationType operation);
    void testPacketReceived(const DataPacket& packet);

    // Сигналы ошибок
    void errorOccurred(const QString& error);
    void timeoutOccurred(const QString& operation);

    // Сигналы для логов
    void logMessage(const QString& message);

private slots:
    void onDataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);
    void onNetworkError(const QString& error);
    void onOperationTimeout();

private:
    // === ВНУТРЕННИЕ МЕТОДЫ ===
 TechCommand m_lastTechCommand;
    // Управление состоянием
    void setState(PPBState state);
    void setError(const QString& error);
    void startOperation(OperationType operation, uint16_t address);
    void completeOperation(bool success, const QString& message = "");

    // Отправка пакетов
    void sendPacket(const QByteArray& packet, const QString& description);
    void sendTestDataPackets();

    // Обработка входящих данных
    void processOkResponse(const OkResponse& response);
    void processDataPacket(const DataPacket& packet);
    void processStatusData(const DataPacket& packet);
    void processTestData(const DataPacket& packet);

    void processBridgeResponse(const BridgeResponse& response);
    void processPPBResponse(const PPBResponse& response);

    // Генерация тестовых данных
    QVector<DataPacket> generateTestPackets(int count);

    // Таймеры
    void startTimeoutTimer(int ms);
    void stopTimeoutTimer();

    // Логирование
    void log(const QString& message, const QString& level = "INFO");

    void attemptRecovery();

    void retryLastCommand();

    uint8_t lfsrNext(uint8_t &lfsr);

private:
    UDPClient* m_udpClient;
    PPBState m_state;

    // Текущая операция
    OperationType m_operation;
    uint16_t m_currentAddress;
    QString m_currentIP;
    quint16 m_currentPort;

    // Таймеры
    QTimer* m_timeoutTimer;
    QElapsedTimer m_operationTimer;

    // Данные операций
    QVector<DataPacket> m_sentPackets;
    QVector<DataPacket> m_receivedPackets;
    QVector<QByteArray> m_statusData;

    // Счетчики для тестов
    int m_expectedPackets;
    int m_receivedPacketCount;
    int m_sentPacketCount;

    // Флаги
    bool m_waitingForConnection;  // Ждем ответ на подключение
    bool m_waitingForOk;          // Ждем OK на обычную команду
    bool m_testDataSent;

    QVector<QPair<TechCommand, QByteArray>> m_retryQueue;
    int m_maxRetries = 3;
    int m_currentRetry = 0;
    bool m_isProcessingRetry = false;

    void enqueueRetry(TechCommand command, const QByteArray& packet)
    {
        if (m_currentRetry < m_maxRetries) {
            m_retryQueue.append(qMakePair(command, packet));
            m_isProcessingRetry = true;
            m_currentRetry++;

            log(QString("Команда поставлена в очередь повтора (%1/%2)")
                    .arg(m_currentRetry).arg(m_maxRetries));

            // Запускаем повтор через 2 секунды
            QTimer::singleShot(2000, this, [this, command, packet]() {
                if (m_isProcessingRetry) {
                    sendPacket(packet, QString("Повтор команды %1").arg((int)command));
                }
            });
        } else {
            log("Исчерпаны попытки повтора", "ERROR");
            m_isProcessingRetry = false;
            m_currentRetry = 0;
            m_retryQueue.clear();
        }
    }

    void clearRetryQueue()
    {
        m_isProcessingRetry = false;
        m_currentRetry = 0;
        m_retryQueue.clear();
    }


    // Ошибки
    QString m_lastError;


};

#endif // PPBCOMMUNICATION_H
