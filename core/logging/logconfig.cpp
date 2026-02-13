#include "logconfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

#include "../logging/logging_unified.h"
LogConfig& LogConfig::instance()
{
    static LogConfig instance;
    return instance;
}

LogConfig::LogConfig(QObject* parent) : QObject(parent)
{
    // Автоматически создаём стандартные каналы
    createDefaultChannels();
}

LogConfig::~LogConfig()
{
}

void LogConfig::setUIMinLevel(LogLevel level)
{
    setMinLevel("ui", level);
}

void LogConfig::setUICategories(const QSet<QString>& categories)
{
    QMutexLocker locker(&m_mutex);

    auto it = m_channels.find("ui");
    if (it != m_channels.end()) {
        it->includeCategories = categories;
        emit configChanged("ui");

        QStringList cats = categories.values();
        LOG_CAT_DEBUG("CONFIG",
                      QString("Установлены категории UI: %1").arg(cats.join(", ")));
    }
}

// Метод для получения текущих настроек UI
LogChannelConfig LogConfig::getUIConfig() const
{
    return getChannel("ui");
}

void LogConfig::createDefaultChannels()
{
    QMutexLocker locker(&m_mutex);

    // 1. UI канал  - для сжатого лога
    LogChannelConfig uiConfig;
    uiConfig.name = "UI Channel";
    uiConfig.description = "Вывод в пользовательский интерфейс для инженеров";
    uiConfig.audience = AUDIENCE_ENGINEER;
    uiConfig.minLevel = LOG_INFO;
    uiConfig.showTimestamp = true;
    uiConfig.showCategory = true;
    uiConfig.showLevel = true;
    uiConfig.showThreadInfo = false;
    uiConfig.enabled = true;
    uiConfig.includeCategories = LogCategories::EngineerCategories;
    m_channels["ui"] = uiConfig;

    // 2. Технический файл (для программистов) - ТОЛЬКО технические категории
    LogChannelConfig techFileConfig;
    techFileConfig.name = "Technical File";
    techFileConfig.description = "Полный технический лог для отладки";
    techFileConfig.audience = AUDIENCE_DEVELOPER;
    techFileConfig.minLevel = LOG_DEBUG;
    techFileConfig.showTimestamp = true;
    techFileConfig.showCategory = true;
    techFileConfig.showLevel = true;
    techFileConfig.showThreadInfo = true;
    techFileConfig.maxFileSize = 50 * 1024 * 1024; // 50 MB
    techFileConfig.maxBackupFiles = 10;
    techFileConfig.enabled = true;
    techFileConfig.includeCategories = LogCategories::DeveloperCategories;
    m_channels["tech_file"] = techFileConfig;

    // 3. Операционный файл (для отчётности) - ТОЛЬКО операционные категории
    LogChannelConfig operFileConfig;
    operFileConfig.name = "Operational File";
    operFileConfig.description = "Операционный лог для отчётности и анализа";
    operFileConfig.audience = AUDIENCE_OPERATIONAL;
    operFileConfig.minLevel = LOG_INFO;
    operFileConfig.showTimestamp = true;
    techFileConfig.showCategory = true;
    techFileConfig.showLevel = true;
    techFileConfig.showThreadInfo = false;
    techFileConfig.maxFileSize = 20 * 1024 * 1024; // 20 MB
    techFileConfig.maxBackupFiles = 5;
    techFileConfig.enabled = true;
    techFileConfig.includeCategories = LogCategories::OperationalCategories;
    m_channels["oper_file"] = operFileConfig;

    // 4. Общий файл (все сообщения) - ВСЕ категории
    LogChannelConfig allFileConfig;
    allFileConfig.name = "All Messages File";
    allFileConfig.description = "Все сообщения всех категорий";
    allFileConfig.audience = AUDIENCE_ALL;
    allFileConfig.minLevel = LOG_DEBUG;
    allFileConfig.showTimestamp = true;
    allFileConfig.showCategory = true;
    allFileConfig.showLevel = true;
    allFileConfig.showThreadInfo = true;
    allFileConfig.maxFileSize = 100 * 1024 * 1024; // 100 MB
    allFileConfig.maxBackupFiles = 20;
    allFileConfig.enabled = true;
    // includeCategories пусто = все разрешены
    m_channels["all_file"] = allFileConfig;

    // 5. Консоль отладки (для программистов) - по умолчанию выключена
    LogChannelConfig consoleConfig;
    consoleConfig.name = "Debug Console";
    consoleConfig.description = "Консоль отладки для разработчиков";
    consoleConfig.audience = AUDIENCE_DEVELOPER;
    consoleConfig.minLevel = LOG_DEBUG;
    consoleConfig.showTimestamp = true;
    consoleConfig.showCategory = true;
    consoleConfig.showLevel = true;
    consoleConfig.showThreadInfo = true;
    consoleConfig.enabled = false; // По умолчанию выключена
    m_channels["debug_console"] = consoleConfig;
}
void LogConfig::initDefaultConfig()
{
    // Уже создано в конструкторе
}

void LogConfig::addChannel(const QString& id, const LogChannelConfig& config)
{
    QMutexLocker locker(&m_mutex);

    if (m_channels.contains(id)) {
        LOG_CAT_WARNING("CONFIG", QString("Канал с ID '%1' уже существует").arg(id));
        return;
    }

    m_channels[id] = config;
    emit channelAdded(id);

    LOG_CAT_DEBUG("CONFIG", QString("Добавлен канал логирования: %1").arg(config.name));
}

void LogConfig::removeChannel(const QString& id)
{
    QMutexLocker locker(&m_mutex);

    if (m_channels.remove(id)) {
        emit channelRemoved(id);
        LOG_CAT_DEBUG("CONFIG", QString("Удалён канал логирования: %1").arg(id));
    }
}

void LogConfig::enableChannel(const QString& id, bool enabled)
{
    QMutexLocker locker(&m_mutex);

    auto it = m_channels.find(id);
    if (it != m_channels.end()) {
        it->enabled = enabled;
        emit configChanged(id);

        LOG_CAT_DEBUG("CONFIG",
                      QString("Канал '%1' %2").arg(id).arg(enabled ? "включён" : "выключен"));
    }
}

LogChannelConfig LogConfig::getChannel(const QString& id) const
{
    QMutexLocker locker(&m_mutex);
    return m_channels.value(id);
}

QList<QString> LogConfig::getChannelIds() const
{
    QMutexLocker locker(&m_mutex);
    return m_channels.keys();
}

bool LogConfig::shouldLogToChannel(const QString& channelId,
                                   LogLevel level,
                                   const QString& category) const
{
    QMutexLocker locker(&m_mutex);

    auto it = m_channels.constFind(channelId);
    if (it == m_channels.constEnd()) {
        return false;
    }

    const LogChannelConfig& config = it.value();

    // Проверка включения канала
    if (!config.enabled) {
        return false;
    }

    // Проверка уровня
    if (level < config.minLevel) {
        return false;
    }

    // Проверка категории
    return config.isCategoryAllowed(category);
}

bool LogConfig::saveToFile(const QString& filename)
{
    QMutexLocker locker(&m_mutex);

    QJsonObject root;
    QJsonObject channelsObj;

    for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
        const QString& id = it.key();
        const LogChannelConfig& config = it.value();

        QJsonObject channelObj;
        channelObj["name"] = config.name;
        channelObj["description"] = config.description;
        channelObj["enabled"] = config.enabled;
        channelObj["minLevel"] = static_cast<int>(config.minLevel);
        channelObj["audience"] = static_cast<int>(config.audience);
        channelObj["showTimestamp"] = config.showTimestamp;
        channelObj["showCategory"] = config.showCategory;
        channelObj["showLevel"] = config.showLevel;
        channelObj["showThreadInfo"] = config.showThreadInfo;
        channelObj["maxFileSize"] = static_cast<qint64>(config.maxFileSize);
        channelObj["maxBackupFiles"] = config.maxBackupFiles;

        // Сохраняем категории
        QJsonArray includeArray;
        for (const QString& cat : config.includeCategories) {
            includeArray.append(cat);
        }
        channelObj["includeCategories"] = includeArray;

        QJsonArray excludeArray;
        for (const QString& cat : config.excludeCategories) {
            excludeArray.append(cat);
        }
        channelObj["excludeCategories"] = excludeArray;

        channelsObj[id] = channelObj;
    }

    root["channels"] = channelsObj;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_CAT_ERROR("CONFIG",
                      QString("Не удалось открыть файл для записи: %1").arg(filename));
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();

    LOG_CAT_INFO("CONFIG",
                 QString("Конфигурация логирования сохранена в: %1").arg(filename));
    return true;
}

bool LogConfig::loadFromFile(const QString& filename)
{
    QMutexLocker locker(&m_mutex);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_CAT_WARNING("CONFIG",
                        QString("Не удалось открыть файл для чтения: %1").arg(filename));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        LOG_CAT_ERROR("CONFIG",
                      QString("Неверный формат JSON в файле: %1").arg(filename));
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject channelsObj = root["channels"].toObject();

    // Очищаем существующие каналы (кроме стандартных)
    QStringList keys = m_channels.keys();
    for (const QString& key : keys) {
        if (!key.startsWith("default_")) {
            m_channels.remove(key);
        }
    }

    // Загружаем каналы из файла
    for (auto it = channelsObj.begin(); it != channelsObj.end(); ++it) {
        QString id = it.key();
        QJsonObject channelObj = it.value().toObject();

        LogChannelConfig config;
        config.name = channelObj["name"].toString();
        config.description = channelObj["description"].toString();
        config.enabled = channelObj["enabled"].toBool(true);
        config.minLevel = static_cast<LogLevel>(channelObj["minLevel"].toInt(LOG_INFO));
        config.audience = static_cast<LogAudience>(channelObj["audience"].toInt(AUDIENCE_ALL));
        config.showTimestamp = channelObj["showTimestamp"].toBool(true);
        config.showCategory = channelObj["showCategory"].toBool(true);
        config.showLevel = channelObj["showLevel"].toBool(true);
        config.showThreadInfo = channelObj["showThreadInfo"].toBool(false);
        config.maxFileSize = static_cast<qint64>(channelObj["maxFileSize"].toDouble(10 * 1024 * 1024));
        config.maxBackupFiles = channelObj["maxBackupFiles"].toInt(5);

        // Загружаем категории
        QJsonArray includeArray = channelObj["includeCategories"].toArray();
        for (const QJsonValue& val : includeArray) {
            config.includeCategories.insert(val.toString());
        }

        QJsonArray excludeArray = channelObj["excludeCategories"].toArray();
        for (const QJsonValue& val : excludeArray) {
            config.excludeCategories.insert(val.toString());
        }

        m_channels[id] = config;
    }

    LOG_CAT_INFO("CONFIG",
                 QString("Конфигурация логирования загружена из: %1").arg(filename));
    return true;
}

void LogConfig::setMinLevel(const QString& channelId, LogLevel level)
{
    QMutexLocker locker(&m_mutex);

    auto it = m_channels.find(channelId);
    if (it != m_channels.end()) {
        it->minLevel = level;
        emit configChanged(channelId);

        LOG_CAT_DEBUG("CONFIG",
                      QString("Установлен уровень %1 для канала '%2'").arg(level).arg(channelId));
    }
}

void LogConfig::setAudience(const QString& channelId, LogAudience audience)
{
    QMutexLocker locker(&m_mutex);

    auto it = m_channels.find(channelId);
    if (it != m_channels.end()) {
        it->audience = audience;
        emit configChanged(channelId);

        LOG_CAT_DEBUG("CONFIG",
                      QString("Установлена аудитория %1 для канала '%2'").arg(audience).arg(channelId));
    }
}
