// logwrapper.cpp
#include "logwrapper.h"
#include "logging/logdistributor.h"
#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include "logging/logconfig.h"
LogWrapper* LogWrapper::m_instance = nullptr;

// Убираем мьютекс из instance(), используем атомарную инициализацию
LogWrapper* LogWrapper::instance()
{
    // Простая проверка без мьютекса для первого вызова
    if (!m_instance) {
        // Создаем в главном потоке
        m_instance = new LogWrapper();
        m_instance->moveToThread(qApp->thread());
    }
    return m_instance;
}

LogWrapper::LogWrapper(QObject *parent) : QObject(parent)
{
    std::cerr << "LogWrapper constructor start" << std::endl;

    // Убедимся, что объект в основном потоке
    if (QThread::currentThread() != qApp->thread()) {
        std::cerr << "LogWrapper: moving to main thread" << std::endl;
        moveToThread(qApp->thread());
    }

    //  ИНИЦИАЛИЗИРУЕМ LOGCONFIG
    LogConfig::instance().initDefaultConfig();

    // Инициализируем распределитель
    LogDistributor::instance().init();

    // Подключаем сигнал от распределителя к нашему сигналу
    connect(&LogDistributor::instance(), &LogDistributor::logForUI,
            this, &LogWrapper::logEntryReceived, Qt::QueuedConnection);

    std::cerr << "LogWrapper инициализирован" << std::endl;
}

LogWrapper::~LogWrapper()
{
    std::cerr << "LogWrapper уничтожен" << std::endl;
    m_instance = nullptr;
}

// Прямой метод для отправки LogEntry
void LogWrapper::log(const LogEntry& entry)
{
    static bool inLog = false;
    if (inLog) {
        std::cerr << "LogWrapper::log: рекурсивный вызов, пропускаем" << std::endl;
        return;
    }

    inLog = true;

    // Логируем сам факт логирования (для отладки маппинга)
    if (entry.category == "LEGACY" || entry.category.contains("[") || entry.category.contains("]")) {
        // Это старый формат - логируем маппинг
        qDebug() << "[LOGGER] Legacy log detected: " << entry.category << " -> "
                 << entry.message.left(50) << "...";
    }

    LogDistributor::instance().distribute(entry);
    inLog = false;
}

// Удаляем все остальные методы sendLog и т.д.
// Оставляем только простые вызовы...

// Статические методы без категории
void LogWrapper::debug(const QString& message)
{
    log(LogEntry("DEBUG", "GENERAL", message));
}

void LogWrapper::info(const QString& message)
{
    log(LogEntry("INFO", "GENERAL", message));
}

void LogWrapper::warning(const QString& message)
{
    log(LogEntry("WARNING", "GENERAL", message));
}

void LogWrapper::error(const QString& message)
{
    log(LogEntry("ERROR", "GENERAL", message));
}

// Статические методы с категорией
void LogWrapper::debug(const QString& category, const QString& message)
{
    log(LogEntry("DEBUG", category, message));
}

void LogWrapper::info(const QString& category, const QString& message)
{
    log(LogEntry("INFO", category, message));
}

void LogWrapper::warning(const QString& category, const QString& message)
{
    log(LogEntry("WARNING", category, message));
}

void LogWrapper::error(const QString& category, const QString& message)
{
    log(LogEntry("ERROR", category, message));
}

void LogWrapper::structuredLog(const QString& level, const QString& category, const QString& message)
{
    log(LogEntry(level, category, message));
}

void LogWrapper::techLog(const QString& level, const QString& category, const QString& message)
{
    log(LogEntry(level, category, message));
}

void LogWrapper::logTable(const TableData& table)
{
    LogEntry entry = LogEntry::createTable("UI_DATA", table);
    log(entry);
}

void LogWrapper::logCard(const CardData& card)
{
    LogEntry entry = LogEntry::createCard("UI_STATUS", card);
    log(entry);
}

void LogWrapper::logProgress(const ProgressData& progress)
{
    LogEntry entry = LogEntry::createProgress("UI_STATUS", progress);
    log(entry);
}

void LogWrapper::emitLogEntry(const LogEntry& entry)
{
    emit logEntryReceived(entry);
}
