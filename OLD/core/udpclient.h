#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UDPClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isBound READ isBound NOTIFY bindingChanged)

public:
    explicit UDPClient(QObject* parent = nullptr);
    ~UDPClient() override;

    // Основные методы
    bool bind(quint16 port = 0, QHostAddress::SpecialAddress address = QHostAddress::Any);
    void unbind();
    bool isBound() const;

    // Отправка данных
    qint64 sendTo(const QByteArray& data, const QString& address, quint16 port);
    qint64 sendBroadcast(const QByteArray& data, quint16 port);

    // Настройки
    void setBroadcastEnabled(bool enabled);
    void setReceiveBufferSize(int size);

    // Информация о подключении
    quint16 boundPort() const;
    QHostAddress boundAddress() const;

signals:
    // Сигналы о данных
    void dataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);

    // Сигналы о состоянии
    void bindingChanged(bool bound);
    void errorOccurred(const QString& error);
    void socketStateChanged(QAbstractSocket::SocketState state);

    // Сигналы отправки
    void dataSent(qint64 bytes);

private slots:
    void readPendingDatagrams();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketStateChanged(QAbstractSocket::SocketState state);

private:
    QUdpSocket* m_socket;
    bool m_isBound;
    quint16 m_boundPort;
    QHostAddress m_boundAddress;
};

#endif // UDPCLIENT_H
