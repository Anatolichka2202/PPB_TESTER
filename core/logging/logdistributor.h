// logdistributor.h
#ifndef LOGDISTRIBUTOR_H
#define LOGDISTRIBUTOR_H

#include <QObject>
#include <QFile>
#include <QMutex>
#include "../logentry.h"

class LogDistributor : public QObject
{
    Q_OBJECT

public:
    static LogDistributor& instance();

    void init();

    void distribute(const LogEntry& entry);

signals:
    void logForUI(const LogEntry& entry);

private:
    explicit LogDistributor(QObject* parent = nullptr);

    void openLogFiles();

    QFile m_techFile;    // Технический файл
    QFile m_operFile;    // Операционный файл
    QFile m_allFile;     // Общий файл

    QMutex m_mutex;
};

#endif // LOGDISTRIBUTOR_H
