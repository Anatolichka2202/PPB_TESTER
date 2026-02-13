#include "logentry.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <QDebug>

QString LogEntry::toLegacyFormat() const
{
    QString result;

    // Формат: [уровень][модуль] сообщение
    QString levelShort;
    if (level == "DEBUG") levelShort = "DBG";
    else if (level == "INFO") levelShort = "INF";
    else if (level == "WARNING") levelShort = "WRN";
    else if (level == "ERROR") levelShort = "ERR";
    else if (level == "CRITICAL") levelShort = "CRT";
    else levelShort = level.left(3).toUpper();

    result += "[" + levelShort + "]";

    // Добавляем категорию, если она не GENERAL
    if (!category.isEmpty() && category != "GENERAL" && category != "LEGACY") {
        // Убираем префиксы для красоты
        QString displayCategory = category;
        if (displayCategory.startsWith("TECH_")) displayCategory = displayCategory.mid(5);
        else if (displayCategory.startsWith("UI_")) displayCategory = displayCategory.mid(3);
        else if (displayCategory.startsWith("OP_")) displayCategory = displayCategory.mid(3);

        result += "[" + displayCategory + "]";
    }

    result += " " + message;

    return result;
}

// ==================== РЕАЛИЗАЦИЯ МЕТОДОВ TableData ====================

QString TableData::toHtml() const {
    QString html;

    // Контейнер таблицы
    html += QString("<div class='log-table-container' id='table-%1'>").arg(id);

    // Заголовок
    if (!title.isEmpty()) {
        html += QString("<div class='table-title'>%1</div>").arg(title);
    }

    // Таблица
    html += "<table class='log-table'";
    if (sortable) html += " data-sortable='true'";
    if (compact) html += " data-compact='true'";
    html += ">";

    // Заголовки столбцов
    if (!headers.isEmpty()) {
        html += "<thead><tr>";
        for (int i = 0; i < headers.size(); ++i) {
            QString style;
            if (columnColors.contains(i)) {
                style = QString("style='background-color: %1;'").arg(columnColors[i].name());
            }
            html += QString("<th %1>%2</th>").arg(style, headers[i]);
        }
        html += "</tr></thead>";
    }

    // Тело таблицы
    html += "<tbody>";
    for (const QStringList& row : rows) {
        html += "<tr>";
        for (int i = 0; i < row.size(); ++i) {
            QString value = row[i];
            QString format = columnFormats.value(i, "");
            QString style;
            QString classes = QString("col-%1").arg(i);

            // Форматирование значения
            if (format == "hex" && !value.isEmpty()) {
                bool ok;
                int num = value.toInt(&ok);
                if (ok) {
                    value = QString("0x%1").arg(num, 0, 16).toUpper();
                    classes += " format-hex";
                }
            } else if (format == "float" && !value.isEmpty()) {
                bool ok;
                double num = value.toDouble(&ok);
                if (ok) {
                    value = QString::number(num, 'f', 3);
                    classes += " format-float";
                }
            } else if (format == "percent" && !value.isEmpty()) {
                bool ok;
                double num = value.toDouble(&ok);
                if (ok) {
                    value = QString("%1%").arg(num, 0, 'f', 1);
                    classes += " format-percent";
                }
            }

            // Цвет ячейки
            if (columnColors.contains(i)) {
                QColor color = columnColors[i];
                style = QString("style='background-color: %1;'").arg(color.name());
            }

            html += QString("<td %1 class='%2'>%3</td>")
                        .arg(style, classes, value);
        }
        html += "</tr>";
    }
    html += "</tbody></table></div>";

    return html;
}

QString TableData::toCsv() const {
    QString csv;
    QTextStream stream(&csv);

    // Заголовки
    if (!headers.isEmpty()) {
        for (int i = 0; i < headers.size(); ++i) {
            if (i > 0) stream << ";";
            stream << headers[i];
        }
        stream << "\n";
    }

    // Данные
    for (const QStringList& row : rows) {
        for (int i = 0; i < row.size(); ++i) {
            if (i > 0) stream << ";";
            QString value = row[i];

            // Экранирование для CSV
            if (value.contains(';') || value.contains('"') || value.contains('\n')) {
                value.replace("\"", "\"\"");
                value = "\"" + value + "\"";
            }

            stream << value;
        }
        stream << "\n";
    }

    return csv;
}

// ==================== РЕАЛИЗАЦИЯ МЕТОДОВ CardData ====================

QString CardData::toHtml() const {
    QString html;

    // Контейнер карточки
    QString style;
    if (backgroundColor.isValid()) {
        style = QString("style='background-color: %1; color: %2;'")
        .arg(backgroundColor.name(),
             textColor.isValid() ? textColor.name() : "#ffffff");
    }

    html += QString("<div class='log-card' id='card-%1' %2>").arg(id, style);

    // Верхняя часть (иконка и заголовок)
    html += "<div class='card-header'>";
    if (!icon.isEmpty()) {
        html += QString("<span class='card-icon'>%1</span>").arg(icon);
    }
    if (!title.isEmpty()) {
        html += QString("<span class='card-title'>%1</span>").arg(title);
    }
    if (closable) {
        html += "<span class='card-close' onclick='this.parentElement.parentElement.remove()'>×</span>";
    }
    html += "</div>";

    // Содержимое
    if (!fields.isEmpty()) {
        html += "<div class='card-content'>";
        for (auto it = fields.begin(); it != fields.end(); ++it) {
            html += QString("<div class='card-field'>"
                            "<span class='card-key'>%1:</span>"
                            "<span class='card-value'>%2</span>"
                            "</div>").arg(it.key(), it.value());
        }
        html += "</div>";
    }

    // Подвал
    if (!footer.isEmpty()) {
        html += QString("<div class='card-footer'>%1</div>").arg(footer);
    }

    html += "</div>";
    return html;
}

// ==================== РЕАЛИЗАЦИЯ МЕТОДОВ ProgressData ====================

QString ProgressData::toHtml() const {
    QString html;

    // Контейнер прогресса
    html += QString("<div class='log-progress' id='progress-%1'>").arg(id);

    // Заголовок
    if (!title.isEmpty()) {
        html += QString("<div class='progress-title'>%1</div>").arg(title);
    }

    // Прогресс-бар
    int percent = percentage();
    QString barStyle = QString("style='width: %1%; background-color: %2;'")
                           .arg(percent)
                           .arg(barColor.isValid() ? barColor.name() : "#4CAF50");

    html += "<div class='progress-container'>";
    html += QString("<div class='progress-bar' %1></div>").arg(barStyle);

    // Текст поверх прогресса
    if (showText) {
        QString text;
        if (!units.isEmpty()) {
            text = QString("%1/%2 %3").arg(current).arg(total).arg(units);
        } else {
            text = QString("%1/%2").arg(current).arg(total);
        }
        html += QString("<div class='progress-text'>%1</div>").arg(text);
    }
    html += "</div>";

    // Процент отдельно
    if (showPercentage) {
        html += QString("<div class='progress-percent'>%1%</div>").arg(percent);
    }

    html += "</div>";
    return html;
}

// ==================== РЕАЛИЗАЦИЯ МЕТОДОВ KeyValueData ====================

QString KeyValueData::toHtml() const {
    QString html;

    // Контейнер ключ-значение
    html += QString("<div class='log-keyvalue' id='kv-%1'>").arg(id);

    // Заголовок
    if (!title.isEmpty()) {
        html += QString("<div class='kv-title'>%1</div>").arg(title);
    }

    // Данные
    html += "<div class='kv-container";
    if (horizontal) html += " kv-horizontal";
    html += "'>";

    for (auto it = data.begin(); it != data.end(); ++it) {
        html += QString("<div class='kv-item'>"
                        "<span class='kv-key'>%1:</span>"
                        "<span class='kv-value'>%2</span>"
                        "</div>").arg(it.key(), it.value());
    }

    html += "</div></div>";
    return html;
}

// ==================== РЕАЛИЗАЦИЯ МЕТОДОВ RawData ====================

QString RawData::toHexDump() const {
    QString result;
    QTextStream stream(&result);

    for (int i = 0; i < data.size(); i += bytesPerLine) {
        // Смещение
        if (showOffset) {
            stream << QString("%1: ").arg(i, 8, 16, QChar('0')).toUpper();
        }

        // HEX
        for (int j = 0; j < bytesPerLine; ++j) {
            if (i + j < data.size()) {
                stream << QString("%1 ").arg((quint8)data[i + j], 2, 16, QChar('0')).toUpper();
            } else {
                stream << "   ";
            }

            if (j == bytesPerLine / 2 - 1) {
                stream << " ";
            }
        }

        // ASCII
        if (showAscii) {
            stream << " |";
            for (int j = 0; j < bytesPerLine; ++j) {
                if (i + j < data.size()) {
                    char c = data[i + j];
                    if (c >= 32 && c <= 126) {
                        stream << c;
                    } else {
                        stream << ".";
                    }
                } else {
                    stream << " ";
                }
            }
            stream << "|";
        }

        stream << "\n";
    }

    return result;
}

QString RawData::toHtml() const {
    QString html;

    html += QString("<div class='log-raw' id='raw-%1'>").arg(id);

    if (!title.isEmpty()) {
        html += QString("<div class='raw-title'>%1</div>").arg(title);
    }

    html += "<pre class='raw-data'>";
    html += toHexDump().toHtmlEscaped();
    html += "</pre></div>";

    return html;
}

// ==================== РЕАЛИЗАЦИЯ МЕТОДОВ LogEntry ====================



QString LogEntry::toString() const
{
    QString result;

    // Время
    result += timestamp.toString("hh:mm:ss.zzz") + " ";

    // Уровень с иконкой
    QString levelIcon;
    if (level == "ERROR") levelIcon = "🔴";
    else if (level == "WARNING") levelIcon = "🟡";
    else if (level == "INFO") levelIcon = "ℹ️";
    else if (level == "DEBUG") levelIcon = "🐛";

    result += levelIcon + "[" + level + "] ";

    // Категория с эмодзи
    QString categoryEmoji;
    if (category.startsWith("UI_")) categoryEmoji = "👨‍💼";
    else if (category.startsWith("TECH_")) categoryEmoji = "👨‍💻";
    else if (category.startsWith("OP_")) categoryEmoji = "📊";
    else if (category == "SYSTEM") categoryEmoji = "⚙️";
    else if (category == "GENERAL") categoryEmoji = "📝";

    result += categoryEmoji + "[" + category + "] ";

    // Сообщение
    result += message;

    return result;
}

QString LogEntry::toHtml() const {
    QString html;

    // Определяем CSS классы
    QString levelClass = "log-level-" + level.toLower();
    QString categoryClass = "log-category-" + category.toLower().replace("_", "-");

    // Основной контейнер
    html += QString("<div class='log-entry %1 %2' data-type='%3'>")
                .arg(levelClass, categoryClass, QString::number((int)dataType));

    // Время
    html += QString("<span class='log-time'>%1</span> ")
                .arg(timestamp.toString("hh:mm:ss.zzz"));

    // Уровень с иконкой
    QString levelIcon;
    if (level == "ERROR") levelIcon = "🔴";
    else if (level == "WARNING") levelIcon = "🟡";
    else if (level == "INFO") levelIcon = "ℹ️";
    else if (level == "DEBUG") levelIcon = "🐛";

    html += QString("<span class='log-level'>%1[%2]</span> ")
                .arg(levelIcon, level);

    // Категория
    html += QString("<span class='log-category'>[%1]</span> ").arg(category);

    // Тело сообщения
    html += "<div class='log-body'>";

    // В зависимости от типа данных
    switch (dataType) {
    case DataType::Text:
        html += QString("<div class='log-message'>%1</div>").arg(message.toHtmlEscaped());
        break;

    case DataType::Table:
        if (structuredData.canConvert<TableData>()) {
            html += getTable().toHtml();
        }
        break;

    case DataType::Card:
        if (structuredData.canConvert<CardData>()) {
            html += getCard().toHtml();
        }
        break;

    case DataType::Progress:
        if (structuredData.canConvert<ProgressData>()) {
            html += getProgress().toHtml();
        }
        break;

    case DataType::KeyValue:
        if (structuredData.canConvert<KeyValueData>()) {
            html += getKeyValue().toHtml();
        }
        break;

    case DataType::RawData:
        if (structuredData.canConvert<RawData>()) {
            html += getRawData().toHtml();
        }
        break;

    default:
        html += QString("<div class='log-message'>%1</div>").arg(message.toHtmlEscaped());
        break;
    }

    // Метаданные (только в режиме отладки)
    if (sourceLine > 0 || threadId > 0) {
        html += "<div class='log-meta'>";
        if (sourceLine > 0) {
            html += QString("<span class='log-source'>%1:%2</span> ")
            .arg(sourceFile).arg(sourceLine);
        }
        if (threadId > 0) {
            html += QString("<span class='log-thread'>[T:%1]</span> ").arg(threadId);
        }
        if (elapsedMs > 0) {
            html += QString("<span class='log-elapsed'>%1ms</span>").arg(elapsedMs);
        }
        html += "</div>";
    }

    html += "</div>"; // .log-body
    html += "</div>"; // .log-entry

    return html;
}

QString LogEntry::toJson() const {
    QJsonObject json;

    json["timestamp"] = timestamp.toString(Qt::ISODateWithMs);
    json["level"] = level;
    json["category"] = category;
    json["message"] = message;
    json["dataType"] = static_cast<int>(dataType);
    json["structuredDataId"] = structuredDataId;

    if (sourceLine > 0) {
        json["sourceFile"] = sourceFile;
        json["sourceLine"] = sourceLine;
    }

    if (threadId > 0) {
        json["threadId"] = QString::number(threadId);
    }

    if (elapsedMs > 0) {
        json["elapsedMs"] = elapsedMs;
    }

    // TODO: Добавить сериализацию структурированных данных

    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}
QString LogEntry::toLegacyString() const
{
    QString result;

    // Формат: [уровень][модуль] сообщение
    result += "[" + level + "]";

    if (!category.isEmpty()) {
        result += "[" + category + "]";
    }

    result += " " + message;

    return result;
}
