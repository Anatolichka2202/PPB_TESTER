#ifndef TESTERWINDOW_H
#define TESTERWINDOW_H

#include <QMainWindow>
#include "ppbcontroller.h"
#include "pult.h"
#include <QGroupBox>
#include "../core/logentry.h"
#include <QTextBrowser>
#include "../core/logging/loguimanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TesterWindow; }
QT_END_NAMESPACE

class TesterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TesterWindow(PPBController* controller = nullptr, QWidget *parent = nullptr);
    ~TesterWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // UI события
    void onConnectClicked();
    void onPollStatusClicked();
    void onResetClicked();
    void onApplyParametersClicked();
    void onAutoPollToggled(bool checked);
    void onTestSequenceClicked();
    void onFUTransmitClicked();
    void onFUReceiveClicked();
    void onDisplayModeChanged(bool showCodes);
    void onPPBSelected(int index);
    void onClearLogClicked();
    void onExportLogClicked();
    void onExitClicked();
    void onPultActiveClicked();

    // PRBS тесты
    void onPRBSM2SClicked();
    void onPRBSS2MClicked();

    // Сигналы от контроллера
    void onControllerConnectionStateChanged(PPBState state);
    void onControllerStatusReceived(uint16_t address, const QVector<QByteArray>& data);
    void onControllerErrorOccurred(const QString& error);
   // void onControllerLogMessage(const QString& message);
    void onControllerChannelStateUpdated(uint8_t ppbIndex, int channel, const UIChannelState& state);
    void onAutoPollToggledFromController(bool enabled);

    // Прогресс операций
    void onOperationProgress(int current, int total, const QString& operation);
    void onOperationCompleted(bool success, const QString& message);

    void onControllerBusyChanged(bool busy);

    //  СЛОТ для обработки логов
    void onLogEntryReceived(const LogEntry& entry);

private:
    void initializeUI();
    void connectSignals();
     void setupLogging();
    void updateConnectionUI();
    void updateChannelDisplay(QGroupBox* channelBox, const UIChannelState& channel, bool showCodes);
    void updateControlsState();
    void setLeftPanelEnabled(bool enabled);
    void appendToLog(const QString& message);
    void showStatusMessage(const QString& message, int timeout = 3000);
    void updateWindowTitle();

    // Получение параметров из UI
    uint16_t getSelectedAddress() const;
    QString getSelectedIP() const;
    quint16 getSelectedPort() const;

    // Форматирование логов
    QString formatLogEntryToHtml(const LogEntry& entry);
    QString formatLogEntryToPlainText(const LogEntry& entry);

    // Утилиты
    QString formatPower(float watts, bool showCodes) const;
    QString formatTemperature(float celsius, bool showCodes) const;
    QString formatVSWR(float vswr, bool showCodes) const;

    //подгрузка ресурсов
    void loadLogStyles();
    QString  loadResourceCSS(const QString& resourcePath);
    QString  loadResourceTemplate(const QString& resourcePath);
    void loadCSS();

    void debugLogSystem();
private:
    Ui::TesterWindow *ui;
    PPBController* m_controller;
    pult* m_pultWindow;
    bool m_displayAsCodes;
    uint8_t m_currentPPBIndex;
    bool m_isExiting;

     QVector<LogEntry> m_logEntries; // Храним логи для экспорта
    LogUIManager* m_logUIManager;
};

#endif // TESTERWINDOW_H
