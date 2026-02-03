#ifndef PULT_H
#define PULT_H

#include <QWidget>
#include "ppbcontroller.h"
#include <QMessageBox>
#include <QLabel>
namespace Ui {
class pult;
}

class pult : public QWidget
{
    Q_OBJECT

public:
    explicit pult(uint16_t address, PPBController* controller, QWidget *parent = nullptr);
    ~pult();

private slots:
    void on_TSComand_clicked();
    void on_TCCommand_clicked();
    void on_PRBS_S2MCommand_clicked();
    void on_PRBS_M2SCommand_clicked();
    void on_VERSComand_clicked();
    void on_VolumeComand_clicked();
    void on_ChecksumCommand_clicked();
    void on_ProgramCommand_clicked();
    void on_CleanCommand_clicked();
    void on_DropCommand_clicked();
    void on_BER_TCommand_clicked();
    void on_BER_FCommand_clicked();
    void onControllerLogMessage(const QString& message);
    void onControllerErrorOccurred(const QString& error);
    void onControllerOperationCompleted(bool success, const QString& message);

private:
    Ui::pult *ui;
    PPBController* m_controller;
    uint16_t m_address;
    QTimer* m_statusTimer;
};

#endif // PULT_H
