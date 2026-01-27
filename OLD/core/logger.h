// В logger.h добавляем:
#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <functional>

class Logger
{
public:
    // Тип для функции обратного вызова в UI
    using UICallback = std::function<void(const QString&)>;

    // Простая инициализация
    static void init();
    static void init(UICallback uiCallback); // С callback для UI

    // Простые методы логирования
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

    static void setUiOutputEnabled(bool enabled);
    static void setFileOutputEnabled(bool enabled);
    static void setConsoleOutputEnabled(bool enabled);

    // Для вывода в UI
    static QString lastLog() { return m_lastLog; }

    // Установка callback для UI
    static void setUICallback(UICallback callback);

private:
    Logger() = delete; // Статический класс

    static void write(const QString& level, const QString& message);
    static void writeToFile(const QString& message);
    static void writeToConsole(const QString& message);
    static void writeToUI(const QString& message);

    static QFile m_logFile;
    static QString m_lastLog;
    static bool m_initialized;
    static UICallback m_uiCallback;
};

// Макросы для удобства
#define LOG_DEBUG(msg) Logger::debug(msg)
#define LOG_INFO(msg) Logger::info(msg)
#define LOG_WARNING(msg) Logger::warning(msg)
#define LOG_ERROR(msg) Logger::error(msg)

#endif // LOGGER_H
