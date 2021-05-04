#include "configdialog.h"
#include "ui_configdialog.h"
#include "utilidades.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
//    QObject::connect(parent, SIGNAL(calibrationDataToConfig(QByteArray, quint16)),
//                     this, SLOT(setValues(QByteArray, qint16)));
    InspPos = 0;
    EspPos = 0;
    PressVal = 0;
    FIVal = 0;
    FEVal = 0;
    displayValues();

}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::displayValues(){
    ui->PConfig_value->setText(QString::number(PressVal));
    ui->FIConfig_value->setText(QString::number(FIVal));
    ui->FEConfig_value->setText(QString::number(FEVal));
    ui->VInsp_textEdit->setPlainText(QString::number(InspPos));
    ui->VEsp_textEdit->setPlainText(QString::number(EspPos));
}

void ConfigDialog::setValues(QByteArray buffer, qint16 bytes_received){
    PressCAL = getuint16fromint8(buffer.at(1), buffer.at(2));
    FICAL = getuint16fromint8(buffer.at(3), buffer.at(4));
    FECAL = getuint16fromint8(buffer.at(5), buffer.at(6));
    PressVal = getuint16fromint8(buffer.at(7), buffer.at(8));
    FIVal = getuint16fromint8(buffer.at(9), buffer.at(10));
    FEVal = getuint16fromint8(buffer.at(11), buffer.at(12));

    ui->PConfig_value->setText(QString::number(PressCAL));
    ui->FIConfig_value->setText(QString::number(FICAL));
    ui->FEConfig_value->setText(QString::number(FECAL));
    ui->PCal_value->setText(QString::number(PressVal));
    ui->FICal_value->setText(QString::number(FIVal));
    ui->FECal_value->setText(QString::number(FEVal));
}

void ConfigDialog::on_VInsp_minus_button_clicked()
{
    int16_t temp =  InspPos;
    temp -= 5;
    if (temp < 0)
        InspPos = 0;
    else
        InspPos= temp;
    //ui->VInsp_textEdit->setPlainText("---");
    ui->VInsp_textEdit->setPlainText(QString::number(InspPos));
    ui->VInsp_textEdit->setAlignment(Qt::AlignCenter);
    emit setValvPos(InspPos, EspPos);
}

void ConfigDialog::on_VInsp_plus_button_clicked()
{
    int16_t temp =  InspPos;
    temp += 5;
    if (temp > 255)
        InspPos = 255;
    else
        InspPos= temp;
    //ui->VInsp_textEdit->setPlainText("---");
    ui->VInsp_textEdit->setPlainText(QString::number(InspPos));
    ui->VInsp_textEdit->setAlignment(Qt::AlignCenter);
    emit setValvPos(InspPos, EspPos);
}

void ConfigDialog::on_VEsp_minus_button_clicked()
{
    int16_t temp =  EspPos;
    temp -= 5;
    if (temp < 0)
        EspPos = 0;
    else
        EspPos= temp;
    //ui->VEsp_textEdit->setPlainText("---");
    ui->VEsp_textEdit->setPlainText(QString::number(EspPos));
    ui->VEsp_textEdit->setAlignment(Qt::AlignCenter);
    emit setValvPos(InspPos, EspPos);
}

void ConfigDialog::on_VEsp_plus_button_clicked()
{
    int16_t temp =  EspPos;
    temp += 5;
    if (temp > 255)
        EspPos = 255;
    else
        EspPos= temp;
    //ui->VEsp_textEdit->setPlainText("---");
    ui->VEsp_textEdit->setPlainText(QString::number(EspPos));
    ui->VEsp_textEdit->setAlignment(Qt::AlignCenter);
    emit setValvPos(InspPos, EspPos);
}
