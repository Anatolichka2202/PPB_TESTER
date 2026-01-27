#include "applicationmanager.h"
#include "udpclient.h"
#include "ppbcommunication.h"
#include "../gui/ppbcontroller.h"
#include "../gui/testerwindow.h"
#include "logger.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

ApplicationManager* ApplicationManager::m_instance = nullptr;
QMutex ApplicationManager::m_instanceMutex;

ApplicationManager& ApplicationManager::instance()
{
    QMutexLocker locker(&m_instanceMutex);
    if (!m_instance) {
        m_instance = new ApplicationManager(qApp);
    }
    return *m_instance;
}

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_udpClient(nullptr)   //юдп объект
    , m_udpThread(nullptr)  //юдп поток
    , m_communication(nullptr)  //объект комуникации
    , m_communicationThread(nullptr)   //поток коммуникации
    , m_controller(nullptr)     //объект контроллера
    , m_mainWindow(nullptr)     //объект окна
{
}

ApplicationManager::~ApplicationManager()
{
    shutdown();
}

bool ApplicationManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    try {
        LOG_INFO("[APPLICATION] Начало инициализации приложения...");

        // 1. Инициализируем UDPClient (должен быть первым)
        initializeUDPClient();
        // Даем потоку UDPClient время на запуск
        QThread::msleep(100);

        // 2. Инициализируем PPBCommunication
        initializePPBCommunication();
        // Даем потоку PPBCommunication время на запуск
        QThread::msleep(100);

        // 3. Инициализируем контроллер
        initializeController();

        // 4. Инициализируем главное окно
        initializeMainWindow();

        m_initialized = true;
        LOG_INFO("[APPLICATION] Инициализация приложения завершена успешно");
        emit initializationComplete();

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[APPLICATION] " + QString("Ошибка инициализации: %1").arg(e.what()));
        emit initializationFailed(e.what());
        return false;
    }
}

void ApplicationManager::initializeUDPClient()
{
    LOG_INFO("[APPLICATION] Инициализация UDPClient...");

    // 1. Создаем UDPClient (НЕ синглтон!)
    m_udpClient = new UDPClient();

    // 2. Создаем поток для UDPClient
    m_udpThread = new QThread();
    m_udpThread->setObjectName("UDPThread");

    // 3. Перемещаем UDPClient в его поток
    m_udpClient->moveToThread(m_udpThread);

    // 4. Подключаем сигналы до запуска потока
    QEventLoop initLoop;
    bool initSuccess = false;

    // Сигнал об успешной инициализации
    QMetaObject::Connection conn = QObject::connect(
        m_udpClient, &UDPClient::initialized,
        [&]() {
            LOG_INFO("[APPLICATION] UDPClient сообщил об успешной инициализации");
            initSuccess = true;
            QMetaObject::invokeMethod(&initLoop, "quit", Qt::QueuedConnection);
        }
        );

    // Сигнал об ошибке
    QObject::connect(
        m_udpClient, &UDPClient::errorOccurred,
        [&](const QString& error) {
            LOG_ERROR(QString("[APPLICATION] Ошибка UDPClient: %1").arg(error));
            if (!initSuccess) {
                QMetaObject::invokeMethod(&initLoop, "quit", Qt::QueuedConnection);
            }
        }
        );

    // Таймаут
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &initLoop, [&]() {
        if (!initSuccess) {
            LOG_ERROR("[APPLICATION] Таймаут инициализации UDPClient");
            initLoop.quit();
        }
    });

    // 5. Запускаем поток
    LOG_INFO("[APPLICATION] Запуск потока UDPClient...");
    m_udpThread->start();

    // Небольшая пауза для запуска потока
    QThread::msleep(50);

    // 6. Инициализируем UDPClient в его потоке
    QMetaObject::invokeMethod(m_udpClient, "initializeInThread", Qt::QueuedConnection);

    // 7. Запускаем таймаут и ждем
    timeoutTimer.start(3000); // 3 секунды таймаут
    initLoop.exec();

    // 8. Отключаем временное соединение
    QObject::disconnect(conn);

    if (!initSuccess) {
        // Останавливаем поток в случае ошибки
        m_udpThread->quit();
        m_udpThread->wait();
        delete m_udpThread;
        delete m_udpClient;
        m_udpThread = nullptr;
        m_udpClient = nullptr;

        throw std::runtime_error("Не удалось инициализировать UDPClient");
    }

    LOG_INFO("[APPLICATION] UDPClient успешно инициализирован");

    // 9. Подключаем остальные сигналы для нормальной работы
    QObject::connect(m_udpThread, &QThread::finished, m_udpClient, &UDPClient::deleteLater);
    QObject::connect(m_udpThread, &QThread::finished, m_udpThread, &QThread::deleteLater);
}

void ApplicationManager::initializePPBCommunication()
{
    LOG_INFO("[APPLICATION] Инициализация PPBCommunication...");

    if (!m_udpClient || !m_udpClient->isBound()) {
        throw std::runtime_error("UDPClient не инициализирован");
    }

    // 1. Создаем PPBCommunication
    m_communication = new PPBCommunication();

    // 2. Создаем и настраиваем поток
    m_communicationThread = new QThread();
    m_communicationThread->setObjectName("CommunicationThread");

    // 3. Перемещаем в поток
    m_communication->moveToThread(m_communicationThread);

    // 4. Подключаем завершение
    QObject::connect(m_communicationThread, &QThread::finished,
                     m_communication, &PPBCommunication::deleteLater);
    QObject::connect(m_communicationThread, &QThread::finished,
                     m_communicationThread, &QThread::deleteLater);

    // 5. Запускаем поток
    LOG_INFO("[APPLICATION] Запуск потока PPBCommunication...");
    m_communicationThread->start();

    // 6. Ждем запуска потока
    QThread::msleep(100);

    // 7. Используем событийный цикл для безопасной инициализации
    QEventLoop initLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool initSuccess = false;
    QString initError;

    // Подключаем сигналы от PPBCommunication
    QObject::connect(m_communication, &PPBCommunication::initialized,
                     &initLoop, [&]() {
                         initSuccess = true;
                         initLoop.quit();
                     });

    QObject::connect(m_communication, &PPBCommunication::errorOccurred,
                     &initLoop, [&](const QString& error) {
                         initError = error;
                         initLoop.quit();
                     });

    // Таймаут
    timeoutTimer.start(5000); // 5 секунд
    QObject::connect(&timeoutTimer, &QTimer::timeout, &initLoop, [&]() {
        if (!initSuccess) {
            initError = "Таймаут инициализации PPBCommunication";
            initLoop.quit();
        }
    });

    // 8. Запускаем инициализацию в потоке объекта
    QMetaObject::invokeMethod(m_communication, "initialize", Qt::QueuedConnection,
                              Q_ARG(UDPClient*, m_udpClient));

    // 9. Ждем завершения инициализации
    initLoop.exec();

    if (!initSuccess) {
        QString errorMsg = initError.isEmpty() ? "Неизвестная ошибка" : initError;
        LOG_ERROR(QString("[APPLICATION] Ошибка инициализации PPBCommunication: %1").arg(errorMsg));
        throw std::runtime_error(errorMsg.toStdString());
    }

    LOG_INFO("[APPLICATION] PPBCommunication успешно инициализирован");
}

void ApplicationManager::initializeController()
{
    LOG_INFO("[APPLICATION] Инициализация PPBController...");

    m_controller = new PPBController(m_communication);

    // Используем прямое присваивание, так как поле public
    // Или добавьте метод setCommunication, если хотите

    LOG_INFO("[APPLICATION] PPBController успешно инициализирован");
}

void ApplicationManager::initializeMainWindow()
{
    LOG_INFO("[APPLICATION] Создание главного окна...");

    // Передаем контроллер в конструктор TesterWindow
    m_mainWindow = new TesterWindow(m_controller);

    LOG_INFO("[APPLICATION] Главное окно создано");
}

void ApplicationManager::shutdown()
{
    LOG_INFO("[APPLICATION] Завершение работы приложения...");

    // Блокируем мьютекс перед доступом к объектам
    QMutexLocker locker(&m_shutdownMutex);

    // 1. Удаляем главное окно (в основном потоке)
    if (m_mainWindow) {
        m_mainWindow->close();
        // Не удаляем сразу, даем Qt самому управлять
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
    }

    // 2. Останавливаем контроллер
    if (m_controller) {
        m_controller->deleteLater();
        m_controller = nullptr;
    }

    // 3. Останавливаем потоки в ПРАВИЛЬНОМ порядке
    // Сначала останавливаем прием данных
    if (m_communication) {
        QMetaObject::invokeMethod(m_communication, "stop", Qt::BlockingQueuedConnection);
    }

    // Затем останавливаем потоки
    if (m_communicationThread && m_communicationThread->isRunning()) {
        m_communicationThread->quit();
        m_communicationThread->wait(2000); // Ждем 2 секунды
        if (m_communicationThread->isRunning()) {
            m_communicationThread->terminate(); // Принудительно, если не останавливается
            m_communicationThread->wait();
        }
        delete m_communicationThread;
        m_communicationThread = nullptr;
    }

    if (m_udpThread && m_udpThread->isRunning()) {
        m_udpThread->quit();
        m_udpThread->wait(2000);
        if (m_udpThread->isRunning()) {
            m_udpThread->terminate();
            m_udpThread->wait();
        }
        delete m_udpThread;
        m_udpThread = nullptr;
    }

    // 4. Удаляем объекты (они уже должны быть удалены deleteLater)
    m_udpClient = nullptr;
    m_communication = nullptr;

    m_initialized = false;
    LOG_INFO("[APPLICATION] Приложение завершено");
}
