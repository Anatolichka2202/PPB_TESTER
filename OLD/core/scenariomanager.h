/* #ifndef SCENARIOMANAGER_H
#define SCENARIOMANAGER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>
#include <QStringList>
#include "udpclient.h"
#include "packetbuilder.h"

class ScenarioManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int currentStep READ currentStep NOTIFY stepChanged)
    Q_PROPERTY(int totalSteps READ totalSteps NOTIFY totalStepsChanged)
    Q_PROPERTY(QString currentScenarioFile READ currentScenarioFile NOTIFY scenarioFileChanged)

public:
    explicit ScenarioManager(UDPClient* client, QObject* parent = nullptr);
    ~ScenarioManager() override;

    // Управление сценариями
    bool loadScenario(const QString& filename);
    void startScenario();
    void stopScenario();
    void pauseScenario();
    void resumeScenario();

    // Состояние
    bool isRunning() const;
    bool isPaused() const;
    int currentStep() const;
    int totalSteps() const;
    QString currentScenarioFile() const;

    // Настройки
    void setDefaultAddress(uint8_t address);
    void setDefaultPort(quint16 port);
    void setStepDelay(int ms);

    // Статистика
    int executionTime() const; // в миллисекундах

signals:
    // Сигналы состояния
    void scenarioStarted();
    void scenarioStopped();
    void scenarioPaused();
    void scenarioResumed();
    void scenarioFinished();//ОДИН ФИНИШ СИГНАЛ
    void runningChanged(bool running);

    // Сигналы выполнения
    void stepExecuted(int current, int total, const QString& description);
    void stepChanged(int current);
    void totalStepsChanged(int total);

    // Сигналы ошибок
    void scenarioError(const QString& error);
    void stepError(int step, const QString& error);

    // Сигналы файлов
    void scenarioFileChanged(const QString& filename);
    void scenarioLoaded(const QString& filename, int stepCount);

    // Сигналы для отправки команд (чтобы TesterWindow мог их логировать)
    void commandSent(const QByteArray& data, const QString& address, quint16 port);
    void broadcastSent(const QByteArray& data, quint16 port);

private slots:
    void executeNextStep();
    void onStepTimeout();
    void onNetworkError(const QString& error);

private:
    // Структура для хранения шага сценария
    struct ScenarioStep {
        QString rawLine;        // Исходная строка
        QString command;        // Команда (верхний регистр)
        QStringList params;     // Параметры
        QString description;    // Описание для лога
        int lineNumber;         // Номер строки в файле
    };

    // Вспомогательные методы
    bool parseScenarioLine(const QString& line, int lineNumber, ScenarioStep& step);
    bool executeStep(const ScenarioStep& step);
    void sendCommandToAddress(const QByteArray& data, uint8_t address);

    // Обработка команд сценария
    bool handlePollStatus(const ScenarioStep& step);
    bool handleSetParams(const ScenarioStep& step);
    bool handleReset(const ScenarioStep& step);
    bool handleFUMode(const ScenarioStep& step);
    bool handleDelay(const ScenarioStep& step);
    bool handleSetAddress(const ScenarioStep& step);
    bool handleComment(const ScenarioStep& step);

    // Сетевые методы
    void setupNetworkConnections();

private:
    UDPClient* m_client;
    QTimer* m_stepTimer;

    // Сценарий
    QString m_currentFile;
    QVector<ScenarioStep> m_steps;
    int m_currentStepIndex;
    bool m_isRunning;
    bool m_isPaused;
    void finishScenarioExecution();

    // Настройки
    uint8_t m_defaultAddress;
    quint16 m_defaultPort;
    int m_stepDelayMs;

    // Время выполнения
    QElapsedTimer m_executionTimer;

    // Статистика
    int m_successfulSteps;
    int m_failedSteps;
};

#endif // SCENARIOMANAGER_H
*/
