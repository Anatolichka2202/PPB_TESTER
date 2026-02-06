// logger.cpp
#include "logger.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include <atomic>

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
    if (m_shutdown.load(std::memory_order_acquire)) {
        // Режим завершения - пропускаем UI callback
        return;
    }

    if (!m_uiCallback) {
        std::cerr << "Logger::writeToUI: no callback set" << std::endl;
        return;
    }

    std::cerr << "Logger::writeToUI: thread = " << QThread::currentThread()
              << ", app thread = " << QCoreApplication::instance()->thread() << std::endl;

    // Проверяем, в каком потоке мы находимся
    // Если не в главном потоке Qt, используем invokeMethod
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        std::cerr << "Logger::writeToUI: NOT in main thread, using invokeMethod" << std::endl;
        // Копируем entry для передачи через очередь
        LogEntry entryCopy = entry;
        try{
        QMetaObject::invokeMethod(QCoreApplication::instance(),
                                  [entryCopy]() {
                                    std::cerr << "Logger::writeToUI: in invokeMethod lambda" << std::endl;
                                      if (m_uiCallback) {
                                          std::cerr << "Logger::writeToUI: calling callback" << std::endl;
                                          m_uiCallback(entryCopy);
                                          std::cerr << "Logger::writeToUI: callback completed" << std::endl;
                                      }
                                  }, Qt::QueuedConnection);
        } catch(const std::exception& e)
        {
            std::cerr << "Logger::writeToUI: exception in invokeMethod: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Logger::writeToUI: unknown exception in invokeMethod" << std::endl;
        }

        return;
    }

    // В главном потоке - вызываем напрямую
    std::cerr << "Logger::writeToUI: in main thread, calling directly" << std::endl;
    try {
        m_uiCallback(entry);
        std::cerr << "Logger::writeToUI: direct callback completed" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Logger::writeToUI: exception in callback: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Logger::writeToUI: unknown exception in callback" << std::endl;
    }
}
