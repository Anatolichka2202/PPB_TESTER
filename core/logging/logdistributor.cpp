// logdistributor.cpp
#include "logdistributor.h"
#include "logconfig.h"
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>

LogDistributor& LogDistributor::instance()
{
    static LogDistributor instance;
    return instance;
}

LogDistributor::LogDistributor(QObject* parent) : QObject(parent)
{
    // Создаем папку для логов
    QDir dir("logs");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void LogDistributor::init()
{
    // Открываем файлы для каждого канала
    openLogFiles();

    // Логируем инициализацию
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString msg = QString("LogDistributor инициализирован: %1").arg(timestamp);
    qDebug() << msg;
}

void LogDistributor::openLogFiles()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

    // Технический лог
    m_techFile.setFileName(QString("logs/tech_%1.log").arg(timestamp));
    if (m_techFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream stream(&m_techFile);
        stream << "=== Технический лог (для программистов) ===\n";
        stream << "=== Начало: " << timestamp << " ===\n\n";
        stream.flush();
    }

    // Операционный лог
    m_operFile.setFileName(QString("logs/oper_%1.log").arg(timestamp));
    if (m_operFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream stream(&m_operFile);
        stream << "=== Операционный лог (для отчётности) ===\n";
        stream << "=== Начало: " << timestamp << " ===\n\n";
        stream.flush();
    }

    // Общий лог (все сообщения)
    m_allFile.setFileName(QString("logs/all_%1.log").arg(timestamp));
    if (m_allFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream stream(&m_allFile);
        stream << "=== Общий лог (все сообщения) ===\n";
        stream << "=== Начало: " << timestamp << " ===\n\n";
        stream.flush();
    }
}

void LogDistributor::distribute(const LogEntry& entry)
{
    static bool inDistribute = false;
    if (inDistribute) {
        qDebug() << "[DISTRIBUTOR] Рекурсивный вызов";
        return;
    }
    inDistribute = true;

    QString formatted = entry.toString();  // Новый формат с эмодзи
    QString legacyFormatted = entry.toLegacyFormat();  // Старый формат

    // ВСЕГДА отправляем в UI все записи (не только UI_*)
    // Убираем фильтрацию по категориям
    emit logForUI(entry);

    // Всегда пишем в общий файл (новый формат)
    if (m_allFile.isOpen()) {
        QTextStream stream(&m_allFile);
        stream << formatted << "\n";
        stream.flush();
    }

    // Определяем категорию
    QString category = entry.category;

    // Определяем тип лога по категории
    bool isTech = category.startsWith("TECH_") ||
                  (!category.startsWith("UI_") &&
                   !category.startsWith("OP_") &&
                   category != "GENERAL" &&
                   category != "SYSTEM");
    bool isOp = category.startsWith("OP_") ||
                category == "UI_OPERATION" ||
                category == "UI_RESULT" ||
                category == "UI_ALERT";

    // Технические категории -> tech_file
    if (isTech && m_techFile.isOpen()) {
        QTextStream stream(&m_techFile);
        stream << legacyFormatted << "\n";
        stream.flush();
    }

    // Операционные категории -> oper_file
    if (isOp && m_operFile.isOpen()) {
        QTextStream stream(&m_operFile);
        stream << formatted << "\n";
        stream.flush();
    }

    // Также выводим в консоль для отладки
    qDebug().noquote() << legacyFormatted;

    inDistribute = false;
}

