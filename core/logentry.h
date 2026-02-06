// logentry.h
#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QDateTime>
#include <QString>
#include <QVariant>

struct LogEntry {
    QDateTime timestamp;
    QString level;      // "DEBUG", "INFO", "WARNING", "ERROR"
    QString category;   // "CONTROLLER", "ENGINE", "COMMAND", "UI", "GENERAL"
    QString message;
    QVariant data;      // Дополнительные данные (необязательно)

    // Конструкторы для удобства
    LogEntry() = default;
    LogEntry(const QString& lvl, const QString& cat, const QString& msg)
        : timestamp(QDateTime::currentDateTime())
        , level(lvl)
        , category(cat)
        , message(msg) {}

    // Методы для преобразования
    QString toString() const {
        return QString("[%1] [%2] [%3] %4")
        .arg(timestamp.toString("hh:mm:ss.zzz"))
            .arg(level, -7)  // Выравнивание по ширине
            .arg(category, -12)
            .arg(message);
    }

    QString toHtml() const {
        // Базовый HTML без стилей - стили будут в CSS
        return QString("<div class='log-entry log-%1 log-category-%2'>"
                       "<span class='log-time'>%3</span> "
                       "<span class='log-level'>[%4]</span> "
                       "<span class='log-category'>[%5]</span> "
                       "<span class='log-message'>%6</span>"
                       "</div>")
            .arg(level.toLower())
            .arg(category.toLower())
            .arg(timestamp.toString("hh:mm:ss.zzz"))
            .arg(level)
            .arg(category)
            .arg(message.toHtmlEscaped());
    }

    // Для сортировки по времени
    bool operator<(const LogEntry& other) const {
        return timestamp < other.timestamp;
    }

    bool operator==(const LogEntry& other) const {
        return timestamp == other.timestamp &&
               level == other.level &&
               category == other.category &&
               message == other.message;
    }
};

Q_DECLARE_METATYPE(LogEntry)

#endif // LOGENTRY_H
