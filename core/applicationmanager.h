#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <memory>

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

    // Геттеры для доступа к компонентам
    UDPClient* udpClient() const { return m_udpClient.get(); }
    PPBCommunication* communication() const { return m_communication.get(); }
    PPBController* controller() const { return m_controller.get(); }
    TesterWindow* mainWindow() const { return m_mainWindow.get(); }

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

    void cleanup();

private:
    static ApplicationManager* m_instance;
    static QMutex m_instanceMutex;

    bool m_initialized;

    // Коммуникационный поток (UDP + PPBCommunication + communicationengine)
    QThread* m_communicationThread;

    // Компоненты приложения
    std::unique_ptr<UDPClient> m_udpClient;
    std::unique_ptr<PPBCommunication> m_communication;
    std::unique_ptr<PPBController> m_controller;
    std::unique_ptr<TesterWindow> m_mainWindow;

    QMutex m_shutdownMutex;
};

#endif // APPLICATIONMANAGER_H
