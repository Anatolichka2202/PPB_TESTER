#ifndef COMMANDSEQUENCER_H
#define COMMANDSEQUENCER_H

#include "packetbuilder.h"
#include "logger.h"
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>
#include <QHostAddress>

class UDPClient;

class CommandSequencer : public QObject
{
    Q_OBJECT

public:
    explicit CommandSequencer(UDPClient* udpClient, QObject* parent = nullptr);
    ~CommandSequencer();

    // === ВЫСОКОУРОВНЕВЫЕ ЦЕПОЧКИ КОМАНД ===

    // 1. Проверка связи (TS команда + прием статуса)
    void executeConnectionTest(uint16_t address, const QString& ip, quint16 port);

    // 2. Полный тест (M2S → данные → S2M → прием → сравнение)
    void executeFullTest(uint16_t address, const QString& ip, quint16 port);

    // 3. Тест передачи (M2S → отправка данных)
    void executeTransmitTest(uint16_t address, const QString& ip, quint16 port);

    // 4. Тест приема (S2M → прием данных)
    void executeReceiveTest(uint16_t address, const QString& ip, quint16 port);

    // 5. Опрос состояния (TS → прием 7 пакетов)
    void executeStatusRequest(uint16_t address, const QString& ip, quint16 port);

    // 6. Сброс ППБ (TC команда)
    void executeReset(uint16_t address, const QString& ip, quint16 port);

    // === ОДИНОЧНЫЕ КОМАНДЫ ===

    // ТУ команды (любая из TechCommand)
    void executeTUCommand(TechCommand cmd, uint16_t address, const QString& ip, quint16 port);

    // ФУ команды
    void executeFUCommand(uint16_t address, bool isTransmit, uint8_t period,
                          const uint8_t fuData[3], const QString& ip, quint16 port);

    // === УПРАВЛЕНИЕ ===
    void stopCurrentSequence();
    bool isRunning() const { return m_isRunning; }

signals:
    // Прогресс выполнения цепочки
    void sequenceStarted(const QString& sequenceName);
    void sequenceStep(const QString& stepName, int currentStep, int totalSteps);
    void sequenceProgress(int percentage);

    // Результаты
    void sequenceCompleted(bool success, const QString& result);
    void statusDataReceived(uint16_t address, const QVector<QByteArray>& dataPackets);
    void testDataResult(const QString& analysisReport);

    // Ошибки
    void sequenceError(const QString& error);
    void timeoutOccurred(const QString& operation);

private slots:
    void onDataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);
    void onTimeout();
    void onSendNextTestPacket();

private:
    // === ВНУТРЕННИЕ КОНСТАНТЫ ===
    enum class SequenceType {
        None,
        ConnectionTest,
        FullTest,
        TransmitTest,
        ReceiveTest,
        StatusRequest,
        Reset,
        TUCommand,
        FUCommand
    };

    struct TestPacket {
        DataPacket packet;
        bool sent;
        bool received;
        bool matched;
    };

    // === МЕТОДЫ УПРАВЛЕНИЯ ЦЕПОЧКАМИ ===
    void startSequence(SequenceType type, const QString& name, uint16_t address,
                       const QString& ip, quint16 port);
    void completeSequence(bool success, const QString& result = "");

    // Шаги для каждой цепочки
    void processConnectionTest();
    void processFullTest();
    void processTransmitTest();
    void processReceiveTest();
    void processStatusRequest();
    void processTUCommand();
    void processFUCommand();

    // Общие шаги
    void sendTUCommandWithResponse(TechCommand cmd);
    void waitForDataPackets(int expectedCount, int timeoutPerPacketMs = 1000);
    void generateTestPackets(int count);
    void sendTestPackets();

    // Обработка ответов
    void handlePPBResponse(const PPBResponse& response);
    void handleBridgeResponse(const BridgeResponse& response);
    void handleDataPacket(const DataPacket& packet);

    // Утилиты
    void sendPacket(const QByteArray& packet, const QString& description);
    void startStepTimer(int ms);
    void stopStepTimer();
    QString sequenceTypeToString(SequenceType type) const;

    // Генерация тестовых данных
    uint8_t lfsrNext(uint8_t &lfsr);

private:
    UDPClient* m_udpClient;

    // Текущая цепочка
    SequenceType m_currentSequence;
    QString m_sequenceName;
    uint16_t m_currentAddress;
    QString m_currentIP;
    quint16 m_currentPort;
    int m_currentStep;
    int m_totalSteps;
    bool m_isRunning;

    // Таймеры
    QTimer m_stepTimer;
    QTimer m_testPacketTimer;
    QElapsedTimer m_sequenceTimer;

    // Состояние для тестовых последовательностей
    QVector<TestPacket> m_testPackets;
    int m_expectedPackets;
    int m_packetsSent;
    int m_packetsReceived;

    // Состояние для статуса
    QVector<QByteArray> m_statusPackets;

    // Флаги
    bool m_waitingForResponse;
    TechCommand m_lastTUCommand;
};

#endif // COMMANDSEQUENCER_H
