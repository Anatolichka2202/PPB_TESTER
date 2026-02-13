#ifndef AKIP_SCPI_H
#define AKIP_SCPI_H

#include <QString>

namespace AkipSCPI {

// ---------- Каналы ----------
enum class Channel {
    A = 1,
    B = 2
};

inline QString channelSuffix(Channel ch) {
    return (ch == Channel::A) ? "A" : "B";
}

// ---------- Базовые команды IEEE-488.2 ----------
inline const QString IDN_QUERY = "*IDN?";
inline const QString RST = "*RST";

// ---------- Управление выходом ----------
inline QString output(Channel ch, bool on) {
    return QString("OUTP:CH%1 %2").arg(channelSuffix(ch)).arg(on ? "ON" : "OFF");
}
inline QString outputQuery(Channel ch) {
    return QString("OUTP:CH%1?").arg(channelSuffix(ch));
}

// ---------- Выбор формы сигнала ----------
// Допустимые значения: SINusoid, SQUare, RAMP, PULSe, EXP, SINC, NOISe, DC
inline QString function(Channel ch, const QString& waveform) {
    return QString("FUNC:CH%1 %2").arg(channelSuffix(ch)).arg(waveform.toUpper());
}
inline QString functionQuery(Channel ch) {
    return QString("FUNC:CH%1?").arg(channelSuffix(ch));
}

// ---------- Установка частоты ----------
// Частота задаётся в Гц (можно с суффиксом kHz, MHz)
inline QString frequency(Channel ch, double freqHz) {
    return QString("FREQ:CH%1 %2").arg(channelSuffix(ch)).arg(freqHz, 0, 'f', 0); // без десятичных
}
// Удобный вариант с указанием единицы (например, "MHz")
inline QString frequency(Channel ch, double value, const QString& unit) {
    return QString("FREQ:CH%1 %2 %3").arg(channelSuffix(ch)).arg(value, 0, 'f', 6).arg(unit);
}
inline QString frequencyQuery(Channel ch) {
    return QString("FREQ:CH%1?").arg(channelSuffix(ch));
}

// ---------- Установка амплитуды/мощности ----------
// Единицы: Vpp, Vrms, dBm (для CHA допустимо dBm, для CHB – Vpp, Vrms)
inline QString voltage(Channel ch, double value, const QString& unit) {
    return QString("VOLT:CH%1 %2 %3").arg(channelSuffix(ch)).arg(value, 0, 'f', 3).arg(unit);
}
inline QString voltageQuery(Channel ch) {
    return QString("VOLT:CH%1?").arg(channelSuffix(ch));
}

// ---------- Специально для канала B: скважность прямоугольного сигнала ----------
inline QString dutyCycleB(double percent) {
    return QString("FUNC:CHB:SQU:DCYC %1").arg(percent, 0, 'f', 3);
}
inline QString dutyCycleBQuery() {
    return "FUNC:CHB:SQU:DCYC?";
}

// ---------- AM модуляция (только для канала A) ----------
// Источник модуляции: INTernal / EXTernal
inline QString amSource(const QString& source = "INT") {
    return QString("AM:SOUR %1").arg(source.toUpper());
}
inline QString amSourceQuery() {
    return "AM:SOUR?";
}

// Частота внутреннего модулирующего сигнала (Гц)
inline QString amInternalFrequency(double freqHz) {
    return QString("AM:INT:FREQ %1").arg(freqHz, 0, 'f', 3);
}
inline QString amInternalFrequencyQuery() {
    return "AM:INT:FREQ?";
}

// Глубина модуляции (%)
inline QString amDepth(double percent) {
    return QString("AM:DEPT %1").arg(percent, 0, 'f', 3);
}
inline QString amDepthQuery() {
    return "AM:DEPT?";
}

// Форма модулирующего сигнала (SIN, SQU, RAMP)
inline QString amInternalFunction(const QString& func = "SIN") {
    return QString("AM:INT:FUNC %1").arg(func.toUpper());
}
inline QString amInternalFunctionQuery() {
    return "AM:INT:FUNC?";
}

} // namespace AkipSCPI

#endif // AKIP_SCPI_H
