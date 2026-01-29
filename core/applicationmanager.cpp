#include "applicationmanager.h"
#include "communication/udpclient.h"
#include "communication/ppbcommunication.h"
#include "../gui/ppbcontroller.h"
#include "../gui/testerwindow.h"
#include "../core/logger.h"
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
    , m_communicationThread(nullptr)
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

    LOG_INFO("[APPLICATION] Начало инициализации приложения...");

    try {
        // Создаем коммуникационный поток
        m_communicationThread = new QThread();
        m_communicationThread->setObjectName("CommunicationThread");

        // 1. Инициализируем UDPClient в коммуникационном потоке
        initializeUDPClient();

        // 2. Инициализируем PPBCommunication в том же потоке
        initializePPBCommunication();

        // 3. Инициализируем контроллер (в основном потоке)
        initializeController();

        // 4. Инициализируем главное окно (в основном потоке)
        initializeMainWindow();

        m_initialized = true;
        LOG_INFO("[APPLICATION] Инициализация приложения завершена успешно");
        emit initializationComplete();

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("[APPLICATION] Ошибка инициализации: " + QString(e.what()));

        // В случае ошибки очищаем ресурсы
        cleanup();

        emit initializationFailed(e.what());
        return false;
    } catch (...) {
        LOG_ERROR("[APPLICATION] Неизвестная ошибка инициализации");

        cleanup();

        emit initializationFailed("Неизвестная ошибка");
        return false;
    }
}

void ApplicationManager::initializeUDPClient()
{
    LOG_INFO("[APPLICATION] Инициализация UDPClient...");

    // 1. Создаем UDPClient
    m_udpClient = std::make_unique<UDPClient>();

    // 2. Перемещаем в коммуникационный поток
    m_udpClient->moveToThread(m_communicationThread);

    // 3. Создаем событийный цикл для ожидания инициализации
    QEventLoop initLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool initSuccess = false;
    QString initError;

    // 4. Подключаем сигналы инициализации
    auto conn = QObject::connect(m_udpClient.get(), &UDPClient::initialized,
                                 &initLoop, [&]() {
                                     LOG_INFO("[APPLICATION] UDPClient инициализирован");
                                     initSuccess = true;
                                     initLoop.quit();
                                 });

    QObject::connect(m_udpClient.get(), &UDPClient::errorOccurred,
                     &initLoop, [&](const QString& error) {
                         LOG_ERROR("[APPLICATION] Ошибка UDPClient: " + error);
                         initError = error;
                         initLoop.quit();
                     });

    // 5. Настраиваем таймаут
    timeoutTimer.start(5000); // 5 секунд
    QObject::connect(&timeoutTimer, &QTimer::timeout, &initLoop, [&]() {
        if (!initSuccess) {
            initError = "Таймаут инициализации UDPClient";
            initLoop.quit();
        }
    });

    // 6. Запускаем поток коммуникаций
    LOG_INFO("[APPLICATION] Запуск коммуникационного потока...");
    m_communicationThread->start();

    // 7. Запускаем инициализацию UDPClient в его потоке
    QMetaObject::invokeMethod(m_udpClient.get(), "initializeInThread", Qt::QueuedConnection);

    // 8. Ждем завершения инициализации
    initLoop.exec();

    // 9. Отключаем временные соединения
    QObject::disconnect(conn);

    // 10. Проверяем результат
    if (!initSuccess) {
        throw std::runtime_error(initError.toStdString());
    }

    LOG_INFO("[APPLICATION] UDPClient успешно инициализирован (порт: " +
             QString::number(m_udpClient->boundPort()) + ")");
}

void ApplicationManager::initializePPBCommunication()
{
    LOG_INFO("[APPLICATION] Инициализация PPBCommunication...");

    // Проверяем, что UDPClient инициализирован
    if (!m_udpClient || !m_udpClient->isBound()) {
        throw std::runtime_error("UDPClient не инициализирован");
    }

    // 1. Создаем PPBCommunication
    m_communication = std::make_unique<PPBCommunication>();

    // 2. Перемещаем в тот же коммуникационный поток
    m_communication->moveToThread(m_communicationThread);

    // 3. Создаем событийный цикл для ожидания инициализации
    QEventLoop initLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool initSuccess = false;
    QString initError;

    // 4. Подключаем сигналы инициализации
    auto conn = QObject::connect(m_communication.get(), &PPBCommunication::initialized,
                                 &initLoop, [&]() {
                                     LOG_INFO("[APPLICATION] PPBCommunication инициализирован");
                                     initSuccess = true;
                                     initLoop.quit();
                                 });

    QObject::connect(m_communication.get(), &PPBCommunication::errorOccurred,
                     &initLoop, [&](const QString& error) {
                         LOG_ERROR("[APPLICATION] Ошибка PPBCommunication: " + error);
                         initError = error;
                         initLoop.quit();
                     });

    // 5. Настраиваем таймаут
    timeoutTimer.start(5000); // 5 секунд
    QObject::connect(&timeoutTimer, &QTimer::timeout, &initLoop, [&]() {
        if (!initSuccess) {
            initError = "Таймаут инициализации PPBCommunication";
            initLoop.quit();
        }
    });

    // 6. Запускаем инициализацию PPBCommunication в его потоке
    QMetaObject::invokeMethod(m_communication.get(), "initialize",
                              Qt::QueuedConnection,
                              Q_ARG(UDPClient*, m_udpClient.get()));

    // 7. Ждем завершения инициализации
    initLoop.exec();

    // 8. Отключаем временные соединения
    QObject::disconnect(conn);

    // 9. Проверяем результат
    if (!initSuccess) {
        throw std::runtime_error(initError.toStdString());
    }

    LOG_INFO("[APPLICATION] PPBCommunication успешно инициализирован");
}

void ApplicationManager::initializeController()
{
    LOG_INFO("[APPLICATION] Инициализация PPBController...");

    // Создаем контроллер в основном потоке
    m_controller = std::make_unique<PPBController>(m_communication.get());

    LOG_INFO("[APPLICATION] PPBController успешно инициализирован");
}

void ApplicationManager::initializeMainWindow()
{
    LOG_INFO("[APPLICATION] Создание главного окна...");

    // Создаем главное окно в основном потоке
    m_mainWindow = std::make_unique<TesterWindow>(m_controller.get());

    LOG_INFO("[APPLICATION] Главное окно создано");
}

void ApplicationManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    LOG_INFO("[APPLICATION] Завершение работы приложения...");

    QMutexLocker locker(&m_shutdownMutex);

    // 1. Закрываем главное окно
    if (m_mainWindow) {
        LOG_INFO("[APPLICATION] Закрытие главного окна...");
        m_mainWindow->close();
        m_mainWindow.reset();
    }

    // 2. Останавливаем контроллер
    if (m_controller) {
        LOG_INFO("[APPLICATION] Остановка контроллера...");
        m_controller.reset();
    }

    // 3. Останавливаем коммуникации
    if (m_communication) {
        LOG_INFO("[APPLICATION] Остановка PPBCommunication...");

        // Вызываем stop() в потоке объекта
        QEventLoop stopLoop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);

        bool stopSuccess = false;

        // Подключаем сигнал завершения (если есть) или используем таймаут
        QObject::connect(&timeoutTimer, &QTimer::timeout, &stopLoop, [&]() {
            stopLoop.quit();
        });

        // Вызываем stop в потоке объекта
        QMetaObject::invokeMethod(m_communication.get(), "stop", Qt::QueuedConnection,
                                  Q_ARG(bool, true));

        // Ждем завершения с таймаутом
        timeoutTimer.start(2000); // 2 секунды
        stopLoop.exec();

        m_communication.reset();
    }

    // 4. Останавливаем UDPClient (автоматически при уничтожении)
    if (m_udpClient) {
        LOG_INFO("[APPLICATION] Остановка UDPClient...");
        m_udpClient.reset();
    }

    // 5. Останавливаем коммуникационный поток
    if (m_communicationThread && m_communicationThread->isRunning()) {
        LOG_INFO("[APPLICATION] Остановка коммуникационного потока...");

        m_communicationThread->quit();

        if (!m_communicationThread->wait(3000)) { // Ждем 3 секунды
            LOG_WARNING("[APPLICATION] Принудительное завершение коммуникационного потока...");
            m_communicationThread->terminate();
            m_communicationThread->wait();
        }

        delete m_communicationThread;
        m_communicationThread = nullptr;
    }

    m_initialized = false;
    LOG_INFO("[APPLICATION] Приложение завершено");
}

void ApplicationManager::cleanup()
{
    LOG_INFO("[APPLICATION] Очистка ресурсов...");

    // Аналогично shutdown, но без блокировки и с обработкой ошибок
    try {
        if (m_mainWindow) {
            m_mainWindow->close();
            m_mainWindow.reset();
        }

        m_controller.reset();
        m_communication.reset();
        m_udpClient.reset();

        if (m_communicationThread && m_communicationThread->isRunning()) {
            m_communicationThread->quit();
            if (!m_communicationThread->wait(1000)) {
                m_communicationThread->terminate();
                m_communicationThread->wait();
            }
            delete m_communicationThread;
            m_communicationThread = nullptr;
        }

        m_initialized = false;

    } catch (const std::exception& e) {
        LOG_ERROR("[APPLICATION] Ошибка при очистке ресурсов: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("[APPLICATION] Неизвестная ошибка при очистке ресурсов");
    }
}
