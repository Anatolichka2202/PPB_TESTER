#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class UDPClient;
class PPBCommunication;
class PPBController;
class TesterWindow;

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    static ApplicationManager& instance();

    bool initialize();
    void shutdown();

    UDPClient* udpClient() const { return m_udpClient; }
    PPBController* controller() const { return m_controller; }
    TesterWindow* mainWindow() const { return m_mainWindow; }

    bool isInitialized() const { return m_initialized; }

signals:
    void initializationComplete();
    void initializationFailed(const QString& error);

private:
    explicit ApplicationManager(QObject* parent = nullptr);
    ~ApplicationManager();

    void initializeUDPClient();
    void initializePPBCommunication();
    void initializeController();
    void initializeMainWindow();

private:
    static ApplicationManager* m_instance;
    static QMutex m_instanceMutex;

    bool m_initialized;

    UDPClient* m_udpClient;
    QThread* m_udpThread;

    PPBCommunication* m_communication;
    QThread* m_communicationThread;

    PPBController* m_controller;
    TesterWindow* m_mainWindow;

     QMutex m_shutdownMutex;
};
#endif // APPLICATIONMANAGER_H
