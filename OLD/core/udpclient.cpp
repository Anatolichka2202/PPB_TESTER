#include "udpclient.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QAbstractSocket>
#include <QVariant>
#include "logger.h"
UDPClient::UDPClient(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_isBound(false)
    , m_boundPort(0)
{
    // Создаем сокет
    m_socket = new QUdpSocket(this);

    if (m_socket->bind(QHostAddress::AnyIPv4, 101)) {
        m_isBound = true;
        m_boundPort = m_socket->localPort();
        m_boundAddress = m_socket->localAddress();

        Logger::info(QString("UDPClient: Создан и привязан к порту %1").arg(m_boundPort));
    } else {
        Logger::error(QString("UDPClient: Ошибка привязки сокета: %1").arg(m_socket->errorString()));
    }

    // Подключаем сигналы
    connect(m_socket, &QUdpSocket::readyRead, this, &UDPClient::readPendingDatagrams);

    // Правильное подключение сигнала errorOccurred в Qt6
    connect(m_socket, &QUdpSocket::errorOccurred, this, &UDPClient::onSocketError);

    connect(m_socket, &QUdpSocket::stateChanged,
            this, &UDPClient::onSocketStateChanged);
}

UDPClient::~UDPClient()
{
    if (m_socket) {
        m_socket->close();
    }
}

bool UDPClient::bind(quint16 port, QHostAddress::SpecialAddress address)
{
    if (m_isBound) {
        qDebug() << "UDPClient: Socket already bound, skipping.";
        return true;
    }

    QHostAddress bindAddress(address);
    if (m_socket->bind(bindAddress, port)) {
        m_isBound = true;
        m_boundPort = m_socket->localPort();
        m_boundAddress = m_socket->localAddress();

        qDebug() << "UDPClient: Bound to" << m_boundAddress.toString() << "port" << m_boundPort;
        emit bindingChanged(true);
        return true;
    }

    emit errorOccurred(QString("Failed to bind to port %1: %2")
                           .arg(port).arg(m_socket->errorString()));
    return false;
}



bool UDPClient::isBound() const
{
    return m_isBound;
}

qint64 UDPClient::sendTo(const QByteArray& data, const QString& address, quint16 port)
{
    if (!m_isBound) {
        Logger::error("UDPClient: Сокет не привязан, отправка невозможна");
        emit errorOccurred("Socket is not bound");
        return -1;
    }

    QHostAddress hostAddress;
    if (!hostAddress.setAddress(address)) {
        Logger::error(QString("UDPClient: Неверный адрес: %1").arg(address));
        emit errorOccurred(QString("Invalid address: %1").arg(address));
        return -1;
    }

    qint64 bytesSent = m_socket->writeDatagram(data, hostAddress, port);

    if (bytesSent == -1) {
        Logger::error(QString("UDPClient: Ошибка отправки данных: %1").arg(m_socket->errorString()));
        emit errorOccurred(QString("Failed to send data: %1").arg(m_socket->errorString()));
    } else {
        Logger::debug(QString("UDPClient: Отправлено %1 байт на %2:%3").arg(bytesSent).arg(address).arg(port));

        // Логируем содержимое пакета
        QString hexData;
        for (int i = 0; i < qMin(data.size(), 16); ++i) {
            hexData += QString("%1 ").arg((uint8_t)data[i], 2, 16, QChar('0')).toUpper();
        }
        if (data.size() > 16) hexData += "...";

        Logger::debug(QString("UDPClient: Данные пакета [%1 байт]: %2").arg(data.size()).arg(hexData));
        emit dataSent(bytesSent);
    }

    return bytesSent;
}

qint64 UDPClient::sendBroadcast(const QByteArray& data, quint16 port)
{
    if (!m_socket) {
        emit errorOccurred("Socket is not initialized");
        return -1;
    }


    // Сохраняем текущее состояние широковещания
    bool wasBroadcastEnabled = m_socket->socketOption(static_cast<QAbstractSocket::SocketOption>(4)).toBool();

    // Включаем широковещание
    m_socket->setSocketOption(static_cast<QAbstractSocket::SocketOption>(4), QVariant(1));

    setBroadcastEnabled(true);
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

void UDPClient::setBroadcastEnabled(bool enabled)
{
    // Используем QUdpSocket вместо QAbstractSocket
    m_socket->setSocketOption(static_cast<QAbstractSocket::SocketOption>(4),
                              QVariant(enabled ? 1 : 0));
}

void UDPClient::setReceiveBufferSize(int size)
{
    // Используем QUdpSocket вместо QAbstractSocket
    m_socket->setSocketOption(QUdpSocket::ReceiveBufferSizeSocketOption, size);
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
