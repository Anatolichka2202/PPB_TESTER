#ifndef LOGCONFIG_H
#define LOGCONFIG_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QMap>
#include <QFile>
#include <QMutex>

// Уровни логирования
enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
    LOG_CRITICAL = 4
};

// Группы категорий для разных аудиторий
enum LogAudience {
    AUDIENCE_ENGINEER,      // Инженер/оператор
    AUDIENCE_DEVELOPER,     // Программист
    AUDIENCE_OPERATIONAL,   // Отчётность
    AUDIENCE_ALL           // Все
};

// Категории логов (сгруппированные по аудиториям)
namespace LogCategories {
// Инженерные категории (для UI)
const QString UI_STATUS = "UI_STATUS";
const QString UI_OPERATION = "UI_OPERATION";
const QString UI_RESULT = "UI_RESULT";
const QString UI_ALERT = "UI_ALERT";
const QString UI_DATA = "UI_DATA";
const QString UI_CONNECTION = "UI_CONNECTION";

// Технические категории (для программистов)
const QString TECH_DEBUG = "TECH_DEBUG";
const QString TECH_NETWORK = "TECH_NETWORK";
const QString TECH_PROTOCOL = "TECH_PROTOCOL";
const QString TECH_THREAD = "TECH_THREAD";
const QString TECH_PERFORMANCE = "TECH_PERFORMANCE";
const QString TECH_MEMORY = "TECH_MEMORY";
const QString TECH_STATE = "TECH_STATE";

// Операционные категории (для отчётности)
const QString OP_SESSION = "OP_SESSION";
const QString OP_OPERATION = "OP_OPERATION";
const QString OP_MEASUREMENT = "OP_MEASUREMENT";
const QString OP_EVENT = "OP_EVENT";
const QString OP_SUMMARY = "OP_SUMMARY";

// Общие категории
const QString GENERAL = "GENERAL";
const QString SYSTEM = "SYSTEM";

// Группы категорий по аудиториям
const QSet<QString> EngineerCategories = {
    UI_STATUS, UI_OPERATION, UI_RESULT, UI_ALERT,
    UI_DATA, UI_CONNECTION, SYSTEM, GENERAL
};

const QSet<QString> DeveloperCategories = {
    TECH_DEBUG, TECH_NETWORK, TECH_PROTOCOL, TECH_THREAD,
    TECH_PERFORMANCE, TECH_MEMORY, TECH_STATE,
    UI_STATUS, UI_OPERATION, UI_RESULT, UI_ALERT,
    UI_DATA, UI_CONNECTION, SYSTEM, GENERAL
};

const QSet<QString> OperationalCategories = {
    OP_SESSION, OP_OPERATION, OP_MEASUREMENT, OP_EVENT,
    OP_SUMMARY, UI_OPERATION, UI_RESULT, UI_ALERT,
    SYSTEM, GENERAL
};
}

// Конфигурация канала вывода
struct LogChannelConfig {
    QString name;
    QString description;

    // Настройки вывода
    bool enabled = true;
    LogLevel minLevel = LOG_INFO;
    LogAudience audience = AUDIENCE_ALL;

    // Форматы вывода
    bool showTimestamp = true;
    bool showCategory = true;
    bool showLevel = true;
    bool showThreadInfo = false;

    // Максимальный размер (для файлов)
    qint64 maxFileSize = 10 * 1024 * 1024; // 10 MB
    int maxBackupFiles = 5;

    // Фильтрация по категориям
    QSet<QString> includeCategories;   // Если пусто - все разрешены
    QSet<QString> excludeCategories;   // Приоритет над include

    bool isCategoryAllowed(const QString& category) const {
        // Проверка исключений
        if (excludeCategories.contains(category)) {
            return false;
        }

        // Проверка включений
        if (!includeCategories.isEmpty() && !includeCategories.contains(category)) {
            return false;
        }

        // Проверка по аудитории
        switch (audience) {
        case AUDIENCE_ENGINEER:
            return LogCategories::EngineerCategories.contains(category);
        case AUDIENCE_DEVELOPER:
            return LogCategories::DeveloperCategories.contains(category);
        case AUDIENCE_OPERATIONAL:
            return LogCategories::OperationalCategories.contains(category);
        case AUDIENCE_ALL:
        default:
            return true;
        }
    }
};

// Главный класс конфигурации логирования
class LogConfig : public QObject
{
    Q_OBJECT

public:
    static LogConfig& instance();

    // Инициализация стандартной конфигурации
    void initDefaultConfig();

    // Управление каналами
    void addChannel(const QString& id, const LogChannelConfig& config);
    void removeChannel(const QString& id);
    void enableChannel(const QString& id, bool enabled);

    // Получение конфигурации
    LogChannelConfig getChannel(const QString& id) const;
    QList<QString> getChannelIds() const;

    // Проверка, должен ли лог попасть в канал
    bool shouldLogToChannel(const QString& channelId,
                            LogLevel level,
                            const QString& category) const;

    // Сохранение/загрузка конфигурации
    bool saveToFile(const QString& filename);
    bool loadFromFile(const QString& filename);

    // Динамическое изменение настроек
    void setMinLevel(const QString& channelId, LogLevel level);
    void setAudience(const QString& channelId, LogAudience audience);

    void setUIMinLevel(LogLevel level);
    void setUICategories(const QSet<QString>& categories);
    LogChannelConfig getUIConfig() const;

signals:
    void configChanged(const QString& channelId);
    void channelAdded(const QString& channelId);
    void channelRemoved(const QString& channelId);

private:
    explicit LogConfig(QObject* parent = nullptr);
    ~LogConfig();

    QMap<QString, LogChannelConfig> m_channels;
    mutable QMutex m_mutex;

    // Стандартные каналы
    void createDefaultChannels();


};

#endif // LOGCONFIG_H
