#include "applicationmanager.h"
#include "communication/udpclient.h"
#include "communication/ppbcommunication.h"
#include "../gui/ppbcontroller.h"
#include "../gui/testerwindow.h"
#include "../core/logwrapper.h"
#include "../core/logger.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>

ApplicationManager* ApplicationManager::m_instance = nullptr;
QMutex ApplicationManager::m_instanceMutex;

// Диагностические константы
const int UDP_INIT_TIMEOUT_MS = 2000;        // 2 секунды вместо 5
const int PPB_INIT_TIMEOUT_MS = 2000;        // 2 секунды вместо 5
const int THREAD_START_TIMEOUT_MS = 1000;    // 1 секунда для запуска потока
const int THREAD_SHUTDOWN_TIMEOUT_MS = 500;  // 500 мс для остановки потока

ApplicationManager& ApplicationManager::instance()
{
    QMutexLocker locker(&m_instanceMutex);
    if (!m_instance) {
        LOG_CAT_INFO("[APPLICATION]", "Создание экземпляра ApplicationManager");
        m_instance = new ApplicationManager(qApp);
    }
    return *m_instance;
}

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_communicationThread(nullptr)
{
    LOG_CAT_INFO("[APPLICATION]", "Конструктор ApplicationManager");
}

ApplicationManager::~ApplicationManager()
{
    LOG_CAT_INFO("[APPLICATION]", "Деструктор ApplicationManager");
    shutdown();
}

bool ApplicationManager::initialize()
{
    if (m_initialized) {
        LOG_CAT_INFO("[APPLICATION]", "Повторная инициализация - уже инициализирован");
        return true;
    }

    LOG_CAT_INFO("[APPLICATION]", "========== НАЧАЛО ИНИЦИАЛИЗАЦИИ ПРИЛОЖЕНИЯ ==========");

    try {
        // Создаем коммуникационный поток
        LOG_CAT_INFO("[APPLICATION]", "Создание коммуникационного потока...");
        m_communicationThread = new QThread();
        m_communicationThread->setObjectName("CommunicationThread");
        LOG_CAT_INFO("[APPLICATION]", QString("Поток создан: %1").arg((quintptr)m_communicationThread, QT_POINTER_SIZE * 2, 16, QChar('0')));

        // 1. Инициализируем UDPClient в коммуникационном потоке
        LOG_CAT_INFO("[APPLICATION]", "Этап 1: Инициализация UDPClient");
        initializeUDPClient();

        // 2. Инициализируем PPBCommunication в том же потоке
        LOG_CAT_INFO("[APPLICATION]", "Этап 2: Инициализация PPBCommunication");
        initializePPBCommunication();

        // 3. Инициализируем контроллер (в основном потоке)
        LOG_CAT_INFO("[APPLICATION]", "Этап 3: Инициализация контроллера");
        initializeController();

        // 4. Инициализируем главное окно (в основном потоке)
        LOG_CAT_INFO("[APPLICATION]", "Этап 4: Инициализация главного окна");
        initializeMainWindow();

        m_initialized = true;
        LOG_CAT_INFO("[APPLICATION]", "========== ИНИЦИАЛИЗАЦИЯ ПРИЛОЖЕНИЯ УСПЕШНО ЗАВЕРШЕНА ==========");
        emit initializationComplete();

        return true;

    } catch (const std::exception& e) {
        LOG_CAT_ERROR("[APPLICATION]", QString("ОШИБКА инициализации: %1").arg(e.what()));

        // В случае ошибки очищаем ресурсы
        cleanup();

        emit initializationFailed(e.what());
        return false;
    } catch (...) {
        LOG_CAT_ERROR("[APPLICATION]", "НЕИЗВЕСТНАЯ ошибка инициализации");

        cleanup();

        emit initializationFailed("Неизвестная ошибка");
        return false;
    }
}

void ApplicationManager::initializeUDPClient()
{
    LOG_CAT_INFO("[APPLICATION]", "----- Инициализация UDPClient -----");

    // Генерируем уникальный ID для этого цикла ожидания
    QString loopId = QString("UDPInit_%1").arg(QDateTime::currentMSecsSinceEpoch());
    LOG_CAT_DEBUG("[APPLICATION]", QString("Loop ID: %1").arg(loopId));

    // 1. Создаем UDPClient
    LOG_CAT_INFO("[APPLICATION]", "Создание UDPClient...");
    m_udpClient = std::make_unique<UDPClient>();
    LOG_CAT_DEBUG("[APPLICATION]", QString("UDPClient создан: %1").arg((quintptr)m_udpClient.get(), QT_POINTER_SIZE * 2, 16, QChar('0')));

    // 2. Перемещаем в коммуникационный поток
    LOG_CAT_INFO("[APPLICATION]", "Перемещение UDPClient в коммуникационный поток...");
    m_udpClient->moveToThread(m_communicationThread);
    LOG_CAT_DEBUG("[APPLICATION]", "UDPClient перемещен");

    // 3. Создаем событийный цикл для ожидания инициализации
    LOG_CAT_INFO("[APPLICATION]", "Создание event loop для ожидания инициализации UDPClient");
    QEventLoop initLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool initSuccess = false;
    QString initError;
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    // 4. Подключаем сигналы инициализации - ВАЖНО: делаем это ПОСЛЕ создания таймера
    LOG_CAT_DEBUG("[APPLICATION]", "Подключение сигналов UDPClient...");

    auto conn1 = QObject::connect(m_udpClient.get(), &UDPClient::initialized,
                                  &initLoop, [&, loopId]() {
                                      qint64 elapsed = elapsedTimer.elapsed();
                                      LOG_CAT_INFO("[APPLICATION]",
                                                   QString("[%1] UDPClient инициализирован (через %2 мс)")
                                                       .arg(loopId).arg(elapsed));
                                      initSuccess = true;
                                      initLoop.quit();
                                  }, Qt::QueuedConnection);  // Явно указываем тип соединения

    auto conn2 = QObject::connect(m_udpClient.get(), &UDPClient::errorOccurred,
                                  &initLoop, [&, loopId](const QString& error) {
                                      qint64 elapsed = elapsedTimer.elapsed();
                                      LOG_CAT_ERROR("[APPLICATION]",
                                                    QString("[%1] Ошибка UDPClient (через %2 мс): %3")
                                                        .arg(loopId).arg(elapsed).arg(error));
                                      initError = error;
                                      initLoop.quit();
                                  }, Qt::QueuedConnection);

    // 5. Настраиваем таймаут
    LOG_CAT_INFO("[APPLICATION]", QString("Установка таймаута инициализации: %1 мс").arg(UDP_INIT_TIMEOUT_MS));
    timeoutTimer.start(UDP_INIT_TIMEOUT_MS);

    auto conn3 = QObject::connect(&timeoutTimer, &QTimer::timeout, &initLoop, [&, loopId]() {
        qint64 elapsed = elapsedTimer.elapsed();
        if (!initSuccess) {
            LOG_CAT_WARNING("[APPLICATION]",
                            QString("[%1] ТАЙМАУТ инициализации UDPClient (через %2 мс)").arg(loopId).arg(elapsed));
            initError = QString("Таймаут инициализации UDPClient (%1 мс)").arg(elapsed);
            initLoop.quit();
        } else {
            LOG_CAT_DEBUG("[APPLICATION]",
                          QString("[%1] Таймаут сработал, но init уже завершен (через %2 мс)").arg(loopId).arg(elapsed));
        }
    }, Qt::QueuedConnection);

    // 6. Запускаем поток коммуникаций
    LOG_CAT_INFO("[APPLICATION]", "Запуск коммуникационного потока...");

    if (!m_communicationThread->isRunning()) {
        m_communicationThread->start();
        LOG_CAT_DEBUG("[APPLICATION]", "Поток запущен");

        // Ждем немного, чтобы поток действительно запустился
        if (!m_communicationThread->wait(THREAD_START_TIMEOUT_MS)) {
            LOG_CAT_WARNING("[APPLICATION]", "Поток не запустился в течение таймаута");
        }
    } else {
        LOG_CAT_WARNING("[APPLICATION]", "Поток уже запущен!");
    }

    // 7. Запускаем инициализацию UDPClient в его потоке
    LOG_CAT_INFO("[APPLICATION]", "Запуск initializeInThread() для UDPClient...");
    QMetaObject::invokeMethod(m_udpClient.get(), "initializeInThread", Qt::QueuedConnection);
    LOG_CAT_DEBUG("[APPLICATION]", "Метод поставлен в очередь");

    // 8. Ждем завершения инициализации
    LOG_CAT_INFO("[APPLICATION]", "Вход в event loop для ожидания инициализации UDPClient");
    qint64 beforeLoop = elapsedTimer.elapsed();
    initLoop.exec();
    qint64 afterLoop = elapsedTimer.elapsed();

    LOG_CAT_INFO("[APPLICATION]",
                 QString("Выход из event loop. Длительность ожидания: %1 мс").arg(afterLoop - beforeLoop));

    // 9. Отключаем временные соединения
    QObject::disconnect(conn1);
    QObject::disconnect(conn2);
    QObject::disconnect(conn3);

    LOG_CAT_DEBUG("[APPLICATION]", "Временные соединения отключены");

    // 10. Проверяем результат
    if (!initSuccess) {
        LOG_CAT_ERROR("[APPLICATION]",
                      QString("Инициализация UDPClient НЕ УДАЛАСЬ: %1").arg(initError));
        throw std::runtime_error(initError.toStdString());
    }

    LOG_CAT_INFO("[APPLICATION]",
                 QString("UDPClient успешно инициализирован (порт: %1, общее время: %2 мс)")
                     .arg(m_udpClient->boundPort())
                     .arg(elapsedTimer.elapsed()));
    LOG_CAT_INFO("[APPLICATION]", "----- UDPClient инициализирован -----");
}

void ApplicationManager::initializePPBCommunication()
{
    LOG_CAT_INFO("[APPLICATION]", "----- Инициализация PPBCommunication -----");

    // Генерируем уникальный ID для этого цикла ожидания
    QString loopId = QString("PPBInit_%1").arg(QDateTime::currentMSecsSinceEpoch());
    LOG_CAT_DEBUG("[APPLICATION]", QString("Loop ID: %1").arg(loopId));

    // Проверяем, что UDPClient инициализирован
    if (!m_udpClient) {
        LOG_CAT_ERROR("[APPLICATION]", "UDPClient не создан!");
        throw std::runtime_error("UDPClient не создан");
    }

    if (!m_udpClient->isBound()) {
        LOG_CAT_ERROR("[APPLICATION]", "UDPClient не привязан к порту!");
        throw std::runtime_error("UDPClient не привязан к порту");
    }

    LOG_CAT_INFO("[APPLICATION]", "UDPClient проверен: isBound = " + QString(m_udpClient->isBound() ? "true" : "false"));

    // 1. Создаем PPBCommunication
    LOG_CAT_INFO("[APPLICATION]", "Создание PPBCommunication...");
    m_communication = std::make_unique<PPBCommunication>();
    LOG_CAT_DEBUG("[APPLICATION]", QString("PPBCommunication создан: %1").arg((quintptr)m_communication.get(), QT_POINTER_SIZE * 2, 16, QChar('0')));

    // 2. Перемещаем в тот же коммуникационный поток
    LOG_CAT_INFO("[APPLICATION]", "Перемещение PPBCommunication в коммуникационный поток...");
    m_communication->moveToThread(m_communicationThread);
    LOG_CAT_DEBUG("[APPLICATION]", "PPBCommunication перемещен");

    // 3. Создаем событийный цикл для ожидания инициализации
    LOG_CAT_INFO("[APPLICATION]", "Создание event loop для ожидания инициализации PPBCommunication");
    QEventLoop initLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool initSuccess = false;
    QString initError;
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    // 4. Подключаем сигналы инициализации
    LOG_CAT_DEBUG("[APPLICATION]", "Подключение сигналов PPBCommunication...");

    auto conn1 = QObject::connect(m_communication.get(), &PPBCommunication::initialized,
                                  &initLoop, [&, loopId]() {
                                      qint64 elapsed = elapsedTimer.elapsed();
                                      LOG_CAT_INFO("[APPLICATION]",
                                                   QString("[%1] PPBCommunication инициализирован (через %2 мс)")
                                                       .arg(loopId).arg(elapsed));
                                      initSuccess = true;
                                      initLoop.quit();
                                  }, Qt::QueuedConnection);

    auto conn2 = QObject::connect(m_communication.get(), &PPBCommunication::errorOccurred,
                                  &initLoop, [&, loopId](const QString& error) {
                                      qint64 elapsed = elapsedTimer.elapsed();
                                      LOG_CAT_ERROR("[APPLICATION]",
                                                    QString("[%1] Ошибка PPBCommunication (через %2 мс): %3")
                                                        .arg(loopId).arg(elapsed).arg(error));
                                      initError = error;
                                      initLoop.quit();
                                  }, Qt::QueuedConnection);

    // 5. Настраиваем таймаут
    LOG_CAT_INFO("[APPLICATION]", QString("Установка таймаута инициализации: %1 мс").arg(PPB_INIT_TIMEOUT_MS));
    timeoutTimer.start(PPB_INIT_TIMEOUT_MS);

    auto conn3 = QObject::connect(&timeoutTimer, &QTimer::timeout, &initLoop, [&, loopId]() {
        qint64 elapsed = elapsedTimer.elapsed();
        if (!initSuccess) {
            LOG_CAT_WARNING("[APPLICATION]",
                            QString("[%1] ТАЙМАУТ инициализации PPBCommunication (через %2 мс)").arg(loopId).arg(elapsed));
            initError = QString("Таймаут инициализации PPBCommunication (%1 мс)").arg(elapsed);
            initLoop.quit();
        } else {
            LOG_CAT_DEBUG("[APPLICATION]",
                          QString("[%1] Таймаут сработал, но init уже завершен (через %2 мс)").arg(loopId).arg(elapsed));
        }
    }, Qt::QueuedConnection);

    // 6. Запускаем инициализацию PPBCommunication в его потоке
    LOG_CAT_INFO("[APPLICATION]", "Запуск initialize() для PPBCommunication...");

    // Проверяем состояние потока перед вызовом
    if (!m_communicationThread->isRunning()) {
        LOG_CAT_ERROR("[APPLICATION]", "Коммуникационный поток НЕ ЗАПУЩЕН!");
        throw std::runtime_error("Коммуникационный поток не запущен");
    }

    QMetaObject::invokeMethod(m_communication.get(), "initialize",
                              Qt::QueuedConnection,
                              Q_ARG(UDPClient*, m_udpClient.get()));
    LOG_CAT_DEBUG("[APPLICATION]", "Метод поставлен в очередь");

    // 7. Ждем завершения инициализации
    LOG_CAT_INFO("[APPLICATION]", "Вход в event loop для ожидания инициализации PPBCommunication");
    qint64 beforeLoop = elapsedTimer.elapsed();
    initLoop.exec();
    qint64 afterLoop = elapsedTimer.elapsed();

    LOG_CAT_INFO("[APPLICATION]",
                 QString("Выход из event loop. Длительность ожидания: %1 мс").arg(afterLoop - beforeLoop));

    // 8. Отключаем временные соединения
    QObject::disconnect(conn1);
    QObject::disconnect(conn2);
    QObject::disconnect(conn3);

    LOG_CAT_DEBUG("[APPLICATION]", "Временные соединения отключены");

    // 9. Проверяем результат
    if (!initSuccess) {
        LOG_CAT_ERROR("[APPLICATION]",
                      QString("Инициализация PPBCommunication НЕ УДАЛАСЬ: %1").arg(initError));
        throw std::runtime_error(initError.toStdString());
    }

    LOG_CAT_INFO("[APPLICATION]",
                 QString("PPBCommunication успешно инициализирован (общее время: %1 мс)")
                     .arg(elapsedTimer.elapsed()));
    LOG_CAT_INFO("[APPLICATION]", "----- PPBCommunication инициализирован -----");
}

void ApplicationManager::initializeController()
{
    LOG_CAT_INFO("[APPLICATION]", "----- Инициализация PPBController -----");

    // Проверяем, что communication существует
    if (!m_communication) {
        LOG_CAT_ERROR("[APPLICATION]", "PPBCommunication не создан!");
        throw std::runtime_error("PPBCommunication не создан");
    }

    // Создаем контроллер в основном потоке
    LOG_CAT_INFO("[APPLICATION]", "Создание PPBController...");
    m_controller = std::make_unique<PPBController>(m_communication.get());
    LOG_CAT_DEBUG("[APPLICATION]", QString("PPBController создан: %1").arg((quintptr)m_controller.get(), QT_POINTER_SIZE * 2, 16, QChar('0')));

    LOG_CAT_INFO("[APPLICATION]", "PPBController успешно инициализирован");
    LOG_CAT_INFO("[APPLICATION]", "----- PPBController инициализирован -----");
}

void ApplicationManager::initializeMainWindow()
{
    LOG_CAT_INFO("[APPLICATION]", "----- Создание главного окна -----");

    // Проверяем, что контроллер существует
    if (!m_controller) {
        LOG_CAT_ERROR("[APPLICATION]", "PPBController не создан!");
        throw std::runtime_error("PPBController не создан");
    }

    // Создаем главное окно в основном потоке
    LOG_CAT_INFO("[APPLICATION]", "Создание TesterWindow...");
    m_mainWindow = std::make_unique<TesterWindow>(m_controller.get());
    LOG_CAT_DEBUG("[APPLICATION]", QString("TesterWindow создан: %1").arg((quintptr)m_mainWindow.get(), QT_POINTER_SIZE * 2, 16, QChar('0')));

    // Устанавливаем иконку (если нужно)
    LOG_CAT_DEBUG("[APPLICATION]", "Установка иконки окна...");
    m_mainWindow->setWindowIcon(QIcon("../bagger.png"));

    LOG_CAT_INFO("[APPLICATION]", "Главное окно создано");
    LOG_CAT_INFO("[APPLICATION]", "----- Главное окно создано -----");
}

void ApplicationManager::shutdown()
{
    if (!m_initialized) {
        LOG_CAT_DEBUG("[APPLICATION]", "shutdown: приложение не инициализировано");
        return;
    }

    LOG_CAT_INFO("[APPLICATION]", "========== ЗАВЕРШЕНИЕ РАБОТЫ ПРИЛОЖЕНИЯ ==========");

    Logger::setShutdownMode(true);

    QMutexLocker locker(&m_shutdownMutex);
    QElapsedTimer shutdownTimer;
    shutdownTimer.start();

    // 1. Закрываем главное окно
    if (m_mainWindow) {
        LOG_CAT_INFO("[APPLICATION]", "Закрытие главного окна...");
        m_mainWindow->close();
        m_mainWindow.reset();
        LOG_CAT_INFO("[APPLICATION]", "Главное окно закрыто");
    }

    // 2. Останавливаем контроллер
    if (m_controller) {
        LOG_CAT_INFO("[APPLICATION]", "Остановка контроллера...");
        m_controller.reset();
        LOG_CAT_INFO("[APPLICATION]", "Контроллер остановлен");
    }

    // 3. Останавливаем коммуникации
    if (m_communication) {
        LOG_CAT_INFO("[APPLICATION]", "Остановка PPBCommunication...");

        // Вызываем stop() без аргументов
        QMetaObject::invokeMethod(m_communication.get(), "stop", Qt::QueuedConnection);
        LOG_CAT_DEBUG("[APPLICATION]", "Команда stop поставлена в очередь");

        // ВМЕСТО msleep используем небольшой таймер для асинхронного ожидания
        // Это предотвращает блокировку потока
        QEventLoop waitLoop;
        QTimer::singleShot(50, &waitLoop, &QEventLoop::quit);
        waitLoop.exec();

        LOG_CAT_DEBUG("[APPLICATION]", "Ожидание завершения PPBCommunication");

        m_communication.reset();
        LOG_CAT_INFO("[APPLICATION]", "PPBCommunication остановлен");
    }

    // 4. Останавливаем UDPClient (автоматически при уничтожении)
    if (m_udpClient) {
        LOG_CAT_INFO("[APPLICATION]", "Остановка UDPClient...");
        m_udpClient.reset();
        LOG_CAT_INFO("[APPLICATION]", "UDPClient остановлен");
    }

    // 5. Останавливаем коммуникационный поток
    if (m_communicationThread && m_communicationThread->isRunning()) {
        LOG_CAT_INFO("[APPLICATION]", "Остановка коммуникационного потока...");

        m_communicationThread->quit();
        LOG_CAT_DEBUG("[APPLICATION]", "Команда quit отправлена в поток");

        if (!m_communicationThread->wait(THREAD_SHUTDOWN_TIMEOUT_MS)) {
            LOG_CAT_WARNING("[APPLICATION]",
                            QString("Принудительное завершение коммуникационного потока (таймаут %1 мс)...")
                                .arg(THREAD_SHUTDOWN_TIMEOUT_MS));
            m_communicationThread->terminate();

            if (!m_communicationThread->wait(100)) {
                LOG_CAT_ERROR("[APPLICATION]", "Не удалось завершить поток даже после terminate!");
            }
        }

        delete m_communicationThread;
        m_communicationThread = nullptr;
        LOG_CAT_INFO("[APPLICATION]", "Коммуникационный поток остановлен");
    } else if (m_communicationThread) {
        LOG_CAT_DEBUG("[APPLICATION]", "Поток не запущен, только удаление");
        delete m_communicationThread;
        m_communicationThread = nullptr;
    }

    m_initialized = false;

    qint64 totalTime = shutdownTimer.elapsed();
    LOG_CAT_INFO("[APPLICATION]",
                 QString("========== ПРИЛОЖЕНИЕ ЗАВЕРШЕНО (общее время: %1 мс) ==========")
                     .arg(totalTime));
}

void ApplicationManager::cleanup()
{
    LOG_CAT_INFO("[APPLICATION]", "========== АВАРИЙНАЯ ОЧИСТКА РЕСУРСОВ ==========");

    // Аналогично shutdown, но без блокировки и с обработкой ошибок
    try {
        if (m_mainWindow) {
            LOG_CAT_INFO("[APPLICATION]", "Аварийное закрытие главного окна...");
            m_mainWindow->close();
            m_mainWindow.reset();
            LOG_CAT_INFO("[APPLICATION]", "Главное окно закрыто");
        }

        if (m_controller) {
            LOG_CAT_INFO("[APPLICATION]", "Аварийная остановка контроллера...");
            m_controller.reset();
            LOG_CAT_INFO("[APPLICATION]", "Контроллер остановлен");
        }

        if (m_communication) {
            LOG_CAT_INFO("[APPLICATION]", "Аварийная остановка PPBCommunication...");
            m_communication.reset();
            LOG_CAT_INFO("[APPLICATION]", "PPBCommunication остановлен");
        }

        if (m_udpClient) {
            LOG_CAT_INFO("[APPLICATION]", "Аварийная остановка UDPClient...");
            m_udpClient.reset();
            LOG_CAT_INFO("[APPLICATION]", "UDPClient остановлен");
        }

        if (m_communicationThread && m_communicationThread->isRunning()) {
            LOG_CAT_INFO("[APPLICATION]", "Аварийная остановка коммуникационного потока...");

            m_communicationThread->quit();
            if (!m_communicationThread->wait(500)) {
                LOG_CAT_WARNING("[APPLICATION]", "Принудительное завершение потока...");
                m_communicationThread->terminate();
                m_communicationThread->wait();
            }
            delete m_communicationThread;
            m_communicationThread = nullptr;
            LOG_CAT_INFO("[APPLICATION]", "Коммуникационный поток остановлен");
        } else if (m_communicationThread) {
            LOG_CAT_DEBUG("[APPLICATION]", "Удаление неактивного потока");
            delete m_communicationThread;
            m_communicationThread = nullptr;
        }

        m_initialized = false;
        LOG_CAT_INFO("[APPLICATION]", "========== АВАРИЙНАЯ ОЧИСТКА ЗАВЕРШЕНА ==========");

    } catch (const std::exception& e) {
        LOG_CAT_ERROR("[APPLICATION]",
                      QString("Ошибка при очистке ресурсов: %1").arg(e.what()));
    } catch (...) {
        LOG_CAT_ERROR("[APPLICATION]", "Неизвестная ошибка при очистке ресурсов");
    }
}
