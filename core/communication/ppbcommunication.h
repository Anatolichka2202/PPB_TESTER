#ifndef PPBCOMMUNICATION_H
#define PPBCOMMUNICATION_H

#include "udpclient.h"
#include "packetbuilder.h"
#include "../logger.h"
#include <QObject>
#include <QTimer>
#include "commandinterface.h"
#include <QDateTime>
#include <QQueue>
#include <QSharedPointer>
#include <QWeakPointer>
#include "communicationengine.h"

class PPBCommand;

class PPBCommunication : public CommandInterface
{
    Q_OBJECT
    Q_PROPERTY(PPBState state READ state NOTIFY stateChanged)

public:
    explicit PPBCommunication(QObject* parent = nullptr);
    ~PPBCommunication();

    // === ОСНОВНЫЕ МЕТОДЫ ===
    bool connectToPPB(uint16_t address, const QString& ip, quint16 port);
    void disconnect();

    // Выполнение команды ТУ
    void executeCommand(TechCommand cmd, uint16_t address);

    // ФУ команды
    void sendFUTransmit(uint16_t address);
    void sendFUReceive(uint16_t address, uint8_t period, const uint8_t fuData[3] = nullptr);

    // Состояние
    PPBState state() const {
        QMutexLocker locker(&m_stateMutex);
        return m_state;
    }

    bool isBusy() const {
        QMutexLocker locker(&m_stateMutex);
        return m_state == PPBState::SendingCommand ||
               m_state == PPBState::WaitingData;
    }

    // Реализация интерфейса команд (упрощённая - делегирует движку)
    void setState(PPBState state) override;
    void startTimeoutTimer(int ms) override;
    void stopTimeoutTimer() override;
    void completeCurrentOperation(bool success, const QString& message = "") override;
    void sendPacket(const QByteArray& packet, const QString& description) override;

public slots:
    // Инициализация (должна вызываться в потоке объекта)
    void initialize(UDPClient* udpClient);

    // Остановка всех операций
    void stop();

    // Постановка команды в очередь (для асинхронного выполнения)
    void enqueueCommand(TechCommand cmd, uint16_t address);

    // Реализация интерфейса CommandInterface
    void sendDataPackets(const QVector<DataPacket>& packets) override;
    QVector<DataPacket> getGeneratedPackets() const override;

signals:
    // Сигналы состояния
    void stateChanged(PPBState state);
    void connected();
    void disconnected();
    void busyChange(bool busy);

    // Сигналы данных
    void statusReceived(uint16_t address, const QVector<QByteArray>& data);
    void commandProgress(int current, int total, TechCommand command);
    void commandCompleted(bool success, const QString& report, TechCommand command);

    // Сигналы ошибок
    void errorOccurred(const QString& error);

    // Сигналы для логов
    void logMessage(const QString& message);

    // Сигнал завершения инициализации
    void initialized();

private slots:
    // Слоты для обработки событий от движка
    void onEngineStateChanged(uint16_t address, PPBState state);
    void onEngineCommandCompleted(bool success, const QString& report, TechCommand command);
    //void onEngineCommandProgress(int current, int total, TechCommand command);
    void onEngineErrorOccurred(const QString& error);
    void onEngineLogMessage(const QString& message);

    // Обработка очереди команд (устаревшее, но оставляем для совместимости)
    void processNextTask();

private:
    // Установка состояния (с синхронизацией)
    void setStateInternal(PPBState state);

    // Установка ошибки
    void setError(const QString& error);

    // Структура для задачи в очереди (для совместимости)
    struct CommandTask {
        TechCommand cmd;
        uint16_t address;
        QDateTime timestamp;
    };

    // Основной движок обработки команд
    std::unique_ptr<communicationengine> m_engine;

    // Текущее состояние (синхронизируется мьютексом)
    PPBState m_state;
    mutable QMutex m_stateMutex;

    // UDP клиент (слабая ссылка, т.к. управляется извне)
    UDPClient* m_udpClient;

    // Текущие настройки подключения
    uint16_t m_currentAddress;
    QString m_currentIP;
    quint16 m_currentPort;

    // Очередь команд (для обратной совместимости)
    QQueue<CommandTask> m_taskQueue;
    bool m_processingTask;

    // Таймер для обработки очереди (устаревшее)
    QTimer* m_taskTimer;

    // Сгенерированные пакеты (для тестовых последовательностей)
    QVector<DataPacket> m_generatedPackets;

    // Последняя ошибка
    QString m_lastError;
};

#endif // PPBCOMMUNICATION_H
