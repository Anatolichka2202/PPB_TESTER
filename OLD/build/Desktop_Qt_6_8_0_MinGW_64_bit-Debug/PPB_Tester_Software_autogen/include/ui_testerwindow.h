/********************************************************************************
** Form generated from reading UI file 'testerwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTERWINDOW_H
#define UI_TESTERWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TesterWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *mainLayout;
    QWidget *leftPanel;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxControl;
    QFormLayout *formLayout;
    QLabel *label;
    QComboBox *comboBoxPPBSelect;
    QGridLayout *gridLayout_7;
    QPushButton *pushButtonPollStatus;
    QPushButton *pushButtonReset;
    QPushButton *pushButtonTestSequence;
    QCheckBox *checkBoxAutoPoll;
    QGroupBox *groupBoxParametrs;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QLineEdit *lineEditPulseDuration;
    QLabel *label_3;
    QLineEdit *lineEditDutyCycle;
    QLabel *label_4;
    QLineEdit *lineEditPulseDelay;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButtonApplyToPPB1;
    QGroupBox *groupBoxFU;
    QGridLayout *gridLayout_6;
    QRadioButton *radioButtonFUReceive;
    QRadioButton *radioButtonFUTransmit;
    QCheckBox *checkBoxDelayed;
    QCheckBox *checkBoxInstant;
    QGroupBox *groupBoxScenarios;
    QGridLayout *gridLayout_5;
    QPushButton *pushButtonRunScenario;
    QPushButton *pushButtonStopScenario;
    QLabel *labelScenarioFile;
    QPushButton *pushButtonLoadScenario;
    QGroupBox *groupBoxDataView;
    QHBoxLayout *horizontalLayout;
    QRadioButton *radioButtonShowPhysical;
    QRadioButton *radioButtonShowCodes;
    QGroupBox *groupBoxNetwork;
    QGridLayout *gridLayout_4;
    QLineEdit *lineEditIPAddress;
    QPushButton *pushButtonConnect;
    QLabel *labelConnectionStatus;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonExit;
    QLineEdit *lineEditPort;
    QLabel *label_for_pupupu;
    QSpacerItem *verticalSpacer;
    QSplitter *rightPanel;
    QWidget *topRight;
    QVBoxLayout *stateLayout;
    QTabWidget *tabWidget;
    QWidget *tab_PPB1;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *PPBChanelLayout;
    QGroupBox *groupBox_PPB1_chanel_1;
    QGridLayout *gridLayout;
    QLabel *label_capacity_ppb_chanel_1;
    QLabel *Power_circle_label_ppb_chanel_1;
    QLabel *label_power_ppb_chanel_1;
    QLabel *KSWN_circle_label_ppb_chanel_1;
    QLabel *label_kswn_ppb_chanel_1;
    QLabel *statuslabel_temp_ppb_chanel_1;
    QLabel *statuslabel_capacity_ppb_chanel_1;
    QLabel *statuslabel_power_ppb_chanel_1;
    QLabel *label_tem_ppb_chanel_1;
    QLabel *statuslabel_kswn_ppb_chanel_1;
    QLabel *tem_circle_label_ppb_chanel_1;
    QLabel *Capacity_circle_label_ppb_chanel_1;
    QGroupBox *groupBox_PPB1_chanel_2;
    QGridLayout *gridLayout_3;
    QLabel *label_capacity_ppb_chanel_2;
    QLabel *Power_circle_label_ppb_chanel_2;
    QLabel *label_tem_ppb_chanel_2;
    QLabel *label_kswn_ppb_chanel_2;
    QLabel *KSWN_circle_label_ppb_chanel_2;
    QLabel *tem_circle_label_ppb_chanel_2;
    QLabel *label_power_ppb_chanel_2;
    QLabel *Capacity_circle_label_ppb_chanel_2;
    QLabel *statuslabel_power_ppb_chanel_2;
    QLabel *statuslabel_capacity_ppb_chanel_2;
    QLabel *statuslabel_kswn_ppb_chanel_2;
    QLabel *statuslabel_temp_ppb_chanel_2;
    QWidget *tab_PPB2;
    QVBoxLayout *verticalLayout_4;
    QWidget *bottomRight;
    QVBoxLayout *logLayout;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_name;
    QHBoxLayout *horizontalLayout_6;
    QComboBox *comboBoxLogLevel;
    QComboBox *comboBoxLogCategory;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *pushButtonClearLog;
    QPushButton *pushButtonExportLog;
    QTextBrowser *textBrowser;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TesterWindow)
    {
        if (TesterWindow->objectName().isEmpty())
            TesterWindow->setObjectName("TesterWindow");
        TesterWindow->resize(1726, 958);
        TesterWindow->setMinimumSize(QSize(1000, 700));
        centralwidget = new QWidget(TesterWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QHBoxLayout(centralwidget);
        mainLayout->setObjectName("mainLayout");
        leftPanel = new QWidget(centralwidget);
        leftPanel->setObjectName("leftPanel");
        leftPanel->setEnabled(true);
        verticalLayout = new QVBoxLayout(leftPanel);
        verticalLayout->setObjectName("verticalLayout");
        groupBoxControl = new QGroupBox(leftPanel);
        groupBoxControl->setObjectName("groupBoxControl");
        groupBoxControl->setMinimumSize(QSize(0, 150));
        formLayout = new QFormLayout(groupBoxControl);
        formLayout->setObjectName("formLayout");
        label = new QLabel(groupBoxControl);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        comboBoxPPBSelect = new QComboBox(groupBoxControl);
        comboBoxPPBSelect->addItem(QString());
        comboBoxPPBSelect->addItem(QString());
        comboBoxPPBSelect->setObjectName("comboBoxPPBSelect");

        formLayout->setWidget(0, QFormLayout::FieldRole, comboBoxPPBSelect);

        gridLayout_7 = new QGridLayout();
        gridLayout_7->setObjectName("gridLayout_7");
        pushButtonPollStatus = new QPushButton(groupBoxControl);
        pushButtonPollStatus->setObjectName("pushButtonPollStatus");
        pushButtonPollStatus->setMinimumSize(QSize(0, 35));

        gridLayout_7->addWidget(pushButtonPollStatus, 0, 0, 1, 1);

        pushButtonReset = new QPushButton(groupBoxControl);
        pushButtonReset->setObjectName("pushButtonReset");
        pushButtonReset->setMinimumSize(QSize(0, 35));

        gridLayout_7->addWidget(pushButtonReset, 0, 1, 1, 1);

        pushButtonTestSequence = new QPushButton(groupBoxControl);
        pushButtonTestSequence->setObjectName("pushButtonTestSequence");
        pushButtonTestSequence->setMinimumSize(QSize(0, 35));

        gridLayout_7->addWidget(pushButtonTestSequence, 1, 0, 1, 2);


        formLayout->setLayout(2, QFormLayout::SpanningRole, gridLayout_7);

        checkBoxAutoPoll = new QCheckBox(groupBoxControl);
        checkBoxAutoPoll->setObjectName("checkBoxAutoPoll");

        formLayout->setWidget(3, QFormLayout::SpanningRole, checkBoxAutoPoll);


        verticalLayout->addWidget(groupBoxControl);

        groupBoxParametrs = new QGroupBox(leftPanel);
        groupBoxParametrs->setObjectName("groupBoxParametrs");
        groupBoxParametrs->setMinimumSize(QSize(0, 180));
        gridLayout_2 = new QGridLayout(groupBoxParametrs);
        gridLayout_2->setObjectName("gridLayout_2");
        label_2 = new QLabel(groupBoxParametrs);
        label_2->setObjectName("label_2");

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        lineEditPulseDuration = new QLineEdit(groupBoxParametrs);
        lineEditPulseDuration->setObjectName("lineEditPulseDuration");
        lineEditPulseDuration->setMaximumSize(QSize(100, 16777215));

        gridLayout_2->addWidget(lineEditPulseDuration, 0, 1, 1, 1);

        label_3 = new QLabel(groupBoxParametrs);
        label_3->setObjectName("label_3");

        gridLayout_2->addWidget(label_3, 1, 0, 1, 1);

        lineEditDutyCycle = new QLineEdit(groupBoxParametrs);
        lineEditDutyCycle->setObjectName("lineEditDutyCycle");
        lineEditDutyCycle->setMaximumSize(QSize(100, 16777215));

        gridLayout_2->addWidget(lineEditDutyCycle, 1, 1, 1, 1);

        label_4 = new QLabel(groupBoxParametrs);
        label_4->setObjectName("label_4");

        gridLayout_2->addWidget(label_4, 2, 0, 1, 1);

        lineEditPulseDelay = new QLineEdit(groupBoxParametrs);
        lineEditPulseDelay->setObjectName("lineEditPulseDelay");
        lineEditPulseDelay->setMaximumSize(QSize(100, 16777215));

        gridLayout_2->addWidget(lineEditPulseDelay, 2, 1, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        pushButtonApplyToPPB1 = new QPushButton(groupBoxParametrs);
        pushButtonApplyToPPB1->setObjectName("pushButtonApplyToPPB1");
        pushButtonApplyToPPB1->setMinimumSize(QSize(0, 40));

        horizontalLayout_4->addWidget(pushButtonApplyToPPB1);


        gridLayout_2->addLayout(horizontalLayout_4, 3, 0, 1, 2);


        verticalLayout->addWidget(groupBoxParametrs);

        groupBoxFU = new QGroupBox(leftPanel);
        groupBoxFU->setObjectName("groupBoxFU");
        groupBoxFU->setMinimumSize(QSize(0, 150));
        gridLayout_6 = new QGridLayout(groupBoxFU);
        gridLayout_6->setObjectName("gridLayout_6");
        radioButtonFUReceive = new QRadioButton(groupBoxFU);
        radioButtonFUReceive->setObjectName("radioButtonFUReceive");
        radioButtonFUReceive->setChecked(true);

        gridLayout_6->addWidget(radioButtonFUReceive, 0, 0, 1, 1);

        radioButtonFUTransmit = new QRadioButton(groupBoxFU);
        radioButtonFUTransmit->setObjectName("radioButtonFUTransmit");

        gridLayout_6->addWidget(radioButtonFUTransmit, 0, 1, 1, 1);

        checkBoxDelayed = new QCheckBox(groupBoxFU);
        checkBoxDelayed->setObjectName("checkBoxDelayed");

        gridLayout_6->addWidget(checkBoxDelayed, 1, 1, 1, 1);

        checkBoxInstant = new QCheckBox(groupBoxFU);
        checkBoxInstant->setObjectName("checkBoxInstant");

        gridLayout_6->addWidget(checkBoxInstant, 1, 0, 1, 1);


        verticalLayout->addWidget(groupBoxFU);

        groupBoxScenarios = new QGroupBox(leftPanel);
        groupBoxScenarios->setObjectName("groupBoxScenarios");
        gridLayout_5 = new QGridLayout(groupBoxScenarios);
        gridLayout_5->setObjectName("gridLayout_5");
        pushButtonRunScenario = new QPushButton(groupBoxScenarios);
        pushButtonRunScenario->setObjectName("pushButtonRunScenario");

        gridLayout_5->addWidget(pushButtonRunScenario, 2, 1, 1, 1);

        pushButtonStopScenario = new QPushButton(groupBoxScenarios);
        pushButtonStopScenario->setObjectName("pushButtonStopScenario");

        gridLayout_5->addWidget(pushButtonStopScenario, 2, 2, 1, 1);

        labelScenarioFile = new QLabel(groupBoxScenarios);
        labelScenarioFile->setObjectName("labelScenarioFile");

        gridLayout_5->addWidget(labelScenarioFile, 0, 2, 1, 1);

        pushButtonLoadScenario = new QPushButton(groupBoxScenarios);
        pushButtonLoadScenario->setObjectName("pushButtonLoadScenario");

        gridLayout_5->addWidget(pushButtonLoadScenario, 0, 1, 1, 1);


        verticalLayout->addWidget(groupBoxScenarios);

        groupBoxDataView = new QGroupBox(leftPanel);
        groupBoxDataView->setObjectName("groupBoxDataView");
        horizontalLayout = new QHBoxLayout(groupBoxDataView);
        horizontalLayout->setObjectName("horizontalLayout");
        radioButtonShowPhysical = new QRadioButton(groupBoxDataView);
        radioButtonShowPhysical->setObjectName("radioButtonShowPhysical");

        horizontalLayout->addWidget(radioButtonShowPhysical);

        radioButtonShowCodes = new QRadioButton(groupBoxDataView);
        radioButtonShowCodes->setObjectName("radioButtonShowCodes");

        horizontalLayout->addWidget(radioButtonShowCodes);


        verticalLayout->addWidget(groupBoxDataView);

        groupBoxNetwork = new QGroupBox(leftPanel);
        groupBoxNetwork->setObjectName("groupBoxNetwork");
        gridLayout_4 = new QGridLayout(groupBoxNetwork);
        gridLayout_4->setObjectName("gridLayout_4");
        lineEditIPAddress = new QLineEdit(groupBoxNetwork);
        lineEditIPAddress->setObjectName("lineEditIPAddress");

        gridLayout_4->addWidget(lineEditIPAddress, 0, 0, 1, 1);

        pushButtonConnect = new QPushButton(groupBoxNetwork);
        pushButtonConnect->setObjectName("pushButtonConnect");

        gridLayout_4->addWidget(pushButtonConnect, 1, 0, 1, 1);

        labelConnectionStatus = new QLabel(groupBoxNetwork);
        labelConnectionStatus->setObjectName("labelConnectionStatus");

        gridLayout_4->addWidget(labelConnectionStatus, 1, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout_4->addItem(horizontalSpacer, 1, 2, 1, 1);

        pushButtonExit = new QPushButton(groupBoxNetwork);
        pushButtonExit->setObjectName("pushButtonExit");
        pushButtonExit->setMinimumSize(QSize(0, 0));
        pushButtonExit->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: #ff0000;\n"
"}"));
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("application-exit")));
        pushButtonExit->setIcon(icon);
        pushButtonExit->setIconSize(QSize(50, 50));

        gridLayout_4->addWidget(pushButtonExit, 2, 0, 1, 3);

        lineEditPort = new QLineEdit(groupBoxNetwork);
        lineEditPort->setObjectName("lineEditPort");

        gridLayout_4->addWidget(lineEditPort, 0, 2, 1, 1);

        label_for_pupupu = new QLabel(groupBoxNetwork);
        label_for_pupupu->setObjectName("label_for_pupupu");

        gridLayout_4->addWidget(label_for_pupupu, 0, 1, 1, 1);


        verticalLayout->addWidget(groupBoxNetwork);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        mainLayout->addWidget(leftPanel);

        rightPanel = new QSplitter(centralwidget);
        rightPanel->setObjectName("rightPanel");
        rightPanel->setOrientation(Qt::Orientation::Vertical);
        rightPanel->setHandleWidth(3);
        topRight = new QWidget(rightPanel);
        topRight->setObjectName("topRight");
        stateLayout = new QVBoxLayout(topRight);
        stateLayout->setObjectName("stateLayout");
        tabWidget = new QTabWidget(topRight);
        tabWidget->setObjectName("tabWidget");
        tab_PPB1 = new QWidget();
        tab_PPB1->setObjectName("tab_PPB1");
        verticalLayout_3 = new QVBoxLayout(tab_PPB1);
        verticalLayout_3->setObjectName("verticalLayout_3");
        PPBChanelLayout = new QHBoxLayout();
        PPBChanelLayout->setObjectName("PPBChanelLayout");
        groupBox_PPB1_chanel_1 = new QGroupBox(tab_PPB1);
        groupBox_PPB1_chanel_1->setObjectName("groupBox_PPB1_chanel_1");
        gridLayout = new QGridLayout(groupBox_PPB1_chanel_1);
        gridLayout->setObjectName("gridLayout");
        label_capacity_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        label_capacity_ppb_chanel_1->setObjectName("label_capacity_ppb_chanel_1");

        gridLayout->addWidget(label_capacity_ppb_chanel_1, 3, 1, 1, 1);

        Power_circle_label_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        Power_circle_label_ppb_chanel_1->setObjectName("Power_circle_label_ppb_chanel_1");
        Power_circle_label_ppb_chanel_1->setMinimumSize(QSize(20, 20));
        Power_circle_label_ppb_chanel_1->setMaximumSize(QSize(20, 20));
        Power_circle_label_ppb_chanel_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(Power_circle_label_ppb_chanel_1, 0, 0, 1, 1);

        label_power_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        label_power_ppb_chanel_1->setObjectName("label_power_ppb_chanel_1");

        gridLayout->addWidget(label_power_ppb_chanel_1, 0, 1, 1, 1);

        KSWN_circle_label_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        KSWN_circle_label_ppb_chanel_1->setObjectName("KSWN_circle_label_ppb_chanel_1");
        KSWN_circle_label_ppb_chanel_1->setMinimumSize(QSize(20, 20));
        KSWN_circle_label_ppb_chanel_1->setMaximumSize(QSize(20, 20));
        KSWN_circle_label_ppb_chanel_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(KSWN_circle_label_ppb_chanel_1, 5, 0, 1, 1);

        label_kswn_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        label_kswn_ppb_chanel_1->setObjectName("label_kswn_ppb_chanel_1");

        gridLayout->addWidget(label_kswn_ppb_chanel_1, 5, 1, 1, 1);

        statuslabel_temp_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        statuslabel_temp_ppb_chanel_1->setObjectName("statuslabel_temp_ppb_chanel_1");

        gridLayout->addWidget(statuslabel_temp_ppb_chanel_1, 8, 2, 1, 1);

        statuslabel_capacity_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        statuslabel_capacity_ppb_chanel_1->setObjectName("statuslabel_capacity_ppb_chanel_1");

        gridLayout->addWidget(statuslabel_capacity_ppb_chanel_1, 3, 2, 1, 1);

        statuslabel_power_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        statuslabel_power_ppb_chanel_1->setObjectName("statuslabel_power_ppb_chanel_1");

        gridLayout->addWidget(statuslabel_power_ppb_chanel_1, 0, 2, 1, 1);

        label_tem_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        label_tem_ppb_chanel_1->setObjectName("label_tem_ppb_chanel_1");

        gridLayout->addWidget(label_tem_ppb_chanel_1, 8, 1, 1, 1);

        statuslabel_kswn_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        statuslabel_kswn_ppb_chanel_1->setObjectName("statuslabel_kswn_ppb_chanel_1");

        gridLayout->addWidget(statuslabel_kswn_ppb_chanel_1, 5, 2, 1, 1);

        tem_circle_label_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        tem_circle_label_ppb_chanel_1->setObjectName("tem_circle_label_ppb_chanel_1");
        tem_circle_label_ppb_chanel_1->setMinimumSize(QSize(20, 20));
        tem_circle_label_ppb_chanel_1->setMaximumSize(QSize(20, 20));
        tem_circle_label_ppb_chanel_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(tem_circle_label_ppb_chanel_1, 8, 0, 1, 1);

        Capacity_circle_label_ppb_chanel_1 = new QLabel(groupBox_PPB1_chanel_1);
        Capacity_circle_label_ppb_chanel_1->setObjectName("Capacity_circle_label_ppb_chanel_1");
        Capacity_circle_label_ppb_chanel_1->setMinimumSize(QSize(20, 20));
        Capacity_circle_label_ppb_chanel_1->setMaximumSize(QSize(20, 20));
        Capacity_circle_label_ppb_chanel_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout->addWidget(Capacity_circle_label_ppb_chanel_1, 3, 0, 1, 1);


        PPBChanelLayout->addWidget(groupBox_PPB1_chanel_1);

        groupBox_PPB1_chanel_2 = new QGroupBox(tab_PPB1);
        groupBox_PPB1_chanel_2->setObjectName("groupBox_PPB1_chanel_2");
        gridLayout_3 = new QGridLayout(groupBox_PPB1_chanel_2);
        gridLayout_3->setObjectName("gridLayout_3");
        label_capacity_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        label_capacity_ppb_chanel_2->setObjectName("label_capacity_ppb_chanel_2");

        gridLayout_3->addWidget(label_capacity_ppb_chanel_2, 2, 2, 1, 1);

        Power_circle_label_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        Power_circle_label_ppb_chanel_2->setObjectName("Power_circle_label_ppb_chanel_2");
        Power_circle_label_ppb_chanel_2->setMinimumSize(QSize(20, 20));
        Power_circle_label_ppb_chanel_2->setMaximumSize(QSize(20, 20));
        Power_circle_label_ppb_chanel_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_3->addWidget(Power_circle_label_ppb_chanel_2, 0, 0, 1, 1);

        label_tem_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        label_tem_ppb_chanel_2->setObjectName("label_tem_ppb_chanel_2");

        gridLayout_3->addWidget(label_tem_ppb_chanel_2, 6, 2, 1, 1);

        label_kswn_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        label_kswn_ppb_chanel_2->setObjectName("label_kswn_ppb_chanel_2");

        gridLayout_3->addWidget(label_kswn_ppb_chanel_2, 4, 2, 1, 1);

        KSWN_circle_label_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        KSWN_circle_label_ppb_chanel_2->setObjectName("KSWN_circle_label_ppb_chanel_2");
        KSWN_circle_label_ppb_chanel_2->setMinimumSize(QSize(20, 20));
        KSWN_circle_label_ppb_chanel_2->setMaximumSize(QSize(20, 20));
        KSWN_circle_label_ppb_chanel_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_3->addWidget(KSWN_circle_label_ppb_chanel_2, 4, 0, 1, 1);

        tem_circle_label_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        tem_circle_label_ppb_chanel_2->setObjectName("tem_circle_label_ppb_chanel_2");
        tem_circle_label_ppb_chanel_2->setMinimumSize(QSize(20, 20));
        tem_circle_label_ppb_chanel_2->setMaximumSize(QSize(20, 20));
        tem_circle_label_ppb_chanel_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_3->addWidget(tem_circle_label_ppb_chanel_2, 6, 0, 1, 1);

        label_power_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        label_power_ppb_chanel_2->setObjectName("label_power_ppb_chanel_2");

        gridLayout_3->addWidget(label_power_ppb_chanel_2, 0, 2, 1, 1);

        Capacity_circle_label_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        Capacity_circle_label_ppb_chanel_2->setObjectName("Capacity_circle_label_ppb_chanel_2");
        Capacity_circle_label_ppb_chanel_2->setMinimumSize(QSize(20, 20));
        Capacity_circle_label_ppb_chanel_2->setMaximumSize(QSize(20, 20));
        Capacity_circle_label_ppb_chanel_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_3->addWidget(Capacity_circle_label_ppb_chanel_2, 2, 0, 1, 1);

        statuslabel_power_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        statuslabel_power_ppb_chanel_2->setObjectName("statuslabel_power_ppb_chanel_2");

        gridLayout_3->addWidget(statuslabel_power_ppb_chanel_2, 0, 3, 1, 1);

        statuslabel_capacity_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        statuslabel_capacity_ppb_chanel_2->setObjectName("statuslabel_capacity_ppb_chanel_2");

        gridLayout_3->addWidget(statuslabel_capacity_ppb_chanel_2, 2, 3, 1, 1);

        statuslabel_kswn_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        statuslabel_kswn_ppb_chanel_2->setObjectName("statuslabel_kswn_ppb_chanel_2");

        gridLayout_3->addWidget(statuslabel_kswn_ppb_chanel_2, 4, 3, 1, 1);

        statuslabel_temp_ppb_chanel_2 = new QLabel(groupBox_PPB1_chanel_2);
        statuslabel_temp_ppb_chanel_2->setObjectName("statuslabel_temp_ppb_chanel_2");

        gridLayout_3->addWidget(statuslabel_temp_ppb_chanel_2, 6, 3, 1, 1);


        PPBChanelLayout->addWidget(groupBox_PPB1_chanel_2);


        verticalLayout_3->addLayout(PPBChanelLayout);

        tabWidget->addTab(tab_PPB1, QString());
        tab_PPB2 = new QWidget();
        tab_PPB2->setObjectName("tab_PPB2");
        verticalLayout_4 = new QVBoxLayout(tab_PPB2);
        verticalLayout_4->setObjectName("verticalLayout_4");
        tabWidget->addTab(tab_PPB2, QString());

        stateLayout->addWidget(tabWidget);

        rightPanel->addWidget(topRight);
        bottomRight = new QWidget(rightPanel);
        bottomRight->setObjectName("bottomRight");
        logLayout = new QVBoxLayout(bottomRight);
        logLayout->setObjectName("logLayout");
        groupBox_2 = new QGroupBox(bottomRight);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout_5 = new QVBoxLayout(groupBox_2);
        verticalLayout_5->setObjectName("verticalLayout_5");
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        label_name = new QLabel(groupBox_2);
        label_name->setObjectName("label_name");

        horizontalLayout_5->addWidget(label_name);


        verticalLayout_5->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        comboBoxLogLevel = new QComboBox(groupBox_2);
        comboBoxLogLevel->setObjectName("comboBoxLogLevel");
        comboBoxLogLevel->setMinimumSize(QSize(120, 0));

        horizontalLayout_6->addWidget(comboBoxLogLevel);

        comboBoxLogCategory = new QComboBox(groupBox_2);
        comboBoxLogCategory->setObjectName("comboBoxLogCategory");
        comboBoxLogCategory->setMinimumSize(QSize(120, 0));

        horizontalLayout_6->addWidget(comboBoxLogCategory);


        verticalLayout_5->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        pushButtonClearLog = new QPushButton(groupBox_2);
        pushButtonClearLog->setObjectName("pushButtonClearLog");
        pushButtonClearLog->setMinimumSize(QSize(0, 35));

        horizontalLayout_7->addWidget(pushButtonClearLog);

        pushButtonExportLog = new QPushButton(groupBox_2);
        pushButtonExportLog->setObjectName("pushButtonExportLog");
        pushButtonExportLog->setMinimumSize(QSize(0, 35));

        horizontalLayout_7->addWidget(pushButtonExportLog);


        verticalLayout_5->addLayout(horizontalLayout_7);

        textBrowser = new QTextBrowser(groupBox_2);
        textBrowser->setObjectName("textBrowser");
        textBrowser->setMinimumSize(QSize(0, 200));

        verticalLayout_5->addWidget(textBrowser);


        logLayout->addWidget(groupBox_2);

        rightPanel->addWidget(bottomRight);

        mainLayout->addWidget(rightPanel);

        mainLayout->setStretch(0, 3);
        mainLayout->setStretch(1, 7);
        TesterWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(TesterWindow);
        statusbar->setObjectName("statusbar");
        TesterWindow->setStatusBar(statusbar);

        retranslateUi(TesterWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(TesterWindow);
    } // setupUi

    void retranslateUi(QMainWindow *TesterWindow)
    {
        TesterWindow->setWindowTitle(QCoreApplication::translate("TesterWindow", "\320\237\321\200\320\276\320\263\321\200\320\260\320\274\320\274\320\260 \321\203\320\277\321\200\320\260\320\262\320\273\320\265\320\275\320\270\321\217 \320\237\320\237\320\221", nullptr));
        groupBoxControl->setTitle(QCoreApplication::translate("TesterWindow", "\320\243\320\277\321\200\320\260\320\262\320\273\320\265\320\275\320\270\320\265 \320\237\320\237\320\221", nullptr));
        label->setText(QCoreApplication::translate("TesterWindow", "\320\222\321\213\320\261\320\276\321\200 \320\237\320\237\320\221:", nullptr));
        comboBoxPPBSelect->setItemText(0, QCoreApplication::translate("TesterWindow", "\320\237\320\237\320\2211", nullptr));
        comboBoxPPBSelect->setItemText(1, QCoreApplication::translate("TesterWindow", "\320\237\320\237\320\2212", nullptr));

        pushButtonPollStatus->setText(QCoreApplication::translate("TesterWindow", "\320\236\320\277\321\200\320\276\321\201 \321\201\320\276\321\201\321\202\320\276\321\217\320\275\320\270\321\217 \320\237\320\237\320\221", nullptr));
        pushButtonReset->setText(QCoreApplication::translate("TesterWindow", "\320\241\320\261\321\200\320\276\321\201", nullptr));
        pushButtonTestSequence->setText(QCoreApplication::translate("TesterWindow", "\320\242\320\265\321\201\321\202 \320\277\320\276\321\201\320\273\320\265\320\264\320\276\320\262\320\260\321\202\320\265\320\273\321\214\320\275\320\276\321\201\321\202\321\214 ", nullptr));
        checkBoxAutoPoll->setText(QCoreApplication::translate("TesterWindow", "\320\220\320\262\321\202\320\276\320\276\320\277\321\200\320\276\321\201 (5 \321\201\320\265\320\272)", nullptr));
        groupBoxParametrs->setTitle(QCoreApplication::translate("TesterWindow", "\320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 \320\263\320\265\320\275\320\265\321\200\320\260\321\202\320\276\321\200\320\260 (\320\220\320\232\320\230\320\237)", nullptr));
        label_2->setText(QCoreApplication::translate("TesterWindow", "\320\224\320\273\320\270\321\202\320\265\320\273\321\214\320\275\320\276\321\201\321\202\321\214 \320\270\320\274\320\277\321\203\320\273\321\214\321\201\320\260:", nullptr));
        lineEditPulseDuration->setText(QCoreApplication::translate("TesterWindow", "27000", nullptr));
        label_3->setText(QCoreApplication::translate("TesterWindow", "\320\241\320\272\320\262\320\260\320\266\320\275\320\276\321\201\321\202\321\214:", nullptr));
        lineEditDutyCycle->setText(QCoreApplication::translate("TesterWindow", "3", nullptr));
        label_4->setText(QCoreApplication::translate("TesterWindow", "\320\227\320\260\320\264\320\265\321\200\320\266\320\272\320\260 \320\270\320\274\320\277\321\203\320\273\321\214\321\201\320\260:", nullptr));
        lineEditPulseDelay->setText(QCoreApplication::translate("TesterWindow", "0", nullptr));
        pushButtonApplyToPPB1->setText(QCoreApplication::translate("TesterWindow", "\320\237\321\200\320\270\320\274\320\265\320\275\320\270\321\202\321\214 \320\272 \320\263\320\265\320\275\320\265\321\200\320\260\321\202\320\276\321\200\321\203", nullptr));
        groupBoxFU->setTitle(QCoreApplication::translate("TesterWindow", "\320\240\320\265\320\266\320\270\320\274 \320\244\320\243", nullptr));
        radioButtonFUReceive->setText(QCoreApplication::translate("TesterWindow", "\320\244\320\243 \320\277\321\200\320\270\320\265\320\274", nullptr));
        radioButtonFUTransmit->setText(QCoreApplication::translate("TesterWindow", "\320\244\320\243 \320\277\320\265\321\200\320\265\320\264\320\260\321\207\320\260", nullptr));
        checkBoxDelayed->setText(QCoreApplication::translate("TesterWindow", "\320\241 \320\267\320\260\320\264\320\265\321\200\320\266\320\272\320\276\320\271 50\320\274\320\272\321\201", nullptr));
        checkBoxInstant->setText(QCoreApplication::translate("TesterWindow", "\320\234\320\263\320\275\320\276\320\262\320\265\320\275\320\275\320\276\320\265 \320\277\320\265\321\200\320\265\320\272\320\273\321\216\321\207\320\265\320\275\320\270\320\265", nullptr));
        groupBoxScenarios->setTitle(QCoreApplication::translate("TesterWindow", "\320\241\321\206\320\265\320\275\320\260\321\200\320\270\320\271", nullptr));
        pushButtonRunScenario->setText(QCoreApplication::translate("TesterWindow", "\320\222\321\213\320\277\320\276\320\273\320\275\320\270\321\202\321\214 \321\201\321\206\320\265\320\275\320\260\321\200\320\270\320\271", nullptr));
        pushButtonStopScenario->setText(QCoreApplication::translate("TesterWindow", "\320\236\321\201\321\202\320\260\320\275\320\276\320\262\320\270\321\202\321\214 \321\201\321\206\320\265\320\275\320\260\321\200\320\270\320\271", nullptr));
        labelScenarioFile->setText(QCoreApplication::translate("TesterWindow", "\320\242\321\203\321\202 \320\270\320\274\321\217 \321\204\320\260\320\271\320\273\320\260", nullptr));
        pushButtonLoadScenario->setText(QCoreApplication::translate("TesterWindow", "\320\227\320\260\320\263\321\200\321\203\320\267\320\270\321\202\321\214 \321\201\321\206\320\265\320\275\320\260\321\200\320\270\320\271", nullptr));
        groupBoxDataView->setTitle(QString());
        radioButtonShowPhysical->setText(QCoreApplication::translate("TesterWindow", "\320\244\320\270\320\267\320\270\321\207\320\265\321\201\320\272\320\270\320\265 \320\262\320\265\320\273\320\270\321\207\320\270\320\275\321\213", nullptr));
        radioButtonShowCodes->setText(QCoreApplication::translate("TesterWindow", "\320\232\320\276\320\264\321\213", nullptr));
        groupBoxNetwork->setTitle(QCoreApplication::translate("TesterWindow", "\320\241\320\265\321\202\321\214", nullptr));
        lineEditIPAddress->setText(QCoreApplication::translate("TesterWindow", "IP \320\260\320\264\321\200\320\265\321\201", nullptr));
        pushButtonConnect->setText(QCoreApplication::translate("TesterWindow", "\320\237\320\276\320\264\320\272\320\273\321\216\321\207\320\265\320\275\320\270\320\265", nullptr));
        labelConnectionStatus->setText(QCoreApplication::translate("TesterWindow", "\320\273\320\260\320\274\320\277\320\276\321\207\320\272\320\260", nullptr));
        pushButtonExit->setText(QCoreApplication::translate("TesterWindow", "SUTDOWN. \320\275\320\260\320\266\320\274\320\270.\320\277\320\276\320\277\321\200\320\276\320\261\321\203\320\271", nullptr));
        lineEditPort->setText(QCoreApplication::translate("TesterWindow", "\320\237\320\276\321\200\321\202", nullptr));
        label_for_pupupu->setText(QString());
        groupBox_PPB1_chanel_1->setTitle(QCoreApplication::translate("TesterWindow", "\320\241\320\276\321\201\321\202\320\276\321\217\320\275\320\270\320\265 \320\277\320\265\321\200\320\262\320\276\320\263\320\276 \320\272\320\276\320\274\320\277\320\273\320\265\320\272\321\202\320\260", nullptr));
        label_capacity_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "\320\234\320\276\321\211\320\275\320\276\321\201\321\202\321\214:", nullptr));
        Power_circle_label_ppb_chanel_1->setText(QString());
        label_power_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "\320\237\320\270\321\202\320\260\320\275\320\270\320\265:", nullptr));
        KSWN_circle_label_ppb_chanel_1->setText(QString());
        label_kswn_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "\320\232\320\241\320\222\320\235:", nullptr));
        statuslabel_temp_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "45\302\260C", nullptr));
        statuslabel_capacity_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "1250 \320\222\320\242", nullptr));
        statuslabel_power_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "\320\222 \320\275\320\276\321\200\320\274\320\265", nullptr));
        label_tem_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "\320\242\320\265\320\274\320\277\320\265\321\200\320\260\321\202\321\203\321\200\320\260:", nullptr));
        statuslabel_kswn_ppb_chanel_1->setText(QCoreApplication::translate("TesterWindow", "1.2", nullptr));
        tem_circle_label_ppb_chanel_1->setText(QString());
        Capacity_circle_label_ppb_chanel_1->setText(QString());
        groupBox_PPB1_chanel_2->setTitle(QCoreApplication::translate("TesterWindow", "\320\241\320\276\321\201\321\202\320\276\321\217\320\275\320\270\320\265 \320\262\321\202\320\276\321\200\320\276\320\263\320\276 \320\272\320\276\320\274\320\277\320\273\320\265\320\272\321\202\320\260", nullptr));
        label_capacity_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "\320\234\320\276\321\211\320\275\320\276\321\201\321\202\321\214:", nullptr));
        Power_circle_label_ppb_chanel_2->setText(QString());
        label_tem_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "\320\242\320\265\320\274\320\277\320\265\321\200\320\260\321\202\321\203\321\200\320\260:", nullptr));
        label_kswn_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "\320\232\320\241\320\222\320\235:", nullptr));
        KSWN_circle_label_ppb_chanel_2->setText(QString());
        tem_circle_label_ppb_chanel_2->setText(QString());
        label_power_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "\320\237\320\270\321\202\320\260\320\275\320\270\320\265:", nullptr));
        Capacity_circle_label_ppb_chanel_2->setText(QString());
        statuslabel_power_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "\320\222 \320\275\320\276\321\200\320\274\320\265", nullptr));
        statuslabel_capacity_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "1250 \320\222\320\242", nullptr));
        statuslabel_kswn_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "1.2", nullptr));
        statuslabel_temp_ppb_chanel_2->setText(QCoreApplication::translate("TesterWindow", "45\302\260C", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_PPB1), QCoreApplication::translate("TesterWindow", "\320\237\320\237\320\2211", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_PPB2), QCoreApplication::translate("TesterWindow", "\320\237\320\237\320\2212", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("TesterWindow", "\320\233\320\276\320\263\320\270 \320\262\320\267\320\260\320\270\320\274\320\276\320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217", nullptr));
        label_name->setText(QCoreApplication::translate("TesterWindow", "\320\237\320\260\320\275\320\265\320\273\321\214 \320\270\320\275\321\201\321\202\321\200\321\203\320\274\320\265\320\275\321\202\320\276\320\262:", nullptr));
        pushButtonClearLog->setText(QCoreApplication::translate("TesterWindow", "\320\236\321\207\320\270\321\201\321\202\320\270\321\202\321\214", nullptr));
        pushButtonExportLog->setText(QCoreApplication::translate("TesterWindow", "\320\255\320\272\321\201\320\277\320\276\321\200\321\202", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TesterWindow: public Ui_TesterWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTERWINDOW_H
