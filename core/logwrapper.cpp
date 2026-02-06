// logwrapper.cpp
#include "logwrapper.h"
#include <QMutex>
#include <QCoreApplication>
#include <QThread>
#include "logger.h"
#include <iostream>
// Инициализация статических членов
LogWrapper* LogWrapper::m_instance = nullptr;
QMutex LogWrapper::m_instanceMutex;

LogWrapper::LogWrapper(QObject *parent) : QObject(parent)
{
    std::cerr << "LogWrapper constructor start" << std::endl;

    // Убедимся, что объект в основном потоке
    if (QThread::currentThread() != qApp->thread()) {
        std::cerr << "LogWrapper: moving to main thread" << std::endl;
        moveToThread(qApp->thread());
    }
     std::cerr << "LogWrapper: initializing Logger with callback" << std::endl;

    // Инициализируем Logger с callback для записи логов
     try {
         Logger::init([this](const LogEntry& entry) {
             std::cerr << "Logger callback invoked, emitting signal" << std::endl;
             emit logEntryReceived(entry);
             std::cerr << "Logger callback completed" << std::endl;
         });
     } catch (const std::exception& e) {
         std::cerr << "LogWrapper: exception in Logger::init: " << e.what() << std::endl;
         throw;
     }

    // Используем макрос с категорией (два аргумента)
    std::cerr << "LogWrapper инициализирован" << std::endl;
}

LogWrapper::~LogWrapper()
{
    std::cerr << "LogWrapper уничтожен" << std::endl;
    m_instance = nullptr;
}

LogWrapper* LogWrapper::instance()
{
    // Двойная проверка блокировки для потокобезопасности
    if (!m_instance) {
        QMutexLocker locker(&m_instanceMutex);
        if (!m_instance) {
             std::cerr << "Создание экземпляра LogWrapper..." << std::endl;
            m_instance = new LogWrapper();
            // Перемещаем в основной поток (на всякий случай)
            m_instance->moveToThread(qApp->thread());
           std::cerr << "Экземпляр LogWrapper создан" << std::endl;
        }
    }
    return m_instance;
}

// Прямой метод для отправки LogEntry
void LogWrapper::log(const LogEntry& entry)
{
    // Получаем экземпляр
    LogWrapper* wrapper = instance();

    // Проверяем, в каком потоке мы находимся
    if (QThread::currentThread() == wrapper->thread()) {
        // В основном потоке - отправляем сразу
        wrapper->sendLog(entry);
    } else {
        // В другом потоке - отправляем через очередь
        QMetaObject::invokeMethod(wrapper, "sendLog",
                                  Qt::QueuedConnection,
                                  Q_ARG(LogEntry, entry));
    }
}

// Внутренний метод для отправки лога
void LogWrapper::sendLog(const LogEntry& entry)
{
    // Отправляем в Logger (который запишет в файл и консоль)
    Logger::write(entry);
}

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
