// loguimanager.h
#ifndef LOGUIMANAGER_H
#define LOGUIMANAGER_H

#include <QObject>
#include <QComboBox>
#include "logconfig.h"

class LogUIManager : public QObject
{
    Q_OBJECT

public:
    explicit LogUIManager(QObject* parent = nullptr);

    // Используем ОДИН метод для установки комбобокса уровня
    void setupLevelComboBox(QComboBox* levelCombo);

    // Получение текущих настроек
    LogLevel getCurrentLevel() const;

public slots:
    void onLevelChanged(int index);

private:
    void updateUIConfig();

    QComboBox* m_levelCombo;
};

#endif // LOGUIMANAGER_H
