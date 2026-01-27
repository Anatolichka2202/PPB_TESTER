#ifndef TESTERWINDOW_H
#define TESTERWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMap>
#include <QGroupBox>

#include "../core/ppbcommunication.h"
#include "../core/dataconverter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TesterWindow;
}
QT_END_NAMESPACE

// Структура для канала
struct UIChannelState {
    float power;
    float temperature;
    float vswr;
    bool isOk;
};

class TesterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TesterWindow(QWidget *parent = nullptr);
    ~TesterWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // === СЛОТЫ ДЛЯ КНОПОК ===
    void onConnectClicked();
    void onPollStatusClicked();
    void onResetClicked();
    void onApplyParametersClicked();
    void onAutoPollToggled(bool checked);
    void onTestSequenceClicked();

    // ФУ команды
    void onFUTransmitClicked();
    void onFUReceiveClicked();

    // Сценарии
    void onLoadScenarioClicked();
    void onRunScenarioClicked();
    void onStopScenarioClicked();

    // Настройки отображения
    void onDisplayModeChanged(bool showCodes);
    void onPPBSelected(int index);

    // Логи
    void onClearLogClicked();
    void onExportLogClicked();
    void onExitClicked();

    // === СЛОТЫ ОТ PPBCOMMUNICATION ===
    void onConnectionStateChanged(PPBState state);
    void onBusyChanged(bool busy);
    void onConnected();
    void onDisconnected();
    void onStatusReceived(uint16_t address, const QVector<QByteArray>& data);
    void onTestProgress(int current, int total, OperationType operation);
    void onTestCompleted(bool success, const QString& report, OperationType operation);
    void onTestPacketReceived(const DataPacket& packet);
    void onErrorOccurred(const QString& error);
    void onTimeoutOccurred(const QString& operation);
    void onLogMessage(const QString& message);

    // Таймеры
    void onAutoPollTimeout();
    void onUIUpdateTimeout();

private:
    // === МЕТОДЫ ИНИЦИАЛИЗАЦИИ ===
    void initializeUI();
    void initializeCommunication();
    void initializeTimers();
    void connectSignals();

    // === МЕТОДЫ ОБНОВЛЕНИЯ UI ===
    void updateConnectionUI();
    void updateChannelDisplay(QGroupBox* channelBox, const UIChannelState& channel, bool showCodes);
    void updateControlsState();
    void setLeftPanelEnabled(bool enabled);

    // === ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ===
    uint16_t getSelectedAddress() const;
    QString getSelectedIP() const;
    quint16 getSelectedPort() const;

    void showStatusMessage(const QString& message, int timeout = 3000);
    void updateWindowTitle();

    // === ОБРАБОТКА ДАННЫХ ===
    void processStatusData(uint16_t address, const QVector<QByteArray>& data);
    void updateStatusFromData(uint16_t address, const QByteArray& channel1Data, const QByteArray& channel2Data);

    // === РАБОТА С ЛОГАМИ ===
    void appendToLog(const QString& message);
    void clearLog();
    void exportLog();

private:
    Ui::TesterWindow *ui;
    PPBCommunication* m_communication;

    // Состояние UI
    QMap<uint8_t, UIChannelState> m_channel1States;
    QMap<uint8_t, UIChannelState> m_channel2States;
    uint8_t m_currentPPBIndex;
    bool m_displayAsCodes;
    bool m_autoPollEnabled;

    // Таймеры
    QTimer* m_autoPollTimer;
    QTimer* m_uiUpdateTimer;

    // Флаги
    bool m_isExiting;

    uint16_t indexToAddress(uint8_t index) const {
        if (index > 7) return BROADCAST_ADDRESS;
        return (1 << index);  // ППБ1=1, ППБ2=2, ППБ3=4...
    }

    // Преобразование адреса в индекс
    uint8_t addressToIndex(uint16_t address) const {
        for (int i = 0; i < 8; i++) {
            if (address == (1 << i)) return i;
        }
        return 0xFF;  // Невалидный адрес
    }
};

#endif // TESTERWINDOW_H
