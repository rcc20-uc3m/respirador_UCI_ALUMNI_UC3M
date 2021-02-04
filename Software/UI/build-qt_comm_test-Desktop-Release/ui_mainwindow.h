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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QComboBox *serialPortDropdown;
    QPushButton *pushButton_connect;
    QPlainTextEdit *plainTextEdit_input_hex;
    QPlainTextEdit *plainTextEdit_show_frames;
    QCustomPlot *pressPlot;
    QCustomPlot *flowPlot;
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
        horizontalLayoutWidget->setGeometry(QRect(0, 0, 791, 71));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
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

        plainTextEdit_input_hex = new QPlainTextEdit(centralwidget);
        plainTextEdit_input_hex->setObjectName(QString::fromUtf8("plainTextEdit_input_hex"));
        plainTextEdit_input_hex->setGeometry(QRect(720, 80, 71, 31));
        plainTextEdit_input_hex->setMinimumSize(QSize(71, 0));
        plainTextEdit_input_hex->setMaximumSize(QSize(50, 16777215));
        plainTextEdit_input_hex->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        plainTextEdit_show_frames = new QPlainTextEdit(centralwidget);
        plainTextEdit_show_frames->setObjectName(QString::fromUtf8("plainTextEdit_show_frames"));
        plainTextEdit_show_frames->setGeometry(QRect(650, 110, 151, 401));
        pressPlot = new QCustomPlot(centralwidget);
        pressPlot->setObjectName(QString::fromUtf8("pressPlot"));
        pressPlot->setGeometry(QRect(10, 80, 631, 221));
        flowPlot = new QCustomPlot(centralwidget);
        flowPlot->setObjectName(QString::fromUtf8("flowPlot"));
        flowPlot->setGeometry(QRect(10, 300, 631, 221));
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
        label->setText(QApplication::translate("MainWindow", "Serial port:", nullptr));
        pushButton_connect->setText(QApplication::translate("MainWindow", "Connect", nullptr));
        plainTextEdit_input_hex->setPlainText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
