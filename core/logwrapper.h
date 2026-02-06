// logwrapper.h
#ifndef LOGWRAPPER_H
#define LOGWRAPPER_H

#include <QObject>
#include "logentry.h"
#include <QMutex>
class LogWrapper : public QObject
{
    Q_OBJECT

public:
    // Удаляем копирование и присваивание
    LogWrapper(const LogWrapper&) = delete;
    LogWrapper& operator=(const LogWrapper&) = delete;

    // Получение экземпляра синглтона
    static LogWrapper* instance();

    // Статические методы для логирования
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

    static void debug(const QString& category, const QString& message);
    static void info(const QString& category, const QString& message);
    static void warning(const QString& category, const QString& message);
    static void error(const QString& category, const QString& message);

    // Прямой метод для отправки LogEntry
    static void log(const LogEntry& entry);

signals:
    // Сигнал с полной информацией о логе
    void logEntryReceived(const LogEntry& entry);

    // Сигналы для удобства (опционально)
    void debugMessage(const QString& category, const QString& message);
    void infoMessage(const QString& category, const QString& message);
    void warningMessage(const QString& category, const QString& message);
    void errorMessage(const QString& category, const QString& message);

private slots:  // <-- ДОБАВЛЯЕМ ЭТО
    // Внутренний метод для отправки лога (должен быть слотом для invokeMethod)
    void sendLog(const LogEntry& entry);

private:
    explicit LogWrapper(QObject *parent = nullptr);
    ~LogWrapper() override;

    // Внутренний метод для отправки лога
    //void sendLog(const LogEntry& entry);

    // Статический экземпляр
    static LogWrapper* m_instance;

    // Мьютекс для потокобезопасного создания
    static QMutex m_instanceMutex;
};

// Макросы для удобного использования
#define LOG_DEBUG(msg) LogWrapper::debug(msg)
#define LOG_INFO(msg) LogWrapper::info(msg)
#define LOG_WARNING(msg) LogWrapper::warning(msg)
#define LOG_ERROR(msg) LogWrapper::error(msg)

#define LOG_CAT_DEBUG(cat, msg) LogWrapper::debug(cat, msg)
#define LOG_CAT_INFO(cat, msg) LogWrapper::info(cat, msg)
#define LOG_CAT_WARNING(cat, msg) LogWrapper::warning(cat, msg)
#define LOG_CAT_ERROR(cat, msg) LogWrapper::error(cat, msg)

// Макросы с автоматическим определением категории
#define LOG_DEBUG_AUTO(msg) LogWrapper::debug(QString(__FILE__), msg)
#define LOG_INFO_AUTO(msg) LogWrapper::info(QString(__FILE__), msg)

// Категории для логов
#define LOG_UDP(msg) LOG_CAT_INFO("UDP", msg)
#define LOG_ENGINE(msg) LOG_CAT_INFO("Engine", msg)
#define LOG_COMM(msg) LOG_CAT_INFO("Comm", msg)
#define LOG_CONTROLLER(msg) LOG_CAT_INFO("Controller", msg)
#define LOG_UI(msg) LOG_CAT_INFO("UI", msg)
#define LOG_COMMAND(msg) LOG_CAT_INFO("Command", msg)

// Быстрые макросы с категориями и уровнями
#define LOG_UDP_INFO(msg) LOG_CAT_INFO("UDP", msg)
#define LOG_UDP_DEBUG(msg) LOG_CAT_DEBUG("UDP", msg)
#define LOG_UDP_ERROR(msg) LOG_CAT_ERROR("UDP", msg)
#define LOG_UDP_WARNING(msg) LOG_CAT_WARNING("UDP", msg)

#define LOG_ENGINE_INFO(msg) LOG_CAT_INFO("Engine", msg)
#define LOG_ENGINE_DEBUG(msg) LOG_CAT_DEBUG("Engine", msg)
#define LOG_ENGINE_ERROR(msg) LOG_CAT_ERROR("Engine", msg)
#define LOG_ENGINE_WARNING(msg) LOG_CAT_WARNING("Engine", msg)

#define LOG_CONTROLLER_INFO(msg) LOG_CAT_INFO("Controller", msg)
#define LOG_CONTROLLER_DEBUG(msg) LOG_CAT_DEBUG("Controller", msg)
#define LOG_CONTROLLER_ERROR(msg) LOG_CAT_ERROR("Controller", msg)
#define LOG_CONTROLLER_WARNING(msg) LOG_CAT_WARNING("Controller", msg)

#define LOG_PPBCOM_INFO(msg) LOG_CAT_INFO("PPBcom", msg)
#define LOG_PPBCOM_DEBUG(msg) LOG_CAT_DEBUG("PPBcom", msg)
#define LOG_PPBCOM_ERROR(msg) LOG_CAT_ERROR("PPBcom", msg)
#define LOG_PPBCOM_WARNING(msg) LOG_CAT_WARNING("PPBcom", msg)

#endif // LOGWRAPPER_H
