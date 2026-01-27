/********************************************************************************
** Form generated from reading UI file 'pult.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PULT_H
#define UI_PULT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_pult
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QPushButton *PRBS_S2MCommand;
    QPushButton *PRBS_M2SCommand;
    QPushButton *BER_TCommand;
    QPushButton *TSComand;
    QPushButton *ChecksumCommand;
    QPushButton *BER_FCommand;
    QPushButton *TCCommand;
    QPushButton *CleanCommand;
    QPushButton *VERSComand;
    QPushButton *VolumeComand;
    QPushButton *ProgramCommand;
    QPushButton *DropCommand;

    void setupUi(QWidget *pult)
    {
        if (pult->objectName().isEmpty())
            pult->setObjectName("pult");
        pult->resize(633, 875);
        gridLayoutWidget = new QWidget(pult);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(19, 39, 601, 821));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        PRBS_S2MCommand = new QPushButton(gridLayoutWidget);
        PRBS_S2MCommand->setObjectName("PRBS_S2MCommand");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PRBS_S2MCommand->sizePolicy().hasHeightForWidth());
        PRBS_S2MCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(PRBS_S2MCommand, 4, 1, 1, 1);

        PRBS_M2SCommand = new QPushButton(gridLayoutWidget);
        PRBS_M2SCommand->setObjectName("PRBS_M2SCommand");
        sizePolicy.setHeightForWidth(PRBS_M2SCommand->sizePolicy().hasHeightForWidth());
        PRBS_M2SCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(PRBS_M2SCommand, 4, 0, 1, 1);

        BER_TCommand = new QPushButton(gridLayoutWidget);
        BER_TCommand->setObjectName("BER_TCommand");
        sizePolicy.setHeightForWidth(BER_TCommand->sizePolicy().hasHeightForWidth());
        BER_TCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(BER_TCommand, 5, 0, 1, 1);

        TSComand = new QPushButton(gridLayoutWidget);
        TSComand->setObjectName("TSComand");
        sizePolicy.setHeightForWidth(TSComand->sizePolicy().hasHeightForWidth());
        TSComand->setSizePolicy(sizePolicy);
        TSComand->setMinimumSize(QSize(0, 0));

        gridLayout->addWidget(TSComand, 0, 0, 1, 1);

        ChecksumCommand = new QPushButton(gridLayoutWidget);
        ChecksumCommand->setObjectName("ChecksumCommand");
        sizePolicy.setHeightForWidth(ChecksumCommand->sizePolicy().hasHeightForWidth());
        ChecksumCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(ChecksumCommand, 2, 0, 1, 1);

        BER_FCommand = new QPushButton(gridLayoutWidget);
        BER_FCommand->setObjectName("BER_FCommand");
        sizePolicy.setHeightForWidth(BER_FCommand->sizePolicy().hasHeightForWidth());
        BER_FCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(BER_FCommand, 5, 1, 1, 1);

        TCCommand = new QPushButton(gridLayoutWidget);
        TCCommand->setObjectName("TCCommand");
        sizePolicy.setHeightForWidth(TCCommand->sizePolicy().hasHeightForWidth());
        TCCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(TCCommand, 0, 1, 1, 1);

        CleanCommand = new QPushButton(gridLayoutWidget);
        CleanCommand->setObjectName("CleanCommand");
        sizePolicy.setHeightForWidth(CleanCommand->sizePolicy().hasHeightForWidth());
        CleanCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(CleanCommand, 3, 0, 1, 1);

        VERSComand = new QPushButton(gridLayoutWidget);
        VERSComand->setObjectName("VERSComand");
        sizePolicy.setHeightForWidth(VERSComand->sizePolicy().hasHeightForWidth());
        VERSComand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(VERSComand, 1, 0, 1, 1);

        VolumeComand = new QPushButton(gridLayoutWidget);
        VolumeComand->setObjectName("VolumeComand");
        sizePolicy.setHeightForWidth(VolumeComand->sizePolicy().hasHeightForWidth());
        VolumeComand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(VolumeComand, 1, 1, 1, 1);

        ProgramCommand = new QPushButton(gridLayoutWidget);
        ProgramCommand->setObjectName("ProgramCommand");
        sizePolicy.setHeightForWidth(ProgramCommand->sizePolicy().hasHeightForWidth());
        ProgramCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(ProgramCommand, 2, 1, 1, 1);

        DropCommand = new QPushButton(gridLayoutWidget);
        DropCommand->setObjectName("DropCommand");
        sizePolicy.setHeightForWidth(DropCommand->sizePolicy().hasHeightForWidth());
        DropCommand->setSizePolicy(sizePolicy);

        gridLayout->addWidget(DropCommand, 3, 1, 1, 1);


        retranslateUi(pult);

        QMetaObject::connectSlotsByName(pult);
    } // setupUi

    void retranslateUi(QWidget *pult)
    {
        pult->setWindowTitle(QCoreApplication::translate("pult", "Form", nullptr));
        PRBS_S2MCommand->setText(QCoreApplication::translate("pult", "\320\262\321\213\320\264\320\260\321\202\321\214 \321\202\320\265\321\201\321\202\320\276\320\262\321\203\321\216 \n"
"\320\277\320\276\321\201\320\273\320\265\320\264\320\276\320\262\321\202\320\265\320\273\321\214\320\275\320\276\321\201\321\202\321\214 \n"
"\320\264\320\260\320\275\320\275\321\213\321\205 \n"
"PRBS_S2M", nullptr));
        PRBS_M2SCommand->setText(QCoreApplication::translate("pult", "\320\277\321\200\320\270\320\275\321\217\321\202\321\214 \321\202\320\265\321\201\321\202\320\276\320\262\321\203\321\216 \320\277\320\276\321\201\320\273\320\265\320\264\320\276\320\262\320\260\321\202\320\265\320\273\321\214\320\275\320\276\321\201\321\202\321\214 \n"
"\320\264\320\260\320\275\320\275\321\213\321\205\n"
"PRBS_M2S", nullptr));
        BER_TCommand->setText(QCoreApplication::translate("pult", "\320\222\321\213\320\264\320\260\321\202\321\214 \320\272\320\276\321\215\321\204\321\204\320\270\321\206\320\270\320\265\320\275\321\202 \320\276\321\210\320\270\320\261\320\276\320\272 \320\273\320\270\320\275\320\270\320\270 \320\242\320\243\n"
"BER_T", nullptr));
        TSComand->setText(QCoreApplication::translate("pult", "\320\262\321\213\320\264\320\260\321\202\321\214 \321\202\320\265\321\205.\321\201\320\276\321\201\321\202\320\276\321\217\320\275\320\270\320\265\n"
"TS", nullptr));
        ChecksumCommand->setText(QCoreApplication::translate("pult", "\320\262\321\213\320\264\320\260\321\202\321\214 \320\272\320\276\320\275\321\202\321\200\320\276\320\273\321\214\320\275\321\203\321\216 \321\201\321\203\320\274\320\274\321\203 \321\204\320\260\320\271\320\273\320\260 \320\237\320\236\n"
"Checksum", nullptr));
        BER_FCommand->setText(QCoreApplication::translate("pult", " \320\222\321\213\320\264\320\260\321\202\321\214 \320\272\320\276\321\215\321\204\321\204\320\270\321\206\320\270\320\265\320\275\321\202 \320\276\321\210\320\270\320\261\320\276\320\272 \320\273\320\270\320\275\320\270\320\270 \320\244\320\243 \n"
"BER_F", nullptr));
        TCCommand->setText(QCoreApplication::translate("pult", "\320\277\321\200\320\270\320\275\321\217\321\202\321\214 \321\202\320\265\321\205.\321\203\320\277\321\200\320\260\320\262\320\273\320\265\320\275\320\270\320\265\n"
"TC", nullptr));
        CleanCommand->setText(QCoreApplication::translate("pult", "\320\276\321\207\320\270\321\201\320\270\321\202\321\214 \320\262\321\200\320\265\320\274\320\265\320\275\320\275\321\213\320\271 \321\204\320\260\320\271\320\273 \320\237\320\236\n"
"Clean", nullptr));
        VERSComand->setText(QCoreApplication::translate("pult", "\320\262\321\213\320\264\320\260\321\202\321\214 \320\262\320\265\321\200\321\201\320\270\321\216\n"
"VERS", nullptr));
        VolumeComand->setText(QCoreApplication::translate("pult", "\320\277\321\200\320\270\320\275\321\217\321\202\321\214 \321\202\320\276\320\274 \320\270\321\201\320\277. \321\204\320\260\320\271\320\273\320\260 \320\237\320\236\n"
"Volume", nullptr));
        ProgramCommand->setText(QCoreApplication::translate("pult", "\320\276\320\261\320\275\320\276\320\262\320\270\321\202\321\214 \320\270\321\201\320\277. \321\204\320\260\320\271\320\273 \320\237\320\236\n"
"Program", nullptr));
        DropCommand->setText(QCoreApplication::translate("pult", "\320\262\321\213\320\264\320\260\321\202\321\214 \320\272\320\276\320\273-\320\262\320\276 \320\276\321\202\320\261\321\200\320\276\321\210\320\265\320\275\320\275\321\213\321\205 \320\277\320\260\320\272\320\265\321\202\320\276\320\262\n"
"\320\244\320\243 \320\277\320\276 \320\272\320\276\320\275\321\202\321\200\320\276\320\273\321\214\320\275\320\276\320\271 \321\201\321\203\320\274\320\274\320\265\n"
"Drop", nullptr));
    } // retranslateUi

};

namespace Ui {
    class pult: public Ui_pult {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PULT_H
