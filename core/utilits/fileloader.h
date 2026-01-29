#ifndef FILELOADER_H
#define FILELOADER_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QDebug>

class FileLoader : public QObject
{
    Q_OBJECT
public:
    explicit FileLoader(QObject *parent = nullptr);

    // Основная функция для поиска и обработки файла
    bool loadAndParseFile();

    // Получить результат разбивки на пакеты
    QVector<QByteArray> getPackets() const;

    // Получить имя найденного файла
    QString getFoundFileName() const;

private:
    // Поиск файла с именем ProgSoft
    QString findProgSoftFile();
    QString searchRecursive(const QDir &dir, const QString &fileName);
    // Разбивка данных на пакеты по 3 байта
    QVector<QByteArray> splitIntoPackets(const QByteArray &data);

    QVector<QByteArray> m_packets;
    QString m_foundFileName;
};
#endif // FILELOADER_H
