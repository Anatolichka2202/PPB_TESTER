// logger.h - добавляем в начало
#ifndef LOGGER_H
#define LOGGER_H
#include <atomic>
#include <QMutex>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <functional>
#include "logentry.h"  // Добавляем

class Logger
{
public:
    // Тип для функции обратного вызова в UI
    using UICallback = std::function<void(const LogEntry&)>;

    // Инициализация
    static void init();
    static void init(UICallback uiCallback);

    // Основной метод записи
    static void write(const LogEntry& entry);

    // Упрощенные методы (для обратной совместимости)
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

    // Методы с категориями
    static void debug(const QString& category, const QString& message);
    static void info(const QString& category, const QString& message);
    static void warning(const QString& category, const QString& message);
    static void error(const QString& category, const QString& message);

    // Установка callback
    static void setUICallback(UICallback callback);

    // Геттеры
    static QString lastLog() { return m_lastLog; }
    static bool isInitialized() { return m_initialized; }
    static void setShutdownMode(bool shutdown) ;
private:
    Logger() = delete;

    // Внутренние методы записи
    static void writeToFile(const QString& message);
    static void writeToConsole(const QString& message);
    static void writeToUI(const LogEntry& entry);

    // Статические члены
    static QMutex m_logMutex;
    static QFile m_logFile;
    static QString m_lastLog;
    static bool m_initialized;
    static UICallback m_uiCallback;
    static std::atomic<bool> m_shutdown;
};

#endif // LOGGER_H
