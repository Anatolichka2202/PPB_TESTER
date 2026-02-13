#ifndef AKIP_PULT_H
#define AKIP_PULT_H

#include <QWidget>

namespace Ui {
class akip_pult;
}

class akip_pult : public QWidget
{
    Q_OBJECT

public:
    explicit akip_pult(QWidget *parent = nullptr);
    ~akip_pult();

private:
    Ui::akip_pult *ui;
};

#endif // AKIP_PULT_H
