// logwrapper.h (только сигналы и методы, без макросов)
#ifndef LOGWRAPPER_H
#define LOGWRAPPER_H

#include <QObject>
#include "logentry.h"

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

    // Методы для структурированных данных
    static void structuredLog(const QString& level, const QString& category, const QString& message);
    static void techLog(const QString& level, const QString& category, const QString& message);
    static void logTable(const TableData& table);
    static void logCard(const CardData& card);
    static void logProgress(const ProgressData& progress);

signals:
    // Сигнал с полной информацией о логе
    void logEntryReceived(const LogEntry& entry);

private slots:
    void emitLogEntry(const LogEntry& entry);

private:
    explicit LogWrapper(QObject *parent = nullptr);
    ~LogWrapper() override;

    // Статический экземпляр
    static LogWrapper* m_instance;
};

#endif // LOGWRAPPER_H
