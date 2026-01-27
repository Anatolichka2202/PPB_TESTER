#include "logger.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>

// Инициализация статических членов
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
    if (m_initialized) return;

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

        info("Логирование инициализировано");
    } else {
        qDebug() << "Ошибка открытия файла лога:" << filename;
    }
}

void Logger::setUICallback(UICallback callback)
{
    m_uiCallback = callback;
}

void Logger::debug(const QString& message)
{
    write("DEBUG", message);
}

void Logger::info(const QString& message)
{
    write("INFO", message);
}

void Logger::warning(const QString& message)
{
    write("WARNING", message);
}

void Logger::error(const QString& message)
{
    write("ERROR", message);
}

void Logger::write(const QString& level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formatted = QString("[%1] [%2] %3").arg(timestamp, level, message);

    writeToConsole(formatted);
    writeToFile(formatted);
    writeToUI(formatted);
    m_lastLog = formatted;
}

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
    qDebug() << message;
}

void Logger::writeToUI(const QString& message)
{
    if (m_uiCallback) {
        // Добавляем HTML форматирование для цветов
        QString formattedMessage = message;

        if (message.contains("[DEBUG]")) {
            formattedMessage = QString("<span style='color: #008000;'>%1</span>").arg(message);
        } else if (message.contains("[INFO]")) {
            formattedMessage = QString("<span style='color: #000000;'>%1</span>").arg(message);
        } else if (message.contains("[WARNING]")) {
            formattedMessage = QString("<span style='color: #FFA500;'>%1</span>").arg(message);
        } else if (message.contains("[ERROR]")) {
            formattedMessage = QString("<span style='color: #FF0000;'>%1</span>").arg(message);
        }

        m_uiCallback(formattedMessage);
    }
}
