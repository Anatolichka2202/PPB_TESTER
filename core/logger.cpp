// logger.cpp
#include "logger.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include <atomic>
#include "logging/logconfig.h"
QMutex Logger::m_logMutex;
QFile Logger::m_logFile;
QString Logger::m_lastLog = "";
bool Logger::m_initialized = false;
Logger::UICallback Logger::m_uiCallback = nullptr;

void Logger::init()
{
    init(nullptr);
}

void Logger::init(UICallback uiCallback)
{
    std::cerr << "Logger::init called from: "
              << QThread::currentThread() << std::endl;

    if (m_initialized) {
        qWarning() << "Logger уже инициализирован!";
        return;
    }

    // Сохраняем callback
    m_uiCallback = uiCallback;

    // Создаем папку logs если её нет
    QDir dir("logs");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Открываем файл лога
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = QString("logs/ppb_log_%1.txt").arg(timestamp);

    m_logFile.setFileName(filename);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream stream(&m_logFile);
        stream << "=== Начало лога: " << timestamp << " ===\n";
        stream.flush();
        m_initialized = true;

        // Используем новый метод write с LogEntry
       // info("Logger", "Логирование инициализировано");
        std::cerr << "Logger инициализирован успешно" << std::endl;
    } else {
        qCritical() << "Ошибка открытия файла лога:" << filename;
    }
}

void Logger::setUICallback(UICallback callback)
{
    m_uiCallback = callback;
}


static LogLevel levelFromString(const QString& levelStr)
{
    if (levelStr == "DEBUG") return LOG_DEBUG;
    if (levelStr == "INFO") return LOG_INFO;
    if (levelStr == "WARNING") return LOG_WARNING;
    if (levelStr == "ERROR") return LOG_ERROR;
    return LOG_INFO; // значение по умолчанию
}

// Основной метод записи
void Logger::write(const LogEntry& entry)
{
    std::cerr << "Logger::write trying to lock mutex..." << std::endl;

    QMutexLocker locker(&m_logMutex);

    std::cerr << "Logger::write mutex locked" << std::endl;

    QString formatted = entry.toString();
    m_lastLog = formatted;

    // Запись в файл
    writeToFile(formatted);

    // Запись в консоль
    writeToConsole(formatted);

    // Запись в UI (если callback установлен)
    writeToUI(entry);

    std::cerr << "Logger::write completed" << std::endl;
}

// Упрощенные методы (для обратной совместимости)
void Logger::debug(const QString& message)
{
    write(LogEntry("DEBUG", "GENERAL", message));
}

void Logger::info(const QString& message)
{
    write(LogEntry("INFO", "GENERAL", message));
}

void Logger::warning(const QString& message)
{
    write(LogEntry("WARNING", "GENERAL", message));
}

void Logger::error(const QString& message)
{
    write(LogEntry("ERROR", "GENERAL", message));
}

// Методы с категориями
void Logger::debug(const QString& category, const QString& message)
{
    write(LogEntry("DEBUG", category, message));
}

void Logger::info(const QString& category, const QString& message)
{
    write(LogEntry("INFO", category, message));
}

void Logger::warning(const QString& category, const QString& message)
{
    write(LogEntry("WARNING", category, message));
}

void Logger::error(const QString& category, const QString& message)
{
    write(LogEntry("ERROR", category, message));
}

// Внутренние методы записи
void Logger::writeToFile(const QString& message)
{
    if (m_initialized && m_logFile.isOpen()) {
        QTextStream stream(&m_logFile);
        stream << message << "\n";
        stream.flush();
    }
}

void Logger::writeToConsole(const QString& message)
{
    qDebug().noquote() << message;
}

std::atomic<bool> Logger::m_shutdown(false);

void Logger::setShutdownMode(bool shutdown)
{
    m_shutdown.store(shutdown, std::memory_order_release);
}

void Logger::writeToUI(const LogEntry& entry)
{
    // Быстрая проверка без блокировок
    if (m_shutdown.load(std::memory_order_acquire) || !m_uiCallback) {
        return;
    }

    //ПРОСТОЙ ВЫЗОВ - без проверок потоков
    // Пусть LogWrapper сам разбирается с потоками
    try {
        m_uiCallback(entry);
    } catch (...) {
        // Игнорируем исключения
    }
}
