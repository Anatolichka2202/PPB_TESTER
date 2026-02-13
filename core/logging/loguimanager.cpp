// loguimanager.cpp
#include "loguimanager.h"
#include <QComboBox>

LogUIManager::LogUIManager(QObject* parent)
    : QObject(parent), m_levelCombo(nullptr)
{
}

void LogUIManager::setupLevelComboBox(QComboBox* levelCombo)
{
    m_levelCombo = levelCombo;

    // Наполняем комбобокс уровней
    levelCombo->clear();
    levelCombo->addItem("DEBUG", LOG_DEBUG);
    levelCombo->addItem("INFO", LOG_INFO);
    levelCombo->addItem("WARNING", LOG_WARNING);
    levelCombo->addItem("ERROR", LOG_ERROR);
    levelCombo->setCurrentIndex(1); // INFO по умолчанию

    // Подключаем сигнал
    connect(levelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogUIManager::onLevelChanged);

    // Применяем начальные настройки
    updateUIConfig();
}

void LogUIManager::onLevelChanged(int index)
{
    Q_UNUSED(index);
    updateUIConfig();
}

void LogUIManager::updateUIConfig()
{
    if (!m_levelCombo) return;

    LogConfig& config = LogConfig::instance();

    // Устанавливаем уровень для UI канала
    LogLevel level = static_cast<LogLevel>(m_levelCombo->currentData().toInt());
    config.setUIMinLevel(level);
}

LogLevel LogUIManager::getCurrentLevel() const
{
    if (m_levelCombo) {
        return static_cast<LogLevel>(m_levelCombo->currentData().toInt());
    }
    return LOG_INFO;
}
