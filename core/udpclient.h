#ifndef UDPCLIENT_H
#define UDPCLIENT_H
#include <QMutex>
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

    Q_INVOKABLE void initializeInThread();

    // Основные методы
    bool bind(quint16 port = 101); // Фиксированный порт 101
    void unbind();
    bool isBound() const;

    // Отправка данных
   Q_INVOKABLE  qint64 sendTo(const QByteArray& data, const QString& address, quint16 port);
   Q_INVOKABLE  qint64 sendBroadcast(const QByteArray& data, quint16 port);

    // Информация
    quint16 boundPort() const;
    QHostAddress boundAddress() const;

signals:
    void dataReceived(const QByteArray& data, const QHostAddress& sender, quint16 port);
    void bindingChanged(bool bound);
    void errorOccurred(const QString& error);
    void socketStateChanged(QAbstractSocket::SocketState state);
    void dataSent(qint64 bytes);
    void initialized();

private slots:
    void readPendingDatagrams();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketStateChanged(QAbstractSocket::SocketState state);

private:
    void setupSocket();

private:
    QUdpSocket* m_socket;
    bool m_isBound;
    quint16 m_boundPort;
    QHostAddress m_boundAddress;
};

#endif // UDPCLIENT_H
