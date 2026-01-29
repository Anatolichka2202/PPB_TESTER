#include "fileloader.h"

FileLoader::FileLoader(QObject *parent) : QObject(parent) {}

bool FileLoader::loadAndParseFile()
{
    // Шаг 1: Поиск файла
    m_foundFileName = findProgSoftFile();

    if (m_foundFileName.isEmpty()) {
        qWarning() << "Файл ProgSoft не найден";
        return false;
    }

    qDebug() << "Найден файл:" << m_foundFileName;

    // Шаг 2: Чтение файла
    QFile file(m_foundFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл:" << file.errorString();
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    qDebug() << "Размер файла:" << fileData.size() << "байт";

    // Шаг 3: Разбивка на пакеты
    m_packets = splitIntoPackets(fileData);

    qDebug() << "Создано пакетов:" << m_packets.size();

    // Вывод информации о пакетах (для отладки)
    for (int i = 0; i < m_packets.size(); ++i) {
        const QByteArray &packet = m_packets[i];
        qDebug() << "Пакет" << i << "(" << packet.size() << "байт):"
                 << packet.toHex(' ').toUpper();
    }

    return true;
}

QString FileLoader::findProgSoftFile()
{
    // Поиск в текущей директории
    QDir currentDir(".");

    // Ищем файлы, начинающиеся с "ProgSoft"
    QStringList filters;
    filters << "ProgSoft*";  // Ищет любые файлы, начинающиеся с ProgSoft

    QStringList files = currentDir.entryList(filters, QDir::Files);

    if (!files.isEmpty()) {
        return currentDir.absoluteFilePath(files.first());
    }

    // Альтернативный вариант: поиск точного имени
    files = currentDir.entryList(QStringList() << "ProgSoft", QDir::Files);

    if (!files.isEmpty()) {
        return currentDir.absoluteFilePath(files.first());
    }

    // Поиск в родительских директориях (рекурсивно)
    return searchRecursive(currentDir, "ProgSoft");
}

QString FileLoader::searchRecursive(const QDir &dir, const QString &fileName)
{
    // Проверяем текущую директорию
    QStringList entries = dir.entryList(QDir::Files);
    for (const QString &entry : entries) {
        if (entry.contains(fileName, Qt::CaseInsensitive)) {
            return dir.absoluteFilePath(entry);
        }
    }

    // Рекурсивный поиск в поддиректориях
    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &subDir : dirs) {
        QDir nextDir(dir.absoluteFilePath(subDir));
        QString found = searchRecursive(nextDir, fileName);
        if (!found.isEmpty()) {
            return found;
        }
    }

    return QString();
}

QVector<QByteArray> FileLoader::splitIntoPackets(const QByteArray &data)
{
    QVector<QByteArray> packets;

    if (data.isEmpty()) {
        return packets;
    }

    // Разбиваем на пакеты по 3 байта
    for (int i = 0; i < data.size(); i += 3) {
        packets.append(data.mid(i, 3));
    }

    // Если последний пакет меньше 3 байт, можно дополнить нулями
    // (раскомментировать при необходимости)

    if (!packets.isEmpty()) {
        QByteArray &lastPacket = packets.last();
        while (lastPacket.size() < 3) {
            lastPacket.append('\0');
        }
    }


    return packets;
}

QVector<QByteArray> FileLoader::getPackets() const
{
    return m_packets;
}

QString FileLoader::getFoundFileName() const
{
    return m_foundFileName;
}
