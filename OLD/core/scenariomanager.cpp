/*#include "scenariomanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QRegularExpression>

ScenarioManager::ScenarioManager(UDPClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_stepTimer(new QTimer(this))
    , m_currentStepIndex(0)
    , m_isRunning(false)
    , m_isPaused(false)
    , m_defaultAddress(PacketBuilder::BROADCAST_ADDRESS)
    , m_defaultPort(12345) // Порт по умолчанию
    , m_stepDelayMs(1000)  // Задержка между шагами 1 секунда
    , m_successfulSteps(0)
    , m_failedSteps(0)
{
    // Настройка таймера
    m_stepTimer->setSingleShot(true);
    connect(m_stepTimer, &QTimer::timeout, this, &ScenarioManager::onStepTimeout);

    // Настройка сетевых соединений
    setupNetworkConnections();
}

ScenarioManager::~ScenarioManager()
{
    stopScenario();
}

void ScenarioManager::setupNetworkConnections()
{
    if (m_client) {
        connect(m_client, &UDPClient::errorOccurred,
                this, &ScenarioManager::onNetworkError);
    }
}

bool ScenarioManager::loadScenario(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit scenarioError(QString("Не удалось открыть файл сценария: %1").arg(filename));
        return false;
    }

    // Очищаем текущий сценарий
    m_steps.clear();
    m_currentStepIndex = 0;

    QTextStream in(&file);
    int lineNumber = 0;
    int validSteps = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        // Пропускаем пустые строки
        if (line.isEmpty()) {
            continue;
        }

        // Парсим строку
        ScenarioStep step;
        if (parseScenarioLine(line, lineNumber, step)) {
            m_steps.append(step);
            validSteps++;
        }
    }

    file.close();

    if (validSteps == 0) {
        emit scenarioError("Файл сценария не содержит валидных команд");
        return false;
    }

    m_currentFile = filename;

    emit scenarioLoaded(filename, validSteps);
    emit totalStepsChanged(validSteps);
    emit scenarioFileChanged(filename);

    qDebug() << "ScenarioManager: Загружен сценарий" << filename
             << "с" << validSteps << "шагами";

    return true;
}

bool ScenarioManager::parseScenarioLine(const QString& line, int lineNumber, ScenarioStep& step)
{
    step.rawLine = line;
    step.lineNumber = lineNumber;

    // Удаляем комментарии (всё после #)
    QString cleanLine = line;
    int commentIndex = line.indexOf('#');
    if (commentIndex != -1) {
        cleanLine = line.left(commentIndex).trimmed();
        step.description = line.mid(commentIndex + 1).trimmed();
    }

    // Если после удаления комментария строка пуста
    if (cleanLine.isEmpty()) {
        step.command = "COMMENT";
        step.params.clear();
        return true; // Комментарии тоже считаем валидными шагами
    }

    // Разбиваем на команду и параметры
    QStringList parts = cleanLine.split(QRegularExpression("\\s+|,"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return false;
    }

    step.command = parts[0].toUpper();
    step.params.clear();

    for (int i = 1; i < parts.size(); i++) {
        step.params.append(parts[i]);
    }

    // Если описание не было извлечено из комментария, используем команду
    if (step.description.isEmpty()) {
        step.description = step.command;
    }

    return true;
}

void ScenarioManager::startScenario()
{
    if (m_isRunning) {
        emit scenarioError("Сценарий уже выполняется");
        return;
    }

    if (m_steps.isEmpty()) {
        emit scenarioError("Нет загруженного сценария");
        return;
    }

    m_currentStepIndex = 0;
    m_isRunning = true;
    m_isPaused = false;
    m_successfulSteps = 0;
    m_failedSteps = 0;

    m_executionTimer.start();

    emit scenarioStarted();
    emit runningChanged(true);
    emit stepChanged(0);

    qDebug() << "ScenarioManager: Запуск сценария, всего шагов:" << m_steps.size();

    // Запускаем первый шаг
    executeNextStep();
}

void ScenarioManager::stopScenario()
{
    if (!m_isRunning) {
        return;
    }

    m_stepTimer->stop();
    m_isRunning = false;
    m_isPaused = false;

    int executionTime = m_executionTimer.elapsed();

    emit scenarioStopped();
    emit runningChanged(false);

    qDebug() << "ScenarioManager: Сценарий остановлен. Время выполнения:"
             << executionTime << "мс, Успешных шагов:" << m_successfulSteps
             << ", Ошибок:" << m_failedSteps;
}

void ScenarioManager::pauseScenario()
{
    if (!m_isRunning || m_isPaused) {
        return;
    }

    m_stepTimer->stop();
    m_isPaused = true;

    emit scenarioPaused();

    qDebug() << "ScenarioManager: Сценарий приостановлен на шаге"
             << m_currentStepIndex + 1;
}

void ScenarioManager::resumeScenario()
{
    if (!m_isRunning || !m_isPaused) {
        return;
    }

    m_isPaused = false;

    emit scenarioResumed();

    // Продолжаем выполнение
    onStepTimeout();
}

void ScenarioManager::executeNextStep()
{
    // Проверяем, не завершился ли сценарий
    if (m_currentStepIndex >= m_steps.size()) {
        finishScenarioExecution();
        return;
    }

    // Выполняем текущий шаг
    const ScenarioStep& step = m_steps[m_currentStepIndex];

    bool success = executeStep(step);

    if (success) {
        m_successfulSteps++;
    } else {
        m_failedSteps++;
    }

    // Уведомляем о выполнении шага
    emit stepExecuted(m_currentStepIndex + 1, m_steps.size(), step.description);

    // Переходим к следующему шагу
    m_currentStepIndex++;
    emit stepChanged(m_currentStepIndex);

    // Если сценарий ещё не завершён, планируем следующий шаг
    if (m_currentStepIndex < m_steps.size() && m_isRunning && !m_isPaused) {
        m_stepTimer->start(m_stepDelayMs);
    } else if (m_currentStepIndex >= m_steps.size()) {
        // Сценарий завершён
        scenarioFinished();
    }
}

bool ScenarioManager::executeStep(const ScenarioStep& step)
{
    qDebug() << "ScenarioManager: Выполнение шага" << step.lineNumber
             << ":" << step.command << "с параметрами:" << step.params;

    // Обрабатываем специальные команды
    if (step.command == "COMMENT") {
        return handleComment(step);
    } else if (step.command == "DELAY") {
        return handleDelay(step);
    } else if (step.command == "SET_ADDRESS") {
        return handleSetAddress(step);
    }

    // Обрабатываем команды ППБ
    try {
        if (step.command == "POLL_STATUS") {
            return handlePollStatus(step);
        } else if (step.command == "SET_PARAMS") {
            return handleSetParams(step);
        } else if (step.command == "RESET") {
            return handleReset(step);
        } else if (step.command == "FU_TRANSMIT" || step.command == "FU_RECEIVE") {
            return handleFUMode(step);
        } else {
            emit stepError(step.lineNumber,
                           QString("Неизвестная команда: %1").arg(step.command));
            return false;
        }
    } catch (const std::exception& e) {
        emit stepError(step.lineNumber,
                       QString("Ошибка выполнения: %1").arg(e.what()));
        return false;
    }
}

bool ScenarioManager::handlePollStatus(const ScenarioStep& step)
{
    // Формат: POLL_STATUS [адрес]
    uint8_t address = m_defaultAddress;
    if (step.params.size() >= 1) {
        address = static_cast<uint8_t>(step.params[0].toUInt());
    }

    QByteArray packet = PacketBuilder::createSimpleStatusRequest(address);
    sendCommandToAddress(packet, address);

    return true;
}

bool ScenarioManager::handleSetParams(const ScenarioStep& step)
{
    // Формат: SET_PARAMS [адрес] [длительность] [скважность] [задержка]
    if (step.params.size() < 3) {
        emit stepError(step.lineNumber,
                       "Недостаточно параметров для SET_PARAMS. Нужно: адрес, длительность, скважность, [задержка]");
        return false;
    }

    uint8_t address = m_defaultAddress;
    if (step.params.size() >= 4) {
        address = static_cast<uint8_t>(step.params[0].toUInt());
    }

    uint32_t duration = step.params.size() >= 4 ?
                            step.params[1].toUInt() : step.params[0].toUInt();
    uint8_t duty = static_cast<uint8_t>(step.params.size() >= 4 ?
                                            step.params[2].toUInt() : step.params[1].toUInt());
    uint32_t delay = step.params.size() >= 4 ?
                         step.params[3].toUInt() :
                         (step.params.size() >= 3 ? step.params[2].toUInt() : 0);

    QByteArray packet = PacketBuilder::createSimpleSetParams(address, duration, duty, delay);
    sendCommandToAddress(packet, address);

    return true;
}

bool ScenarioManager::handleReset(const ScenarioStep& step)
{
    // Формат: RESET [адрес]
    uint8_t address = m_defaultAddress;
    if (step.params.size() >= 1) {
        address = static_cast<uint8_t>(step.params[0].toUInt());
    }

    QByteArray packet = PacketBuilder::createSimpleReset(address);
    sendCommandToAddress(packet, address);

    return true;
}

bool ScenarioManager::handleFUMode(const ScenarioStep& step)
{
    // Формат: FU_TRANSMIT [адрес] или FU_RECEIVE [адрес]
    uint8_t address = m_defaultAddress;
    if (step.params.size() >= 1) {
        address = static_cast<uint8_t>(step.params[0].toUInt());
    }

    bool transmit = (step.command == "FU_TRANSMIT");
    QByteArray packet = PacketBuilder::createFUModePacket(address, transmit);
    sendCommandToAddress(packet, address);

    return true;
}

bool ScenarioManager::handleDelay(const ScenarioStep& step)
{
    // Формат: DELAY [время_мс]
    if (step.params.isEmpty()) {
        emit stepError(step.lineNumber, "Не указано время задержки");
        return false;
    }

    int delayMs = step.params[0].toInt();
    if (delayMs <= 0) {
        delayMs = 1000; // Задержка по умолчанию 1 секунда
    }

    // Для задержки просто ждём указанное время
    // Таймер уже установлен на m_stepDelayMs, но для DELAY нужно специальная обработка
    // В данном случае просто пропускаем шаг без отправки команд

    qDebug() << "ScenarioManager: Задержка" << delayMs << "мс";

    return true;
}

bool ScenarioManager::handleSetAddress(const ScenarioStep& step)
{
    // Формат: SET_ADDRESS [адрес]
    if (step.params.isEmpty()) {
        emit stepError(step.lineNumber, "Не указан адрес");
        return false;
    }

    m_defaultAddress = static_cast<uint8_t>(step.params[0].toUInt());
    qDebug() << "ScenarioManager: Установлен адрес по умолчанию:" << m_defaultAddress;

    return true;
}

bool ScenarioManager::handleComment(const ScenarioStep& step)
{
    // Комментарии просто пропускаются, но логируются
    qDebug() << "ScenarioManager: Комментарий:" << step.description;
    return true;
}

void ScenarioManager::sendCommandToAddress(const QByteArray& data, uint8_t address)
{
    if (!m_client || !m_client->isBound()) {
        emit stepError(m_currentStepIndex + 1, "Сетевой клиент не готов");
        return;
    }

    // Для широковещательного адреса используем широковещательную отправку
    if (address == PacketBuilder::BROADCAST_ADDRESS) {
        m_client->sendBroadcast(data, m_defaultPort);
        emit broadcastSent(data, m_defaultPort);
    } else {
        // В реальном приложении нужно преобразовать адрес ППБ в IP
        // Пока используем широковещательную рассылку для всех команд
        m_client->sendBroadcast(data, m_defaultPort);
        emit broadcastSent(data, m_defaultPort);

        // Логируем для отладки
        qDebug() << "ScenarioManager: Отправлена команда на адрес" << address
                 << ", размер:" << data.size() << "байт";
    }
}

void ScenarioManager::onStepTimeout()
{
    if (m_isRunning && !m_isPaused) {
        executeNextStep();
    }
}

void ScenarioManager::onNetworkError(const QString& error)
{
    emit scenarioError(QString("Сетевая ошибка: %1").arg(error));

    // При ошибке сети останавливаем сценарий
    if (m_isRunning) {
        stopScenario();
    }
}

bool ScenarioManager::isRunning() const
{
    return m_isRunning;
}

bool ScenarioManager::isPaused() const
{
    return m_isPaused;
}

int ScenarioManager::currentStep() const
{
    return m_currentStepIndex;
}

int ScenarioManager::totalSteps() const
{
    return m_steps.size();
}

QString ScenarioManager::currentScenarioFile() const
{
    return m_currentFile;
}

void ScenarioManager::setDefaultAddress(uint8_t address)
{
    m_defaultAddress = address;
}

void ScenarioManager::setDefaultPort(quint16 port)
{
    m_defaultPort = port;
}

void ScenarioManager::setStepDelay(int ms)
{
    m_stepDelayMs = ms;
}

int ScenarioManager::executionTime() const
{
    return m_executionTimer.elapsed();
}

void ScenarioManager::finishScenarioExecution()  // СТАЛО
{
    m_isRunning = false;
    m_isPaused = false;
    m_stepTimer->stop();

    int executionTime = m_executionTimer.elapsed();

    emit scenarioFinished();  // Эмитим сигнал с оригинальным именем
    emit runningChanged(false);

    qDebug() << "ScenarioManager: Сценарий завершён. Время выполнения:"
             << executionTime << "мс, Успешных шагов:" << m_successfulSteps
             << ", Ошибок:" << m_failedSteps;
}
*/
