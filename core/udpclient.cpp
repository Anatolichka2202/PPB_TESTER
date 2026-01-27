#include "udpclient.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QAbstractSocket>
#include <QVariant>
#include "logger.h"
#include <QThread>
#include <QEventLoop>
#include <QTimer>
/*
UDPClient* UDPClient::m_instance = nullptr;



UDPClient* UDPClient::instance()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if(!m_instance)
    {
        m_instance = new UDPClient();  // Конструктор не создает сокет

        // Создаем отдельный поток для UDPClient
        QThread* thread = new QThread();
        m_instance->moveToThread(thread);

        bool initialized = false;
        QMetaObject::Connection conn = QObject::connect(
            m_instance, &UDPClient::initialized,[&](){initialized=true;});

        QMetaObject::invokeMethod(m_instance, "initializeInThread", Qt::QueuedConnection);

        // Подключаем завершение потока к удалению
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        thread->start();

        QEventLoop loop;
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        if(!initialized)
        {
            QObject::connect(m_instance, &UDPClient::initialized, &loop, &QEventLoop::quit);
            loop.exec();
        }
        QObject::disconnect(conn);
        qDebug() << "UDPClient создан в потоке:" << thread;
    }
    return m_instance;
}
*/
void UDPClient::initializeInThread()
{
    // Этот метод выполняется уже в отдельном потоке
    setupSocket();
    LOG_INFO("UDPClient инициализирован в потоке: " +
             QString::number((qulonglong)QThread::currentThreadId()));
        emit initialized();
}
void UDPClient::setupSocket()
{
    // Убедимся, что мы в правильном потоке
    Q_ASSERT(QThread::currentThread() == this->thread());

    if (m_socket) {
        return;
    }

    m_socket = new QUdpSocket(this);  // СОЗДАЕМ
    // Включаем опции
    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 65536);
    m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 65536);

    // Привязываемся к порту 101
    if (m_socket->bind(QHostAddress::Any, 101, QUdpSocket::ReuseAddressHint)) {
        m_isBound = true;
        m_boundPort = m_socket->localPort();
        m_boundAddress = m_socket->localAddress();
        LOG_INFO(QString("UDPClient привязан к порту %1").arg(m_boundPort));
        emit bindingChanged(true);
    } else {
        LOG_ERROR(QString("Не удалось привязать UDP сокет: %1").arg(m_socket->errorString()));
        return; // Не испускаем initialized в случае ошибки
    }

    connect(m_socket, &QUdpSocket::readyRead, this, &UDPClient::readPendingDatagrams);
    connect(m_socket, &QUdpSocket::errorOccurred, this, &UDPClient::onSocketError);
    connect(m_socket, &QUdpSocket::stateChanged, this, &UDPClient::onSocketStateChanged);
}


UDPClient::UDPClient(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_isBound(false)
    , m_boundPort(0)
{
}

UDPClient::~UDPClient()
{
    if (m_socket) {
        m_socket->close();
        delete m_socket;
    }
}

bool UDPClient::bind(quint16 port)
{
    if (!m_socket) {
        return false;
    }

    if (m_socket->state() == QAbstractSocket::BoundState) {
        return true;
        emit bindingChanged(true);
    }

    if (m_socket->bind(QHostAddress::Any, port, QUdpSocket::ReuseAddressHint)) {
        m_isBound = true;
        m_boundPort = m_socket->localPort();
        m_boundAddress = m_socket->localAddress();
        emit bindingChanged(true);
        return true;
    }

    LOG_ERROR(QString("Ошибка привязки к порту %1: %2").arg(port).arg(m_socket->errorString()));
    return false;
}


bool UDPClient::isBound() const
{
 return m_isBound && m_socket != nullptr;
}

qint64 UDPClient::sendTo(const QByteArray& data, const QString& address, quint16 port)
{
    if (!m_socket) {
        LOG_ERROR("UDPClient: Сокет не привязан, отправка невозможна");
        emit errorOccurred("Socket is not bound");
        return -1;
    }

    QHostAddress hostAddress;
    if (!hostAddress.setAddress(address)) {
        LOG_ERROR(QString("UDPClient: Неверный адрес: %1").arg(address));
        emit errorOccurred(QString("Invalid address: %1").arg(address));
        return -1;
    }

    qint64 bytesSent = m_socket->writeDatagram(data, hostAddress, port);

    if (bytesSent == -1) {
       LOG_ERROR(QString("UDPClient: Ошибка отправки данных: %1").arg(m_socket->errorString()));
        emit errorOccurred(QString("Failed to send data: %1").arg(m_socket->errorString()));
    } else {
        LOG_DEBUG(QString("UDPClient: Отправлено %1 байт на %2:%3").arg(bytesSent).arg(address).arg(port));

        // Логируем содержимое пакета
        QString hexData;
        for (int i = 0; i < qMin(data.size(), 16); ++i) {
            hexData += QString("%1 ").arg((uint8_t)data[i], 2, 16, QChar('0')).toUpper();
        }
        if (data.size() > 16) hexData += "...";

        LOG_DEBUG(QString("UDPClient: Данные пакета [%1 байт]: %2").arg(data.size()).arg(hexData));
        emit dataSent(bytesSent);
    }

    return bytesSent;
}

qint64 UDPClient::sendBroadcast(const QByteArray& data, quint16 port)
{
    LOG_INFO(QString("UDPClient::sendBroadcast: порт=%1, размер=%2 байт, поток=%3")
                 .arg(port).arg(data.size()).arg((qulonglong)QThread::currentThreadId()));

    if (!m_socket) {
        LOG_ERROR("Сокет не инициализирован");
        emit errorOccurred("Socket is not initialized");
        return -1;
    }


    // Сохраняем текущее состояние широковещания
    bool wasBroadcastEnabled = m_socket->socketOption(static_cast<QAbstractSocket::SocketOption>(4)).toBool();

    // Включаем широковещание
    m_socket->setSocketOption(static_cast<QAbstractSocket::SocketOption>(4), QVariant(1));


    // Отправляем широковещательный пакет
    qint64 bytesSent = m_socket->writeDatagram(data, QHostAddress::Broadcast, port);

    qDebug() << "Broadcast sent to port:" << port << "bytes:" << bytesSent;
    qDebug() << "Socket error:" << m_socket->errorString();

    // Восстанавливаем предыдущее состояние
    if (!wasBroadcastEnabled) {
        m_socket->setSocketOption(static_cast<QAbstractSocket::SocketOption>(4), QVariant(0));
    }

    if (bytesSent == -1) {
        emit errorOccurred(QString("Failed to send broadcast: %1").arg(m_socket->errorString()));
    } else {
        qDebug() << "UDPClient: Sent broadcast" << bytesSent << "bytes to port" << port;
        emit dataSent(bytesSent);
    }

    return bytesSent;
}



quint16 UDPClient::boundPort() const
{
    return m_boundPort;
}

QHostAddress UDPClient::boundAddress() const
{
    return m_boundAddress;
}

void UDPClient::readPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        if (datagram.isValid()) {
            // Логируем факт получения
            Logger::debug(QString("UDP <- %1:%2 [%3 bytes]")
                              .arg(datagram.senderAddress().toString())
                              .arg(datagram.senderPort())
                              .arg(datagram.data().size()));

            emit dataReceived(datagram.data(),
                              datagram.senderAddress(),
                              datagram.senderPort());
        }
    }
}

void UDPClient::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = QString("Socket error: %1 - %2")
    .arg(error)
        .arg(m_socket->errorString());

    emit errorOccurred(errorString);
    qDebug() << "UDPClient:" << errorString;
}

void UDPClient::onSocketStateChanged(QAbstractSocket::SocketState state)
{
    emit socketStateChanged(state);

    // Если сокет перешел в состояние UnconnectedState и мы были привязаны
    if (state == QAbstractSocket::UnconnectedState && m_isBound) {
        m_isBound = false;
        m_boundPort = 0;
        m_boundAddress.clear();
        emit bindingChanged(false);
    }
}
