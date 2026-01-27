#include "analysisfactory.h"
#include "shiftdetector.h"
#include "guerrillaanalyzer.h"
#include <QVector>

QString AnalysisFactory::autoAnalyze(const QVector<DataPacket>& sent,
                                     const QVector<DataPacket>& received)
{
    QString report;

    // Если нет данных
    if (sent.isEmpty() || received.isEmpty()) {
        report = "Нет данных для анализа";
        return report;
    }

    // Часть 1: Детектор сдвига (только если размеры совпадают)
    if (sent.size() == received.size()) {
        ShiftDetector shiftDetector;
        auto shiftResult = shiftDetector.analyze(sent, received);
        report += "1. Анализ сдвига:\n";
        report += "   " + shiftResult.analysis + "\n\n";
    } else {
        report += "1. Анализ сдвига: не применим (разные размеры)\n\n";
    }

    // Часть 2: Партизанский анализ (всегда)
    report += "2. Детальный анализ потерь и соответствий:\n";
    GuerrillaAnalyzer guerrillaAnalyzer;
    auto guerrillaResult = guerrillaAnalyzer.analyze(sent, received);
    report += guerrillaAnalyzer.generateReport(guerrillaResult, sent, received);

    return report;
}

QString AnalysisFactory::analyzeWithShiftDetector(const QVector<DataPacket>& sent,
                                                  const QVector<DataPacket>& received)
{
    ShiftDetector detector;
    auto result = detector.analyze(sent, received);
    return result.analysis;
}

QString AnalysisFactory::analyzeWithGuerrilla(const QVector<DataPacket>& sent,
                                              const QVector<DataPacket>& received)
{
    GuerrillaAnalyzer analyzer;
    auto result = analyzer.analyze(sent, received);
    return analyzer.generateReport(result, sent, received);
}

QString AnalysisFactory::analyzeCombined(const QVector<DataPacket>& sent,
                                         const QVector<DataPacket>& received)
{
    QString report = "=== КОМБИНИРОВАННЫЙ АНАЛИЗ ===\n\n";

    // Анализ сдвига
    ShiftDetector shiftDetector;
    auto shiftResult = shiftDetector.analyze(sent, received);
    report += "1. Анализ сдвига:\n";
    report += "   " + shiftResult.analysis + "\n\n";

    // Партизанский анализ
    GuerrillaAnalyzer guerrillaAnalyzer;
    auto guerrillaResult = guerrillaAnalyzer.analyze(sent, received);
    report += "2. Детальный анализ:\n";
    report += guerrillaAnalyzer.generateReport(guerrillaResult, sent, received);

    return report;
}
