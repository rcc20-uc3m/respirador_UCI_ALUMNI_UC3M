/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *onOff_button;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QComboBox *serialPortDropdown;
    QPushButton *pushButton_connect;
    QCustomPlot *pressPlot;
    QCustomPlot *flowPlot;
    QFrame *Fr_frame;
    QLabel *Fr_label;
    QTextEdit *Fr_textEdit;
    QPushButton *Fr_minus_button;
    QPushButton *Fr_plus_button;
    QFrame *ETOI_frame;
    QLabel *Etoi_label;
    QTextEdit *Etoi_textEdit;
    QPushButton *Etoi_minus_button;
    QPushButton *Etoi_plus_button;
    QFrame *PIP_frame;
    QLabel *Pip_label;
    QTextEdit *Pip_textEdit;
    QPushButton *Pip_minus_button;
    QPushButton *Pip_plus_button;
    QFrame *VT_frame;
    QLabel *Vt_label;
    QTextEdit *Vt_textEdit;
    QPushButton *Vt_minus_button;
    QPushButton *Vt_plus_button;
    QFrame *PEEP_frame;
    QLabel *Peep_label;
    QTextEdit *Peep_textEdit;
    QPushButton *Peep_minus_button;
    QPushButton *Peep_plus_button;
    QFrame *PSOP_frame;
    QLabel *Psop_label;
    QTextEdit *Psop_textEdit;
    QPushButton *Psop_minus_button;
    QPushButton *Psop_plus_button;
    QMenuBar *menubar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1200, 682);
        MainWindow->setMaximumSize(QSize(1200, 768));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayoutWidget = new QWidget(centralwidget);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(0, 0, 661, 71));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        onOff_button = new QPushButton(horizontalLayoutWidget);
        onOff_button->setObjectName(QString::fromUtf8("onOff_button"));
        onOff_button->setMinimumSize(QSize(20, 50));
        onOff_button->setBaseSize(QSize(20, 50));

        horizontalLayout->addWidget(onOff_button);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        serialPortDropdown = new QComboBox(horizontalLayoutWidget);
        serialPortDropdown->setObjectName(QString::fromUtf8("serialPortDropdown"));

        horizontalLayout->addWidget(serialPortDropdown);

        pushButton_connect = new QPushButton(horizontalLayoutWidget);
        pushButton_connect->setObjectName(QString::fromUtf8("pushButton_connect"));

        horizontalLayout->addWidget(pushButton_connect);

        pressPlot = new QCustomPlot(centralwidget);
        pressPlot->setObjectName(QString::fromUtf8("pressPlot"));
        pressPlot->setGeometry(QRect(10, 80, 650, 221));
        flowPlot = new QCustomPlot(centralwidget);
        flowPlot->setObjectName(QString::fromUtf8("flowPlot"));
        flowPlot->setGeometry(QRect(10, 300, 650, 221));
        Fr_frame = new QFrame(centralwidget);
        Fr_frame->setObjectName(QString::fromUtf8("Fr_frame"));
        Fr_frame->setGeometry(QRect(10, 520, 100, 95));
        Fr_frame->setBaseSize(QSize(1, 1));
        Fr_frame->setFrameShape(QFrame::Panel);
        Fr_frame->setFrameShadow(QFrame::Raised);
        Fr_label = new QLabel(Fr_frame);
        Fr_label->setObjectName(QString::fromUtf8("Fr_label"));
        Fr_label->setGeometry(QRect(0, 0, 100, 19));
        QFont font;
        font.setPointSize(16);
        Fr_label->setFont(font);
        Fr_label->setAlignment(Qt::AlignCenter);
        Fr_textEdit = new QTextEdit(Fr_frame);
        Fr_textEdit->setObjectName(QString::fromUtf8("Fr_textEdit"));
        Fr_textEdit->setGeometry(QRect(0, 20, 100, 51));
        QFont font1;
        font1.setPointSize(20);
        Fr_textEdit->setFont(font1);
        Fr_textEdit->setReadOnly(true);
        Fr_textEdit->setAcceptRichText(false);
        Fr_minus_button = new QPushButton(Fr_frame);
        Fr_minus_button->setObjectName(QString::fromUtf8("Fr_minus_button"));
        Fr_minus_button->setGeometry(QRect(0, 70, 50, 25));
        QFont font2;
        font2.setPointSize(22);
        font2.setBold(true);
        font2.setWeight(75);
        Fr_minus_button->setFont(font2);
        Fr_plus_button = new QPushButton(Fr_frame);
        Fr_plus_button->setObjectName(QString::fromUtf8("Fr_plus_button"));
        Fr_plus_button->setGeometry(QRect(50, 70, 50, 25));
        Fr_plus_button->setFont(font2);
        ETOI_frame = new QFrame(centralwidget);
        ETOI_frame->setObjectName(QString::fromUtf8("ETOI_frame"));
        ETOI_frame->setGeometry(QRect(120, 520, 100, 95));
        ETOI_frame->setBaseSize(QSize(1, 1));
        ETOI_frame->setFrameShape(QFrame::Panel);
        ETOI_frame->setFrameShadow(QFrame::Raised);
        Etoi_label = new QLabel(ETOI_frame);
        Etoi_label->setObjectName(QString::fromUtf8("Etoi_label"));
        Etoi_label->setGeometry(QRect(0, 0, 100, 19));
        Etoi_label->setFont(font);
        Etoi_label->setAlignment(Qt::AlignCenter);
        Etoi_textEdit = new QTextEdit(ETOI_frame);
        Etoi_textEdit->setObjectName(QString::fromUtf8("Etoi_textEdit"));
        Etoi_textEdit->setGeometry(QRect(0, 20, 100, 51));
        Etoi_textEdit->setFont(font1);
        Etoi_textEdit->setReadOnly(true);
        Etoi_textEdit->setAcceptRichText(false);
        Etoi_minus_button = new QPushButton(ETOI_frame);
        Etoi_minus_button->setObjectName(QString::fromUtf8("Etoi_minus_button"));
        Etoi_minus_button->setGeometry(QRect(0, 70, 50, 25));
        Etoi_minus_button->setFont(font2);
        Etoi_plus_button = new QPushButton(ETOI_frame);
        Etoi_plus_button->setObjectName(QString::fromUtf8("Etoi_plus_button"));
        Etoi_plus_button->setGeometry(QRect(50, 70, 50, 25));
        Etoi_plus_button->setFont(font2);
        PIP_frame = new QFrame(centralwidget);
        PIP_frame->setObjectName(QString::fromUtf8("PIP_frame"));
        PIP_frame->setGeometry(QRect(230, 520, 100, 95));
        PIP_frame->setBaseSize(QSize(1, 1));
        PIP_frame->setFrameShape(QFrame::Panel);
        PIP_frame->setFrameShadow(QFrame::Raised);
        Pip_label = new QLabel(PIP_frame);
        Pip_label->setObjectName(QString::fromUtf8("Pip_label"));
        Pip_label->setGeometry(QRect(0, 0, 100, 19));
        Pip_label->setFont(font);
        Pip_label->setAlignment(Qt::AlignCenter);
        Pip_textEdit = new QTextEdit(PIP_frame);
        Pip_textEdit->setObjectName(QString::fromUtf8("Pip_textEdit"));
        Pip_textEdit->setGeometry(QRect(0, 20, 100, 51));
        Pip_textEdit->setFont(font1);
        Pip_textEdit->setReadOnly(true);
        Pip_textEdit->setAcceptRichText(false);
        Pip_minus_button = new QPushButton(PIP_frame);
        Pip_minus_button->setObjectName(QString::fromUtf8("Pip_minus_button"));
        Pip_minus_button->setGeometry(QRect(0, 70, 50, 25));
        Pip_minus_button->setFont(font2);
        Pip_plus_button = new QPushButton(PIP_frame);
        Pip_plus_button->setObjectName(QString::fromUtf8("Pip_plus_button"));
        Pip_plus_button->setGeometry(QRect(50, 70, 50, 25));
        Pip_plus_button->setFont(font2);
        VT_frame = new QFrame(centralwidget);
        VT_frame->setObjectName(QString::fromUtf8("VT_frame"));
        VT_frame->setGeometry(QRect(340, 520, 100, 95));
        VT_frame->setBaseSize(QSize(1, 1));
        VT_frame->setFrameShape(QFrame::Panel);
        VT_frame->setFrameShadow(QFrame::Raised);
        Vt_label = new QLabel(VT_frame);
        Vt_label->setObjectName(QString::fromUtf8("Vt_label"));
        Vt_label->setGeometry(QRect(0, 0, 100, 19));
        Vt_label->setFont(font);
        Vt_label->setAlignment(Qt::AlignCenter);
        Vt_textEdit = new QTextEdit(VT_frame);
        Vt_textEdit->setObjectName(QString::fromUtf8("Vt_textEdit"));
        Vt_textEdit->setGeometry(QRect(0, 20, 100, 51));
        Vt_textEdit->setFont(font1);
        Vt_textEdit->setReadOnly(true);
        Vt_textEdit->setAcceptRichText(false);
        Vt_minus_button = new QPushButton(VT_frame);
        Vt_minus_button->setObjectName(QString::fromUtf8("Vt_minus_button"));
        Vt_minus_button->setGeometry(QRect(0, 70, 50, 25));
        Vt_minus_button->setFont(font2);
        Vt_plus_button = new QPushButton(VT_frame);
        Vt_plus_button->setObjectName(QString::fromUtf8("Vt_plus_button"));
        Vt_plus_button->setGeometry(QRect(50, 70, 50, 25));
        Vt_plus_button->setFont(font2);
        PEEP_frame = new QFrame(centralwidget);
        PEEP_frame->setObjectName(QString::fromUtf8("PEEP_frame"));
        PEEP_frame->setGeometry(QRect(450, 520, 100, 95));
        PEEP_frame->setBaseSize(QSize(1, 1));
        PEEP_frame->setFrameShape(QFrame::Panel);
        PEEP_frame->setFrameShadow(QFrame::Raised);
        Peep_label = new QLabel(PEEP_frame);
        Peep_label->setObjectName(QString::fromUtf8("Peep_label"));
        Peep_label->setGeometry(QRect(0, 0, 100, 19));
        Peep_label->setFont(font);
        Peep_label->setAlignment(Qt::AlignCenter);
        Peep_textEdit = new QTextEdit(PEEP_frame);
        Peep_textEdit->setObjectName(QString::fromUtf8("Peep_textEdit"));
        Peep_textEdit->setGeometry(QRect(0, 20, 100, 51));
        Peep_textEdit->setFont(font1);
        Peep_textEdit->setReadOnly(true);
        Peep_textEdit->setAcceptRichText(false);
        Peep_minus_button = new QPushButton(PEEP_frame);
        Peep_minus_button->setObjectName(QString::fromUtf8("Peep_minus_button"));
        Peep_minus_button->setGeometry(QRect(0, 70, 50, 25));
        Peep_minus_button->setFont(font2);
        Peep_plus_button = new QPushButton(PEEP_frame);
        Peep_plus_button->setObjectName(QString::fromUtf8("Peep_plus_button"));
        Peep_plus_button->setGeometry(QRect(50, 70, 50, 25));
        Peep_plus_button->setFont(font2);
        PSOP_frame = new QFrame(centralwidget);
        PSOP_frame->setObjectName(QString::fromUtf8("PSOP_frame"));
        PSOP_frame->setGeometry(QRect(560, 520, 100, 95));
        PSOP_frame->setBaseSize(QSize(1, 1));
        PSOP_frame->setFrameShape(QFrame::Panel);
        PSOP_frame->setFrameShadow(QFrame::Raised);
        Psop_label = new QLabel(PSOP_frame);
        Psop_label->setObjectName(QString::fromUtf8("Psop_label"));
        Psop_label->setGeometry(QRect(0, 0, 100, 19));
        Psop_label->setFont(font);
        Psop_label->setAlignment(Qt::AlignCenter);
        Psop_textEdit = new QTextEdit(PSOP_frame);
        Psop_textEdit->setObjectName(QString::fromUtf8("Psop_textEdit"));
        Psop_textEdit->setGeometry(QRect(0, 20, 100, 51));
        Psop_textEdit->setFont(font1);
        Psop_textEdit->setReadOnly(true);
        Psop_textEdit->setAcceptRichText(false);
        Psop_minus_button = new QPushButton(PSOP_frame);
        Psop_minus_button->setObjectName(QString::fromUtf8("Psop_minus_button"));
        Psop_minus_button->setGeometry(QRect(0, 70, 50, 25));
        Psop_minus_button->setFont(font2);
        Psop_plus_button = new QPushButton(PSOP_frame);
        Psop_plus_button->setObjectName(QString::fromUtf8("Psop_plus_button"));
        Psop_plus_button->setGeometry(QRect(50, 70, 50, 25));
        Psop_plus_button->setFont(font2);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1200, 22));
        MainWindow->setMenuBar(menubar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        onOff_button->setText(QApplication::translate("MainWindow", "ON/OFF", nullptr));
        label->setText(QApplication::translate("MainWindow", "Serial port:", nullptr));
        pushButton_connect->setText(QApplication::translate("MainWindow", "Connect", nullptr));
        Fr_label->setText(QApplication::translate("MainWindow", "Fr", nullptr));
        Fr_textEdit->setPlaceholderText(QString());
        Fr_minus_button->setText(QApplication::translate("MainWindow", "-", nullptr));
        Fr_plus_button->setText(QApplication::translate("MainWindow", "+", nullptr));
        Etoi_label->setText(QApplication::translate("MainWindow", "I:E", nullptr));
        Etoi_textEdit->setPlaceholderText(QString());
        Etoi_minus_button->setText(QApplication::translate("MainWindow", "-", nullptr));
        Etoi_plus_button->setText(QApplication::translate("MainWindow", "+", nullptr));
        Pip_label->setText(QApplication::translate("MainWindow", "PIP / P lim", nullptr));
        Pip_textEdit->setPlaceholderText(QString());
        Pip_minus_button->setText(QApplication::translate("MainWindow", "-", nullptr));
        Pip_plus_button->setText(QApplication::translate("MainWindow", "+", nullptr));
        Vt_label->setText(QApplication::translate("MainWindow", "Vt", nullptr));
        Vt_textEdit->setPlaceholderText(QString());
        Vt_minus_button->setText(QApplication::translate("MainWindow", "-", nullptr));
        Vt_plus_button->setText(QApplication::translate("MainWindow", "+", nullptr));
        Peep_label->setText(QApplication::translate("MainWindow", "PEEP", nullptr));
        Peep_textEdit->setPlaceholderText(QString());
        Peep_minus_button->setText(QApplication::translate("MainWindow", "-", nullptr));
        Peep_plus_button->setText(QApplication::translate("MainWindow", "+", nullptr));
        Psop_label->setText(QApplication::translate("MainWindow", "Psupport", nullptr));
        Psop_textEdit->setPlaceholderText(QString());
        Psop_minus_button->setText(QApplication::translate("MainWindow", "-", nullptr));
        Psop_plus_button->setText(QApplication::translate("MainWindow", "+", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
