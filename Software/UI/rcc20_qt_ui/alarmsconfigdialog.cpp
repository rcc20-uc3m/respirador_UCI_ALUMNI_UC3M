#include "alarmsconfigdialog.h"
#include "ui_alarmsconfigdialog.h"

extern uint16_t P_alarmas[];


AlarmsConfigDialog::AlarmsConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlarmsConfigDialog)
{
    ui->setupUi(this);

    Pmax = P_alarmas[0];
    DeltaPeep = P_alarmas[1];
    DeltaVt = P_alarmas[2];
    MV_max = P_alarmas[3];
    MV_min = P_alarmas[4];
    Pip_Desc = P_alarmas[5];
    N_Desc = P_alarmas[6];
    DisplayParametros();
}

AlarmsConfigDialog::~AlarmsConfigDialog()
{
    delete ui;
}

void AlarmsConfigDialog::leeParametros(){
    Pmax = P_alarmas[0];
    DeltaPeep = P_alarmas[1];
    DeltaVt = P_alarmas[2];
    MV_max = P_alarmas[3];
    MV_min = P_alarmas[4];
    Pip_Desc = P_alarmas[5];
    N_Desc = P_alarmas[6];
    DisplayParametros();
}

void AlarmsConfigDialog::DisplayParametros(){
    ui->PMAX_textEdit->setPlainText(QString::number(Pmax/10.0, 'f', 1));
    ui->PMAX_textEdit->setAlignment(Qt::AlignCenter);

    ui->DELTAPEEP_textEdit->setPlainText(QString::number(DeltaPeep));
    ui->DELTAPEEP_textEdit->setAlignment(Qt::AlignCenter);

    ui->DELTAVT_textEdit->setPlainText(QString::number(DeltaVt));
    ui->DELTAVT_textEdit->setAlignment(Qt::AlignCenter);

    ui->MVMIN_textEdit->setPlainText(QString::number(MV_min/1000.0, 'f', 1));
    ui->MVMIN_textEdit->setAlignment(Qt::AlignCenter);

    ui->MVMAX_textEdit->setPlainText(QString::number(MV_max/1000.0, 'f', 1));
    ui->MVMAX_textEdit->setAlignment(Qt::AlignCenter);

    ui->PIPDESC_textEdit->setPlainText(QString::number(Pip_Desc/10.0, 'f', 1));
    ui->PIPDESC_textEdit->setAlignment(Qt::AlignCenter);

    ui->NDESC_textEdit->setPlainText(QString::number(N_Desc));
    ui->NDESC_textEdit->setAlignment(Qt::AlignCenter);
}

void AlarmsConfigDialog::on_PMAX_minus_button_clicked()
{
    uint16_t tmp = Pmax;
    tmp -= 50;
    if (tmp < 350) Pmax = 350;
    else Pmax = tmp;
    ui->PMAX_textEdit->setPlainText(QString::number(Pmax/10.0, 'f', 1));
    ui->PMAX_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_PMAX_plus_button_clicked()
{
    uint16_t tmp = Pmax;
    tmp += 50;
    if (tmp > 1400) Pmax = 1400;
    else Pmax = tmp;
    ui->PMAX_textEdit->setPlainText(QString::number(Pmax/10.0, 'f', 1));
    ui->PMAX_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_DELTAPEEP_minus_button_clicked()
{
    uint16_t tmp = DeltaPeep;
    tmp -= 5;
    if (tmp < 10) DeltaPeep = 10;
    else DeltaPeep = tmp;
    ui->DELTAPEEP_textEdit->setPlainText(QString::number(DeltaPeep));
    ui->DELTAPEEP_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_DELTAPEEP_plus_button_clicked()
{
    uint16_t tmp = DeltaPeep;
    tmp += 5;
    if (tmp > 90) DeltaPeep = 90;
    else DeltaPeep = tmp;
    ui->DELTAPEEP_textEdit->setPlainText(QString::number(DeltaPeep));
    ui->DELTAPEEP_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_DELTAVT_minus_button_clicked()
{
    uint16_t tmp = DeltaVt;
    tmp -= 5;
    if (tmp < 10) DeltaVt = 10;
    else DeltaVt = tmp;
    ui->DELTAVT_textEdit->setPlainText(QString::number(DeltaVt));
    ui->DELTAVT_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_DELTAVT_plus_button_clicked()
{
    uint16_t tmp = DeltaVt;
    tmp += 5;
    if (tmp > 90) DeltaVt = 90;
    else DeltaVt = tmp;
    ui->DELTAVT_textEdit->setPlainText(QString::number(DeltaVt));
    ui->DELTAVT_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_MVMIN_minus_button_clicked()
{
    uint16_t tmp = MV_min;
    tmp -=100;
    if (tmp < 2000) MV_min = 2000;
    else MV_min = tmp;
    ui->MVMIN_textEdit->setPlainText(QString::number(MV_min/1000.0, 'f', 1));
    ui->MVMIN_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_MVMIN_plus_button_clicked()
{
    uint16_t tmp = MV_min;
    tmp +=100;
    if (tmp > 5000) MV_min = 5000;
    else MV_min = tmp;
    ui->MVMIN_textEdit->setPlainText(QString::number(MV_min/1000.0, 'f', 1));
    ui->MVMIN_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_MVMAX_minus_button_clicked()
{
    uint16_t tmp = MV_max;
    tmp -=100;
    if (tmp < 6000) MV_max = 6000;
    else MV_max = tmp;
    ui->MVMAX_textEdit->setPlainText(QString::number(MV_max/1000.0, 'f', 1));
    ui->MVMAX_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_MVMAX_plus_button_clicked()
{
    uint16_t tmp = MV_max;
    tmp +=100;
    if (tmp > 15000) MV_max = 15000;
    else MV_max = tmp;
    ui->MVMAX_textEdit->setPlainText(QString::number(MV_max/1000.0, 'f', 1));
    ui->MVMAX_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_PIPDESC_minus_button_clicked()
{
    uint16_t tmp = Pip_Desc;
    tmp -=5;
    if (tmp < 20) Pip_Desc = 20;
    else Pip_Desc = tmp;
    ui->PIPDESC_textEdit->setPlainText(QString::number(Pip_Desc/10.0, 'f', 1));
    ui->PIPDESC_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_PIPDESC_plus_button_clicked()
{
    uint16_t tmp = Pip_Desc;
    tmp +=5;
    if (tmp > 120) Pip_Desc = 120;
    else Pip_Desc = tmp;
    ui->PIPDESC_textEdit->setPlainText(QString::number(Pip_Desc/10.0, 'f', 1));
    ui->PIPDESC_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_NDESC_minus_button_clicked()
{
    uint16_t tmp = N_Desc;
    tmp -=1;
    if (tmp < 1) N_Desc = 1;
    else N_Desc = tmp;
    ui->NDESC_textEdit->setPlainText(QString::number(N_Desc));
    ui->NDESC_textEdit->setAlignment(Qt::AlignCenter);
    emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}

void AlarmsConfigDialog::on_NDESC_plus_button_clicked()
{
    uint16_t tmp = N_Desc;
    tmp +=1;
    if (tmp > 5) N_Desc = 5;
    else N_Desc = tmp;
    ui->NDESC_textEdit->setPlainText(QString::number(N_Desc));
    ui->NDESC_textEdit->setAlignment(Qt::AlignCenter);
   emit AlarmParamsReady(Pmax, DeltaPeep, DeltaVt, MV_max, MV_min, Pip_Desc, N_Desc);
}
