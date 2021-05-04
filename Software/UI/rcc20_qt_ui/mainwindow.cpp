#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <configdialog.h>
#include <alarmsconfigdialog.h>
#include "hdlc_qt.h"
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <parametros.h>
#include <utilidades.h>
#include <calibracion.h>
#include <alarms.h>
#include <QString>
#include <QFlags>

extern CALIBRACION miCalibracion;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("Pulse botón ON para iniciar el RCC-20", 3000);
    serial = new QSerialPort(this);
    this->fillPortsInfo();

    HDLC_qt* hdlc = HDLC_qt::instance();

    confDlg = new ConfigDialog(this);
    confDlg->setModal(true);

    alarmDlg = new AlarmsConfigDialog(this);
    alarmDlg->setModal(true);

    miSndState = new SndState(ui->alarmaMensaje);

    QObject::connect(alarmDlg, SIGNAL(AlarmParamsReady(quint16, quint16, quint16, quint16, quint16, quint16, quint16)),
                     SLOT(send_alarms_param(quint16, quint16, quint16, quint16, quint16, quint16, quint16)));

    QObject::connect(this, SIGNAL(alarmaRecibida(quint16)),
                     miSndState, SLOT(checkNewSndState(quint16)));

    QObject::connect(miSndState, SIGNAL(setSendNewAlarmSnd(quint8)),
                     this, SLOT(sendAlarmSnd(quint8)));

    QObject::connect(hdlc, SIGNAL(hdlcTransmitByte(QByteArray)),
                     this, SLOT(putChar(QByteArray)));

    QObject::connect(hdlc, SIGNAL(hdlcTransmitByte(char)),
                     this, SLOT(putChar(char)));

    QObject::connect(hdlc, SIGNAL(hdlcValidFrameReceived(QByteArray,quint16)),
                     this, SLOT(HDLC_CommandRouter(QByteArray,quint16)));

    // All data coming from serial port (should go straight to HDLC validation)
    QObject::connect(this, SIGNAL(dataReceived(QByteArray)),
                     hdlc, SLOT(charReceiver(QByteArray)));


    QObject::connect(this, SIGNAL(sendDataFrame(QByteArray,quint16)),
                     hdlc, SLOT(frameDecode(QByteArray,quint16)));

    QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                     this, SLOT(openSerialPort()));

    QObject::connect(serial, SIGNAL(readyRead()),
                     this, SLOT(readData()));

    QObject::connect(ui->marchaParoButton, SIGNAL(clicked()), this, SLOT(marchaParo()));

    QObject::connect(confDlg, SIGNAL(setValvPos(quint16, quint16)), this, SLOT(sendValvPos(quint16, quint16)));
    QObject::connect(this, &MainWindow::calibrationDataToConfig, confDlg, &ConfigDialog::setValues);



    this->ptr = 0;

    MakeCanvas();
    init_parametros();
    init_gui();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::send_alarms_param(quint16 a, quint16 b, quint16 c, quint16 d, quint16 e, quint16 f, quint16 g){
    P_alarmas[0] = a;
    P_alarmas[1] = b;
    P_alarmas[2] = c;
    P_alarmas[3] = d;
    P_alarmas[4] = e;
    P_alarmas[5] = f;
    P_alarmas[6] = g;

    QByteArray data;
    data.append((uint8_t) FrameType::SEND_PALARM);
    for (int i = 0; i < (int) PARAM_ALARM::Count; i++){
        data.append(get_lsb(P_alarmas[i]));
        data.append(get_msb(P_alarmas[i]));
    }
    emit sendDataFrame(data, (quint16) data.size());
}


void MainWindow::init_controller(){
    QByteArray data;
    miEstado.estado = MachineState::RESET;
    miEstado.modo = MODOS::PC_CMV;
    miEstado.disparo = TRIGGER::Presion;
    data.append((uint8_t) FrameType::ASK_STATE);
    sendData(data);
}

void MainWindow::set_params_ciclo_Text(sendDataCycleStruct mdat){
     ui->PpicoShow_value->setText(QString::number(mdat.Ppico/10.0, 'f', 1));
     ui->Pplat_value->setText(QString::number(mdat.Pplateau/10.0, 'f', 1));
     ui->VtShow_value->setText(QString::number(mdat.Vinsp));
     ui->PeepShow_value->setText(QString::number(mdat.peepM/10.0, 'f', 1));
     ui->FIMaxShow_value->setText(QString::number(mdat.FImax/10.0));
     ui->FEMaxShow_value->setText(QString::number(mdat.FEmax/10.0));
     ui->CompShow_value->setText(QString::number(mdat.Comp/10.0, 'f', 1));
     ui->ResShow_value->setText(QString::number(mdat.Res/10.0, 'f', 1));
     ui->CdinShow_value->setText(QString::number(mdat.Vinsp/(float)(mdat.Ppico-mdat.peepM)*10.0, 'f', 1));
     ui->CestShow_value->setText(QString::number(mdat.Vinsp/(float)(mdat.Pplateau-mdat.peepM)*10.0, 'f', 1));
     ui->MinVolShow_value->setText(QString::number(mdat.MinVol/1000.0, 'f', 1));     
}


void MainWindow::setPropTpText(qint16 val){
    ui->propTp_textEdit->setPlainText(QString::number(val));
    ui->propTp_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setTrigLvlPressText(qint16 val){
    ui->ThresLvlPress_textEdit->setPlainText(QString::number((float) val));
    ui->ThresLvlPress_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setFrText(quint16 val)
{
    ui->Fr_textEdit->setPlainText(QString::number(val));
    ui->Fr_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setEtoiText(quint16 val)
{
    ui->Etoi_textEdit->setPlainText(Etoi_labels.at(val) );
    ui->Etoi_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setPipText(quint16 val)
{
    ui->Pip_textEdit->setPlainText(QString::number(val/10.0, 'f', 1));
    ui->Pip_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setVtText(quint16 val)
{
    ui->Vt_textEdit->setPlainText(QString::number(val));
    ui->Vt_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setPeepText(quint16 val)
{
    ui->Peep_textEdit->setPlainText(QString::number((float) val/10.0,'f', 1   ));
    ui->Peep_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::setPsopText(quint16 val)
{
    ui->Psop_textEdit->setPlainText(QString::number(val/10.0, 'f', 1));
    ui->Psop_textEdit->setAlignment(Qt::AlignCenter);
}

void MainWindow::activate_gui(){
    ui->pushButton_calibracion->setEnabled(true);
    ui->Fr_frame->setEnabled(true);
    ui->ETOI_frame->setEnabled(true);
    ui->PIP_frame->setEnabled(true);
    ui->VT_frame->setEnabled(true);
    ui->PEEP_frame->setEnabled(true);
    ui->PSOP_frame->setEnabled(true);
    ui->pressPlot->setEnabled(true);
    ui->flowPlot->setEnabled(true);
    ui->marchaParoButton->setEnabled(true);
    ui->PpicoShow_frame->setEnabled(true);
    ui->Pplat_frame->setEnabled(true);
    ui->VtShow_frame->setEnabled(true);
    ui->PeepShow_frame->setEnabled(true);
    ui->CompShow_frame->setEnabled(true);
    ui->ResShow_frame->setEnabled(true);
    ui->FIMaxShow_frame->setEnabled(true);
    ui->FEMaxShow_frame->setEnabled(true);
    ui->CdinShow_frame->setEnabled(true);
    ui->CestShow_frame->setEnabled(true);
    ui->Modos_frame->setEnabled(true);
    ui->config_frame->setEnabled(true);
    ui->PCCMV->setStyleSheet(modoInactivo);
    ui->PCAC->setStyleSheet(modoInactivo);
    ui->PCSIMV->setStyleSheet(modoInactivo);
    ui->VCCMV->setStyleSheet(modoInactivo);
    ui->VCAC->setStyleSheet(modoInactivo);
    ui->VCSIMV->setStyleSheet(modoInactivo);
    ui->configAlarms->setEnabled(true);
    ui->silencioAlarms->setEnabled(true);
    ui->MinVolShow_frame->setEnabled(true);
    ui->FIO2Show_frame->setEnabled(true);
}

void MainWindow::deactivate_gui(){
    ui->pushButton_calibracion->setEnabled(false);
    ui->Fr_frame->setEnabled(false);
    ui->ETOI_frame->setEnabled(false);
    ui->PIP_frame->setEnabled(false);
    ui->VT_frame->setEnabled(false);
    ui->PEEP_frame->setEnabled(false);
    ui->PSOP_frame->setEnabled(false);
    ui->pressPlot->setEnabled(false);
    ui->flowPlot->setEnabled(false);
    ui->marchaParoButton->setEnabled(false);
    ui->PpicoShow_frame->setEnabled(false);
    ui->Pplat_frame->setEnabled(false);
    ui->VtShow_frame->setEnabled(false);
    ui->PeepShow_frame->setEnabled(false);
    ui->CompShow_frame->setEnabled(false);
    ui->ResShow_frame->setEnabled(false);
    ui->FIMaxShow_frame->setEnabled(false);
    ui->FEMaxShow_frame->setEnabled(false);
    ui->CdinShow_frame->setEnabled(false);
    ui->CestShow_frame->setEnabled(false);
    ui->Modos_frame->setEnabled(false);
    ui->config_frame->setEnabled(false);
    ui->PCCMV->setStyleSheet(modoDesactivado);
    ui->PCAC->setStyleSheet(modoDesactivado);
    ui->PCSIMV->setStyleSheet(modoDesactivado);
    ui->VCCMV->setStyleSheet(modoDesactivado);
    ui->VCAC->setStyleSheet(modoDesactivado);
    ui->VCSIMV->setStyleSheet(modoDesactivado);
    ui->configAlarms->setEnabled(false);
    ui->silencioAlarms->setEnabled(false);
    ui->MinVolShow_frame->setEnabled(false);
    ui->FIO2Show_frame->setEnabled(false);
}

void MainWindow::init_gui()
{
    deactivate_gui();
    setFrText(P_Operacion[(int) PARAM_OPERACION::Fr]);
    setEtoiText(P_Operacion[(int) PARAM_OPERACION::iex10]);
    setPipText(P_Operacion[(int) PARAM_OPERACION::Pip]);
    setVtText(P_Operacion[(int) PARAM_OPERACION::Vt]);
    setPeepText(P_Operacion[(int) PARAM_OPERACION::Peep]);
    setPsopText(P_Operacion[(int) PARAM_OPERACION::Psop]);
    setPropTpText(P_config[(int) PARAM_CONFIG::porTp]);
    setTrigLvlPressText(P_config[(int) PARAM_CONFIG::TrigLvl_Press]);
//    setTrigLvlFlowText(P_config[(int) PARAM_CONFIG::TrigLvl_Flow]);
}

void MainWindow::init_parametros(){
    P_Operacion[(int) PARAM_OPERACION::Fr] = FR_DEFAULT;
    P_Operacion[(int) PARAM_OPERACION::iex10] = IEX10_DEFAULT;
    P_Operacion[(int) PARAM_OPERACION::Pip] = PIP_DEFAULT;
    P_Operacion[(int) PARAM_OPERACION::Psop] = PSOP_DEFAULT;
    P_Operacion[(int) PARAM_OPERACION::Peep] = PEEP_DEFAULT;
    P_Operacion[(int) PARAM_OPERACION::Vt] = VT_DEFAULT;
    P_Operacion[(int) PARAM_OPERACION::FiO2] = FIO2_DEFAULT;

    P_config[(int) PARAM_CONFIG::Tri] = TRI_DEFAULT;
    P_config[(int) PARAM_CONFIG::porTp] = PROPTPAUSA_DEFAULT;
    P_config[(int) PARAM_CONFIG::Pm] = PM_DEFAULT;
    P_config[(int) PARAM_CONFIG::N] = N_DEFAULT;
    P_config[(int) PARAM_CONFIG::Pc] = PC_DEFAULT;
    P_config[(int) PARAM_CONFIG::Palarm] = PALARM_DEFAULT;
    P_config[(int) PARAM_CONFIG::Valarm] = VALARM_DEFAULT;
    P_config[(int) PARAM_CONFIG::FlowChange] = FLOW_DEFAULT;
    P_config[(int) PARAM_CONFIG::Trig_Wnd] = TRIGWND_DEFAULT;
    P_config[(int) PARAM_CONFIG::Trig_Mnd] = TRIGMND_DEFAULT;
    P_config[(int) PARAM_CONFIG::TrigLvl_Press] = TRIGLVLPRESS_DEFAULT;
    P_config[(int) PARAM_CONFIG::TrigLvl_Flow] = TRIGLVLFLOW_DEFAULT;
    P_config[(int) PARAM_CONFIG::C0] = C0_DEFAULT;
    P_config[(int) PARAM_CONFIG::R0] = R0_DEFAULT;
    P_config[(int) PARAM_CONFIG::Trigger_Flow] = TRIGGER_FLOW;
    P_config[(int) PARAM_CONFIG::newAlgorithm] = NEWALGORITHM;
    if (P_config[(int) PARAM_CONFIG::newAlgorithm]) ui->newExpiration->setChecked(true);

    P_hardware[(int) PARAM_HARDW::ps_eps] = PS_EPS;
    P_hardware[(int) PARAM_HARDW::ps_inc] = PS_INC;
    P_hardware[(int) PARAM_HARDW::ps_inc_timer] = PS_INC_TIMER;
    P_hardware[(int) PARAM_HARDW::ps_pordebajodepip] = PS_PORDEBAJOPIP;
    P_hardware[(int) PARAM_HARDW::ps_peep_menos] = PS_PEEP_MENOS;
    P_hardware[(int) PARAM_HARDW::ps_caida_timer] = PS_CAIDA_TIMER;
    P_hardware[(int) PARAM_HARDW::ps_subida_timer] = PS_SUBIDA_TIMER;
    P_hardware[(int) PARAM_HARDW::ps_peep_close_min] = PS_PEEP_CLOSE_MIN;
    P_hardware[(int) PARAM_HARDW::espmin] = ESPMIN;
    P_hardware[(int) PARAM_HARDW::espmaxN] = ESPMAXN;
    P_hardware[(int) PARAM_HARDW::espmaxF0] = ESPMAXF0;
    P_hardware[(int) PARAM_HARDW::espmaxF1] = ESPMAXF1;
    QFile file("espiracionPID.txt");
    QMessageBox msgBox;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        msgBox.setText("No he encontrado valores PID espiración.\nUsando valores por defecto");
        msgBox.exec();
        P_hardware[(int) PARAM_HARDW::espKp1] = ESPKP1;
        P_hardware[(int) PARAM_HARDW::espKi1] = ESPKI1;
        P_hardware[(int) PARAM_HARDW::espKd1] = ESPKD1;
        P_hardware[(int) PARAM_HARDW::espKp2] = ESPKP2;
        P_hardware[(int) PARAM_HARDW::espKi2] = ESPKI2;
        P_hardware[(int) PARAM_HARDW::espKd2] = ESPKD2;
        P_hardware[(int) PARAM_HARDW::espKp3] = ESPKP3;
        P_hardware[(int) PARAM_HARDW::espKi3] = ESPKI3;
        P_hardware[(int) PARAM_HARDW::espKd3] = ESPKD3;
     } else {
        QTextStream in(&file);
        QString line;
        QStringList list1;
        line = in.readLine();
        list1 = line.split(QLatin1Char('|'));
        P_hardware[(int) PARAM_HARDW::espKp1] = list1.at(0).toInt();
        P_hardware[(int) PARAM_HARDW::espKi1] = list1.at(1).toInt();;
        P_hardware[(int) PARAM_HARDW::espKd1] = list1.at(2).toInt();;

        line = in.readLine();
        list1 = line.split(QLatin1Char('|'));
        P_hardware[(int) PARAM_HARDW::espKp2] = list1.at(0).toInt();
        P_hardware[(int) PARAM_HARDW::espKi2] = list1.at(1).toInt();;
        P_hardware[(int) PARAM_HARDW::espKd2] = list1.at(2).toInt();;

        line = in.readLine();
        list1 = line.split(QLatin1Char('|'));
        P_hardware[(int) PARAM_HARDW::espKp3] = list1.at(0).toInt();
        P_hardware[(int) PARAM_HARDW::espKi3] = list1.at(1).toInt();;
        P_hardware[(int) PARAM_HARDW::espKd3] = list1.at(2).toInt();;
//        QString mens = "Valores obtenidos:\n";
//        mens += QString::number(P_hardware[(int) PARAM_HARDW::espKp1])+ " " + QString::number(P_hardware[(int) PARAM_HARDW::espKi1])
//                + " " + QString::number(P_hardware[(int) PARAM_HARDW::espKd1])+"\n";
//        mens += QString::number(P_hardware[(int) PARAM_HARDW::espKp2])+ " " + QString::number(P_hardware[(int) PARAM_HARDW::espKi2])
//                + " " + QString::number(P_hardware[(int) PARAM_HARDW::espKd2])+"\n";
//        mens += QString::number(P_hardware[(int) PARAM_HARDW::espKp3])+ " " + QString::number(P_hardware[(int) PARAM_HARDW::espKi3])
//                + " " + QString::number(P_hardware[(int) PARAM_HARDW::espKd3]);
//        msgBox.setText(mens);
//        msgBox.exec();
    }

    P_hardware[(int) PARAM_HARDW::insmin] = INSMIN ;
    P_hardware[(int) PARAM_HARDW::insmax] = INSMAX;
    P_hardware[(int) PARAM_HARDW::pressKpF] = PRESSKPF;
    P_hardware[(int) PARAM_HARDW::pressKiF] = PRESSKIF;
    P_hardware[(int) PARAM_HARDW::pressKdF] = PRESSKDF;
    P_hardware[(int) PARAM_HARDW::pressKpS] = PRESSKPS;
    P_hardware[(int) PARAM_HARDW::pressKiS] = PRESSKIS;
    P_hardware[(int) PARAM_HARDW::pressKdS] = PRESSKDS;
    P_hardware[(int) PARAM_HARDW::volKpF] = VOLKPF;
    P_hardware[(int) PARAM_HARDW::volKiF] = VOLKIF;
    P_hardware[(int) PARAM_HARDW::volKdF] = VOLKDF;
    P_hardware[(int) PARAM_HARDW::volKpS] = VOLKPS;
    P_hardware[(int) PARAM_HARDW::volKiS] = VOLKIS;
    P_hardware[(int) PARAM_HARDW::volKdS] = VOLKDS;
    P_hardware[(int) PARAM_HARDW::ps_inc_peep_close] = PS_INC_PEEP_CLOSE;
    P_hardware[(int) PARAM_HARDW::ps_cambio_pid] = PS_CAMBIO_PID;
    P_hardware[(int) PARAM_HARDW::ps_limite_rcbajo] = PS_LIMITE_RCBAJO;

    P_alarmas[(int)PARAM_ALARM::ps_max] = 600;
    P_alarmas[(int)PARAM_ALARM::delta_peep] = 20;
    P_alarmas[(int)PARAM_ALARM::delta_vt] = 20;
    P_alarmas[(int)PARAM_ALARM::mv_max] = 10000;
    P_alarmas[(int)PARAM_ALARM::mv_min] = 4000;
    P_alarmas[(int)PARAM_ALARM::pip_disconnect] = 50;
    P_alarmas[(int)PARAM_ALARM::n_disconnect] = 3;

    alarmDlg->leeParametros();
}

void MainWindow::marchaParo(){
    struct estadoMaquinaStruct tmp;
    if (miEstado.estado == MachineState::OFF) {
        tmp = {MachineState::ON, miEstado.modo, miEstado.disparo};
        sendState(tmp);
        //ui->Modos_frame->setEnabled(false);
        //ui->config_frame->setEnabled(false);
        ui->pushButton_calibracion->setEnabled(false);
        ui->pushButton_connect->setEnabled(false);
        ui->configAlarms->setEnabled(false);
    } else if (miEstado.estado == MachineState::ON) {
        tmp = {MachineState::OFF, miEstado.modo, miEstado.disparo};
        sendState(tmp);
        //ui->Modos_frame->setEnabled(true);
        //ui->config_frame->setEnabled(true);
        ui->pushButton_calibracion->setEnabled(true);
        ui->pushButton_connect->setEnabled(true);
        ui->configAlarms->setEnabled(true);

    } else if (miEstado.estado == MachineState::CAL){
        return;
    }
}


void MainWindow::on_Fr_plus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Fr];
    temp += Fr_inc;
    if (temp > Fr_max) return;
    P_Operacion[(int) PARAM_OPERACION::Fr]= temp;
    ui->Fr_textEdit->setPlainText("---");
    ui->Fr_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Fr, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Fr_minus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Fr];
    temp -= Fr_inc;
    if (temp < Fr_min) return;
    P_Operacion[(int) PARAM_OPERACION::Fr] = temp;
    ui->Fr_textEdit->setPlainText("---");
    ui->Fr_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Fr, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Psop_plus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Psop];
    temp += Psop_inc;
    if (temp > Psop_max) return;
    P_Operacion[(int) PARAM_OPERACION::Psop]= temp;
    ui->Psop_textEdit->setPlainText("---");
    ui->Psop_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Psop, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Psop_minus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Psop];
    temp -= Psop_inc;
    if (temp < Psop_min || temp < P_Operacion[(int)PARAM_OPERACION::Peep]) return;
    P_Operacion[(int) PARAM_OPERACION::Psop]= temp;
    ui->Psop_textEdit->setPlainText("---");
    ui->Psop_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Psop, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Peep_plus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Peep];
    if (temp == 0) temp += 50;
    else temp += Peep_inc;
    if (temp > Peep_max) return;
    P_Operacion[(int) PARAM_OPERACION::Peep]= temp;
    ui->Peep_textEdit->setPlainText("---");
    ui->Peep_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Peep, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Peep_minus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Peep];
    temp -= Peep_inc;
    if (temp < 50) temp = 0;
    if (temp < Peep_min) return;
    P_Operacion[(int) PARAM_OPERACION::Peep]= temp;
    ui->Peep_textEdit->setPlainText("---");
    ui->Peep_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Peep, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Vt_plus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Vt];
    temp += Vt_inc;
    if (temp > Vt_max) return;
    P_Operacion[(int) PARAM_OPERACION::Vt]= temp;
    ui->Vt_textEdit->setPlainText("---");
    ui->Vt_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Vt, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Vt_minus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Vt];
    temp -= Vt_inc;
    if (temp < Vt_min) return;
    P_Operacion[(int) PARAM_OPERACION::Vt]= temp;
    ui->Vt_textEdit->setPlainText("---");
    ui->Vt_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Vt, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Pip_plus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Pip];
    temp += Pip_inc;
    if (temp > Pip_max) return;
    P_Operacion[(int) PARAM_OPERACION::Pip]= temp;
    ui->Pip_textEdit->setPlainText("---");
    ui->Pip_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Pip, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Pip_minus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::Pip];
    temp -= Pip_inc;
    if (temp < Pip_min) return;
    P_Operacion[(int) PARAM_OPERACION::Pip]= temp;
    ui->Pip_textEdit->setPlainText("---");
    ui->Pip_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::Pip, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Etoi_plus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::iex10];
    temp += Etoi_inc;
    if (temp > Etoi_max) return;
    P_Operacion[(int) PARAM_OPERACION::iex10]= temp;
    ui->Etoi_textEdit->setPlainText("---");
    ui->Etoi_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::iex10, (uint16_t) Etoi_vals[temp]);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_Etoi_minus_button_clicked()
{
    int16_t temp =  P_Operacion[(int) PARAM_OPERACION::iex10];
    temp -= Etoi_inc;
    if (temp < Etoi_min) return;
    P_Operacion[(int) PARAM_OPERACION::iex10]= temp;
    ui->Etoi_textEdit->setPlainText("---");
    ui->Etoi_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_param_frame((uint8_t) PARAM_OPERACION::iex10, (uint16_t) Etoi_vals[temp]);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::putChar(QByteArray data)
{
    this->writeData(data);
}

void MainWindow::putChar(char data)
{
    QByteArray qdata(1, data);
    this->writeData(qdata);
}

void MainWindow::HDLC_CommandRouter(QByteArray buffer, quint16 bytes_received)
{
    enum FrameType command = static_cast<FrameType>(buffer.at(0));
    switch(command) {
    case FrameType::DATA: this->recibir_datos(buffer, bytes_received); break;
    case FrameType::DATACYCLE: this->recibir_datosciclo(buffer, bytes_received); break;
    case FrameType::PARAM_OK: this->confirm_param(buffer, bytes_received); break;
    case FrameType::ANS_STATE: this->set_state(buffer, bytes_received); break;
    case FrameType::PCONFIG_OK: this->send_params_operacion(); break;
    case FrameType::POPER_OK: this->send_params_alarmas(); break;
    case FrameType::PALARM_OK : this->start_machine(); break;
    case FrameType::STATE_OK: this->set_state(buffer, bytes_received);  break;
    case FrameType::CONFIG_OK: this->confirm_config(buffer, bytes_received); break;
    case FrameType::SENDDEBUG: this->display_debug(buffer, bytes_received); break;
    case FrameType::CAL_PARAM_OK: this->calibrationParamOk(buffer, bytes_received); break;
    case FrameType::CAL_DATA: this->calibrationDataReceived(buffer, bytes_received); break;
    case FrameType::HW_OK: this->send_params_config(); break;
    case FrameType::CAL_POINTS_OK: this->send_params_hardware(); break;
    case FrameType::DATA_OK:
    case FrameType::DATACYCLE_OK:
    case FrameType::ASK_STATE:
    case FrameType::SEND_POPER:
    case FrameType::SEND_STATE:
    case FrameType::SENDCONFIG:
    case FrameType::CAL_PARAM:
    case FrameType::SEND_HW:
    case FrameType::SEND_CAL_POINTS:
    case FrameType::SET_ALARM:
    case FrameType::SENDPARAM:
    case FrameType::SEND_PCONFIG:
    case FrameType::SEND_PALARM:
    case FrameType::Count:
        break;
    }
}

void MainWindow::display_debug(QByteArray buffer, qint16 bytes_received){

    QString mensaje(buffer.mid(1, bytes_received-1));
     ui->pizarra->appendPlainText(mensaje);
}

void MainWindow::sendState(estadoMaquinaStruct estado)
{
    QByteArray data;
    data.append((uint8_t) FrameType::SEND_STATE);
    data.append((uint8_t) estado.estado);
    data.append((uint8_t) estado.modo);
    data.append((uint8_t) estado.disparo);
    sendData(data);
}

void MainWindow::start_machine(){
    activate_gui();
    QThread::msleep(200);
    estadoMaquinaStruct tmp = {MachineState::OFF, miEstado.modo, miEstado.disparo};
    sendState(tmp);
}

void MainWindow::send_params_hardware(void){
    QByteArray data;
    data.append((uint8_t) FrameType::SEND_HW);
    for (int i = 0; i < (int) PARAM_HARDW::Count; i++){
        //ui->pizarra->appendPlainText(QString::number(i)+" = " + QString::number(P_hardware[i]));
        data.append(get_lsb(P_hardware[i]));
        data.append(get_msb(P_hardware[i]));
    }
    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::send_params_config(void){
    QByteArray data;
    data.append((uint8_t) FrameType::SEND_PCONFIG);
    for (int i = 0; i < (int) PARAM_CONFIG::Count; i++){
        data.append(get_lsb(P_config[i]));
        data.append(get_msb(P_config[i]));
    }
    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::send_params_operacion(void){
    uint16_t tmp;
    QByteArray data;
    data.append((uint8_t) FrameType::SEND_POPER);
    for (int i = 0; i < (int) PARAM_OPERACION::Count; i++){
        if (i == (int) PARAM_OPERACION::iex10){
            tmp = Etoi_vals[P_Operacion[i]];
        } else{
            tmp = P_Operacion[i];
        }
        data.append(get_lsb(tmp));
        data.append(get_msb(tmp));
    }
    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::send_params_alarmas(void){
    QByteArray data;
    data.append((uint8_t) FrameType::SEND_PALARM);
    for (int i = 0; i < (int) PARAM_ALARM::Count; i++){
        data.append(get_lsb(P_alarmas[i]));
        data.append(get_msb(P_alarmas[i]));
    }
    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::send_puntos_calibracion(void){
    QByteArray data;
    data.append((uint8_t) FrameType::SEND_CAL_POINTS);
    data.append((uint8_t) miCalibracion.n_puntos);
    for (int i = 0; i < miCalibracion.n_puntos; i++){
        data.append(get_lsb(miCalibracion.XFI[i]));
        data.append(get_msb(miCalibracion.XFI[i]));
        data.append(get_lsb(miCalibracion.XFE[i]));
        data.append(get_msb(miCalibracion.XFE[i]));
        data.append(get_lsb(miCalibracion.YFX[i]));
        data.append(get_msb(miCalibracion.YFX[i]));
    }
    data.append(get_lsb(miCalibracion.zeroPress));
    data.append(get_msb(miCalibracion.zeroPress));
    data.append(get_lsb(miCalibracion.zeroFI));
    data.append(get_msb(miCalibracion.zeroFI));
    data.append(get_lsb(miCalibracion.zeroFE));
    data.append(get_msb(miCalibracion.zeroFE));
    data.append(get_lsb(miCalibracion.coeffPress));
    data.append(get_msb(miCalibracion.coeffPress));
    qDebug() << data.size();
    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::set_state(QByteArray buffer, qint16 bytes_received){
    miEstado.estado = (MachineState)buffer.at(1);
    miEstado.modo = (MODOS) buffer.at(2);
    miEstado.disparo = (TRIGGER) buffer.at(3);

//    if (miEstado.disparo == TRIGGER::Presion) ui->triggerModePressure->setChecked(true);
//    else if (miEstado.disparo == TRIGGER::Flujo) ui->triggerModeFlow->setChecked(true);

    switch(miEstado.modo){
        case MODOS::PC_CMV:{
            ui->PCCMV->setChecked(true);
            if (!ui->Modos_frame->isEnabled()){
                ui->PCAC->setStyleSheet(modoDesactivado);
                ui->PCSIMV->setStyleSheet(modoDesactivado);
                ui->VCCMV->setStyleSheet(modoDesactivado);
                ui->VCAC->setStyleSheet(modoDesactivado);
                ui->VCSIMV->setStyleSheet(modoDesactivado);
            } else {
                ui->PCAC->setStyleSheet(modoInactivo);
                ui->PCSIMV->setStyleSheet(modoInactivo);
                ui->VCCMV->setStyleSheet(modoInactivo);
                ui->VCAC->setStyleSheet(modoInactivo);
                ui->VCSIMV->setStyleSheet(modoInactivo);
            }
            ui->PCCMV->setStyleSheet(modoActivo);
            ui->propTp_frame->setEnabled(false);
//            ui->ThresLvlFlow_frame->setEnabled(false);
            ui->ThresLvlPress_frame->setEnabled(false);
//            ui->triggerMode->setEnabled(false);

            ui->PSOP_frame->setEnabled(false);
            ui->Vt_label->setText("Vt MAX");
            ui->Pip_label->setText("PIP");
            break;
        }
        case MODOS::PC_AC:{
            ui->PCAC->setChecked(true);
            if (!ui->Modos_frame->isEnabled()){
                ui->PCCMV->setStyleSheet(modoDesactivado);
                ui->PCSIMV->setStyleSheet(modoDesactivado);
                ui->VCCMV->setStyleSheet(modoDesactivado);
                ui->VCAC->setStyleSheet(modoDesactivado);
                ui->VCSIMV->setStyleSheet(modoDesactivado);
            }else {
                ui->PCCMV->setStyleSheet(modoInactivo);
                ui->PCSIMV->setStyleSheet(modoInactivo);
                ui->VCCMV->setStyleSheet(modoInactivo);
                ui->VCAC->setStyleSheet(modoInactivo);
                ui->VCSIMV->setStyleSheet(modoInactivo);
            }
            ui->PCAC->setStyleSheet(modoActivo);
            ui->propTp_frame->setEnabled(false);
//            ui->ThresLvlFlow_frame->setEnabled(true);
            ui->ThresLvlPress_frame->setEnabled(true);
//            ui->triggerMode->setEnabled(true);

            ui->PSOP_frame->setEnabled(false);
            ui->Vt_label->setText("Vt MAX");
            ui->Pip_label->setText("PIP");

            break;
        }
        case MODOS::PC_SIMV:{
            ui->PCSIMV->setChecked(true);
            if (!ui->Modos_frame->isEnabled()){
                ui->PCCMV->setStyleSheet(modoDesactivado);
                ui->PCAC->setStyleSheet(modoDesactivado);
                ui->VCCMV->setStyleSheet(modoDesactivado);
                ui->VCAC->setStyleSheet(modoDesactivado);
                ui->VCSIMV->setStyleSheet(modoDesactivado);
            }else {
                ui->PCCMV->setStyleSheet(modoInactivo);
                ui->PCAC->setStyleSheet(modoInactivo);
                ui->VCCMV->setStyleSheet(modoInactivo);
                ui->VCAC->setStyleSheet(modoInactivo);
                ui->VCSIMV->setStyleSheet(modoInactivo);
            }
            ui->PCSIMV->setStyleSheet(modoActivo);
            ui->propTp_frame->setEnabled(false);
//            ui->ThresLvlFlow_frame->setEnabled(true);
            ui->ThresLvlPress_frame->setEnabled(true);
//            ui->triggerMode->setEnabled(true);

            ui->PSOP_frame->setEnabled(true);
            ui->Vt_label->setText("Vt MAX");
            ui->Pip_label->setText("PIP");

            break;
        }
        case MODOS::VC_CMV:{
            ui->VCCMV->setChecked(true);
            if (!ui->Modos_frame->isEnabled()){
                ui->PCCMV->setStyleSheet(modoDesactivado);
                ui->PCSIMV->setStyleSheet(modoDesactivado);
                ui->PCAC->setStyleSheet(modoDesactivado);
                ui->VCAC->setStyleSheet(modoDesactivado);
                ui->VCSIMV->setStyleSheet(modoDesactivado);
            } else {
                ui->PCCMV->setStyleSheet(modoInactivo);
                ui->PCSIMV->setStyleSheet(modoInactivo);
                ui->PCAC->setStyleSheet(modoInactivo);
                ui->VCAC->setStyleSheet(modoInactivo);
                ui->VCSIMV->setStyleSheet(modoInactivo);
            }
            ui->VCCMV->setStyleSheet(modoActivo);
            ui->propTp_frame->setEnabled(true);
//            ui->ThresLvlFlow_frame->setEnabled(false);
            ui->ThresLvlPress_frame->setEnabled(false);
//            ui->triggerMode->setEnabled(false);

            ui->PSOP_frame->setEnabled(false);
            ui->Vt_label->setText("Vt");
            ui->Pip_label->setText("P MAX");

            break;
        }
        case MODOS::VC_AC:{
            ui->VCAC->setChecked(true);
            if (!ui->Modos_frame->isEnabled()){
                ui->PCCMV->setStyleSheet(modoDesactivado);
                ui->PCSIMV->setStyleSheet(modoDesactivado);
                ui->VCCMV->setStyleSheet(modoDesactivado);
                ui->PCAC->setStyleSheet(modoDesactivado);
                ui->VCSIMV->setStyleSheet(modoDesactivado);
            } else {
                ui->PCCMV->setStyleSheet(modoInactivo);
                ui->PCAC->setStyleSheet(modoInactivo);
                ui->VCCMV->setStyleSheet(modoInactivo);
                ui->PCAC->setStyleSheet(modoInactivo);
                ui->VCSIMV->setStyleSheet(modoInactivo);
            }
            ui->VCAC->setStyleSheet(modoActivo);
            ui->propTp_frame->setEnabled(true);
//            ui->ThresLvlFlow_frame->setEnabled(true);
            ui->ThresLvlPress_frame->setEnabled(true);
//            ui->triggerMode->setEnabled(true);

            ui->PSOP_frame->setEnabled(false);
            ui->Vt_label->setText("Vt");
            ui->Pip_label->setText("P MAX");
            break;
        }
        case MODOS::VC_SIMV:{
            ui->VCSIMV->setChecked(true);
            if (!ui->Modos_frame->isEnabled()){
                ui->PCCMV->setStyleSheet(modoDesactivado);
                ui->PCSIMV->setStyleSheet(modoDesactivado);
                ui->VCCMV->setStyleSheet(modoDesactivado);
                ui->VCAC->setStyleSheet(modoDesactivado);
                ui->PCAC->setStyleSheet(modoDesactivado);
            } else {
                ui->PCCMV->setStyleSheet(modoInactivo);
                ui->PCAC->setStyleSheet(modoInactivo);
                ui->VCCMV->setStyleSheet(modoInactivo);
                ui->VCAC->setStyleSheet(modoInactivo);
                ui->PCAC->setStyleSheet(modoInactivo);
            }
            ui->VCSIMV->setStyleSheet(modoActivo);
            ui->propTp_frame->setEnabled(true);
//            ui->ThresLvlFlow_frame->setEnabled(true);
            ui->ThresLvlPress_frame->setEnabled(true);
//            ui->triggerMode->setEnabled(true);

            ui->PSOP_frame->setEnabled(true);
            ui->Vt_label->setText("Vt");
            ui->Pip_label->setText("P MAX");
            break;
        }
    case MODOS::Count: break;
    }

    QString mensaje;
    mensaje = "Estado = ";
    switch (miEstado.estado) {
        case MachineState::RESET: {
            mensaje = mensaje + "RESET";
            //send_params_hardware();
            send_puntos_calibracion();
            break;
        }
        case MachineState::OFF: {
            mensaje = mensaje + "OFF";
            ui->marchaParoButton->setText("MARCHA");
            break;
        }
        case MachineState::ON:{
            mensaje = mensaje + "ON";
            ui->marchaParoButton->setText("PARO");
            break;
        }
        case MachineState::CAL: {
            mensaje = mensaje + "CAL";
        }
    }
    ui->pizarra->appendPlainText(mensaje);
}

void MainWindow::confirm_config(QByteArray buffer, qint16 bytes_received){
    uint8_t index;
    uint16_t val;
    index = buffer.at(1);
    val = getuint16fromint8((uint8_t) buffer.at(2), (uint8_t) buffer.at(3));
    switch(index){
    case (int) PARAM_CONFIG::porTp: setPropTpText(val); break;
    case (int) PARAM_CONFIG::TrigLvl_Press: setTrigLvlPressText(val); break;
//    case (int) PARAM_CONFIG::TrigLvl_Flow: setTrigLvlFlowText(val); break;
    }
}


void MainWindow::confirm_param(QByteArray buffer, qint16 bytes_received){
    uint8_t index;
    uint16_t val;
    index = buffer.at(1);
    val = getuint16fromint8((uint8_t) buffer.at(2), (uint8_t) buffer.at(3));
    switch(index){
        case (int) PARAM_OPERACION::Fr: setFrText(val); break;
        case (int) PARAM_OPERACION::iex10: {
            uint8_t indice = 0;
            while (indice <= 4){
                if (Etoi_vals[indice] == val) break;
                indice++;
            }
            setEtoiText(indice);
            break;
        }
        case (int) PARAM_OPERACION::Pip: setPipText(val); break;
        case (int) PARAM_OPERACION::Vt: setVtText(val); break;
        case (int) PARAM_OPERACION::Peep: setPeepText(val); break;
        case (int) PARAM_OPERACION::Psop: setPsopText(val); break;
    }
}


void MainWindow::recibir_datos(QByteArray buffer, quint16 bytes_received){
    misDatos.pressure = getuint16fromint8(buffer.at(1), buffer.at(2));
    misDatos.Insp_Flow= getuint16fromint8(buffer.at(3), buffer.at(4));
    misDatos.Esp_Flow = getuint16fromint8(buffer.at(5), buffer.at(6));
    misDatos.FiO2 = getuint16fromint8(buffer.at(7), buffer.at(8));
    ui->FIO2Show_value->setText(QString::number(misDatos.FiO2));
    updateGrafico(misDatos);
}

void MainWindow::recibir_datosciclo(QByteArray buffer, quint16 bytes_received){
    misDatosCiclo.Ppico = getuint16fromint8(buffer.at(1), buffer.at(2));
    misDatosCiclo.Pplateau = getuint16fromint8(buffer.at(3), buffer.at(4));
    misDatosCiclo.Vinsp = getuint16fromint8(buffer.at(5), buffer.at(6));
    misDatosCiclo.peepM = getuint16fromint8(buffer.at(7), buffer.at(8));
    misDatosCiclo.Comp = getuint16fromint8(buffer.at(9), buffer.at(10));
    misDatosCiclo.Res = getuint16fromint8(buffer.at(11), buffer.at(12));
    misDatosCiclo.FImax = getuint16fromint8(buffer.at(13), buffer.at(14));
    misDatosCiclo.FEmax = getuint16fromint8(buffer.at(15), buffer.at(16));
    misDatosCiclo.MinVol = getuint16fromint8(buffer.at(17), buffer.at(18));
    misDatosCiclo.Alarma = getuint16fromint8(buffer.at(19), buffer.at(20));
    emit alarmaRecibida(misDatosCiclo.Alarma);
    set_params_ciclo_Text(misDatosCiclo);
}

void MainWindow::fillPortsInfo()
{
    ui->serialPortDropdown->clear();
    static const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortDropdown->addItem(list.first(), list);
    }
}


void MainWindow::openSerialPort()
{
    serial->setPortName(ui->serialPortDropdown->currentText());
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
            ui->statusBar->showMessage(tr("Conectado al MCU por el puerto %1 : %2, %3, %4, %5, %6")
                                       .arg(ui->serialPortDropdown->currentText())
                                       .arg(QSerialPort::Baud115200)
                                       .arg(QSerialPort::Data8)
                                       .arg(QSerialPort::NoParity)
                                       .arg(QSerialPort::OneStop)
                                       .arg(QSerialPort::NoFlowControl));
            ui->pushButton_connect->setText("OFF");
            disconnect( ui->pushButton_connect, SIGNAL(clicked()),0, 0);
            QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                             this, SLOT(closeSerialPort()));
            QThread::msleep(500);
            init_controller();
    } else {
        QMessageBox::critical(this, tr("Error Crítico"), "No se puede comunicar con MCU");
        ui->statusBar->showMessage(tr("Error Crítico"));
    }

}

void MainWindow::closeSerialPort()
{
    estadoMaquinaStruct tmp = {MachineState::OFF, miEstado.modo, miEstado.disparo};
    sendState(tmp);
    deactivate_gui();
    QThread::msleep(1000);
    serial->close();
    ui->statusBar->showMessage(tr("Disconnected"));
    disconnect( ui->pushButton_connect, SIGNAL(clicked()),0, 0);
    QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                     this, SLOT(openSerialPort()));
    ui->pushButton_connect->setText("ON");
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();
//    ui->plainTextEdit_input->appendPlainText(data);
    int dSize = data.size();
    if (dSize > 0) {
        emit dataReceived(data);
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::sendData(QByteArray data) {
    quint16 length = data.length();
    emit sendDataFrame(data, length);
}

void MainWindow::setStatusbarText(const QString& text) {
    ui->statusBar->showMessage(text);
}


void MainWindow::updateGrafico(sendDataStruct datos)
{
    this->pressCurve[ptr] = datos.pressure/10.0;
    this->expFlowCurve[ptr] = -datos.Esp_Flow/10.0;
    this->inspFlowCurve[ptr] = datos.Insp_Flow/10.0;
    int j;
    for(int i = 1; i < 11; i++){
        j = i+ptr;
        if (j >= PLOTLENGTH) j = i;

        this->pressCurve[j] = 0;
        this->expFlowCurve[j] = 0;
        this->inspFlowCurve[j] = 0;
    }
    this->ptr ++;
    if (ptr>=PLOTLENGTH) ptr=0;

    ui->pressPlot->graph(0)->setData(timeVect, pressCurve);
    ui->flowPlot->graph(0)->setData(timeVect, expFlowCurve);
    ui->flowPlot->graph(1)->setData(timeVect, inspFlowCurve);
    ui->pressPlot->yAxis->rescale();
    ui->flowPlot->yAxis->rescale();
    ui->pressPlot->replot();
    ui->flowPlot->replot();
    //ui->pressPlot->update();
}

void MainWindow::MakeCanvas()
{

    //airway pressure plot
    //ui->pressPlot->setOpenGl(true);
    ui->pressPlot->addGraph();
    ui->pressPlot->graph(0)->setData(timeVect,pressCurve);
    ui->pressPlot->graph(0)->setPen(QPen(QColor(255, 255, 255)));
//    ui->pressPlot->graph(0)->setBrush(QBrush(QColor(255, 255, 255)));
    //ui->pressPlot->graph(0)->setPen(QPen(Qt::red, 4));

    ui->pressPlot->xAxis->setLabelColor(QColor(255,255,255));
    ui->pressPlot->xAxis->setTickLabelColor(QColor(255,255,255));
    ui->pressPlot->yAxis->setLabelColor(QColor(255,255,255));
    ui->pressPlot->yAxis->setTickLabelColor(QColor(255,255,255));
    ui->pressPlot->xAxis->setLabel("Time (s) ");
    ui->pressPlot->yAxis->setLabel("Pressure (cmH2O)");
    ui->pressPlot->xAxis->setRange(0,12);
    ui->pressPlot->yAxis->setRange(0,80);
    ui->pressPlot->setBackground(QColor(52, 101, 164));
    ui->pressPlot->replot();

    //expiration flow plot
    //ui->flowPlot->setOpenGl(true);
    ui->flowPlot->addGraph();
    ui->flowPlot->graph(0)->setData(timeVect,expFlowCurve);
    ui->flowPlot->graph(0)->setName("Expiration");
    ui->flowPlot->graph(0)->setPen(QPen(QColor(255, 255, 255)));
//    ui->flowPlot->graph(0)->setBrush(QBrush(QColor(255, 255, 255)));

    ui->flowPlot->addGraph();
    ui->flowPlot->graph(1)->setData(timeVect,inspFlowCurve);
    ui->flowPlot->graph(1)->setName("Inspiration");
    ui->flowPlot->graph(1)->setPen(QPen(QColor(255, 255, 255)));
//    ui->flowPlot->graph(1)->setBrush(QBrush(QColor(255, 255, 255)));
    //ui->flowPlot->graph(0)->setPen(QPen(Qt::white, 4));
    //ui->flowPlot->graph(1)->setPen(QPen(Qt::white, 4));

    ui->flowPlot->xAxis->setLabelColor(QColor(255,255,255));
    ui->flowPlot->xAxis->setTickLabelColor(QColor(255,255,255));
    ui->flowPlot->yAxis->setLabelColor(QColor(255,255,255));
    ui->flowPlot->yAxis->setTickLabelColor(QColor(255,255,255));
    ui->flowPlot->xAxis->setLabel("Time (s) ");
    ui->flowPlot->yAxis->setLabel("Exp Flow (l/min)");
    ui->flowPlot->xAxis->setRange(0,12);
    ui->flowPlot->yAxis->setRange(-120,120);
    ui->flowPlot->setBackground(QColor(52, 101, 164));
    ui->flowPlot->replot();

    this->timeVect.fill(0,PLOTLENGTH);
    this->pressCurve.fill(10,PLOTLENGTH);
    this->expFlowCurve.fill(0,PLOTLENGTH);
    this->inspFlowCurve.fill(0,PLOTLENGTH);

    for (int i=0; i<PLOTLENGTH; ++i)
    {
        this->timeVect[i] = i/100.0; //10 mseg
        this->pressCurve[i] = 0.0;
        this->expFlowCurve[i] = 0.0;
        this->inspFlowCurve[i] = 0.0;
    }
}


void MainWindow::ClearCanvas()
{
    // It should be another way to clear the graph but ok
    //make sure that the vectors not keep increasing
    timeVect.fill(0.0,PLOTLENGTH);
    pressCurve.fill(0.0,PLOTLENGTH);
    expFlowCurve.fill(0.0,PLOTLENGTH);
    inspFlowCurve.fill(0.0,PLOTLENGTH);

    ui->pressPlot->graph(0)->setData(timeVect,pressCurve);
    ui->pressPlot->graph(1)->setData(timeVect,pressCurve);
    ui->pressPlot->replot();
    ui->pressPlot->update();

    ui->flowPlot->graph(0)->setData(timeVect,expFlowCurve);
    ui->flowPlot->graph(1)->setData(timeVect,inspFlowCurve);
    ui->flowPlot->graph(2)->setData(timeVect,inspFlowCurve);
    ui->flowPlot->replot();
    ui->flowPlot->update();
}

void MainWindow::on_PCCMV_toggled(bool checked)
{
    if (checked){
        ui->PCCMV->setStyleSheet(this->modoActivo);
        miEstado.modo = MODOS::PC_CMV;

        ui->propTp_frame->setEnabled(false);
//        ui->ThresLvlFlow_frame->setEnabled(false);
        ui->ThresLvlPress_frame->setEnabled(false);
//        ui->triggerMode->setEnabled(false);

        ui->PSOP_frame->setEnabled(false);
        ui->Vt_label->setText("Vt MAX");
        ui->Pip_label->setText("PIP");
        miEstado.disparo = TRIGGER::None;
        sendState(miEstado);
    }  else
        ui->PCCMV->setStyleSheet(modoInactivo);
}

void MainWindow::on_PCAC_toggled(bool checked)
{
    if (checked){
        ui->PCAC->setStyleSheet(modoActivo);
        miEstado.modo = MODOS::PC_AC;
        ui->propTp_frame->setEnabled(false);
//        ui->ThresLvlFlow_frame->setEnabled(true);
        ui->ThresLvlPress_frame->setEnabled(true);
//        ui->triggerMode->setEnabled(true);

        ui->PSOP_frame->setEnabled(false);
        ui->Vt_label->setText("Vt MAX");
        ui->Pip_label->setText("PIP");
//        if (ui->triggerModePressure->isChecked())
            miEstado.disparo = TRIGGER::Presion;
//        else
//            miEstado.disparo = TRIGGER::Flujo;

        sendState(miEstado);
    } else
        ui->PCAC->setStyleSheet(modoInactivo);
}

void MainWindow::on_PCSIMV_toggled(bool checked)
{
    if (checked){
        ui->PCSIMV->setStyleSheet(modoActivo);
        miEstado.modo = MODOS::PC_SIMV;

        ui->propTp_frame->setEnabled(false);
//        ui->ThresLvlFlow_frame->setEnabled(true);
        ui->ThresLvlPress_frame->setEnabled(true);
//        ui->triggerMode->setEnabled(true);

        ui->PSOP_frame->setEnabled(true);
        ui->Vt_label->setText("Vt MAX");
        ui->Pip_label->setText("PIP");
//        if (ui->triggerModePressure->isChecked())
            miEstado.disparo = TRIGGER::Presion;
//        else
//            miEstado.disparo = TRIGGER::Flujo;

        sendState(miEstado);
    }else
        ui->PCSIMV->setStyleSheet(modoInactivo);
}

void MainWindow::on_VCCMV_toggled(bool checked)
{
    if (checked){
        ui->VCCMV->setStyleSheet(modoActivo);
        miEstado.modo = MODOS::VC_CMV;

        ui->propTp_frame->setEnabled(true);
//        ui->ThresLvlFlow_frame->setEnabled(false);
        ui->ThresLvlPress_frame->setEnabled(false);
//        ui->triggerMode->setEnabled(false);

        ui->PSOP_frame->setEnabled(false);
        ui->Vt_label->setText("Vt");
        ui->Pip_label->setText("P MAX");
        miEstado.disparo = TRIGGER::None;
        sendState(miEstado);

    } else
        ui->VCCMV->setStyleSheet(modoInactivo);
}

void MainWindow::on_VCAC_toggled(bool checked)
{
    if (checked){
        ui->VCAC->setStyleSheet(modoActivo);
        miEstado.modo = MODOS::VC_AC;

        ui->propTp_frame->setEnabled(true);
//        ui->ThresLvlFlow_frame->setEnabled(true);
        ui->ThresLvlPress_frame->setEnabled(true);
//        ui->triggerMode->setEnabled(true);

        ui->PSOP_frame->setEnabled(false);
        ui->Vt_label->setText("Vt");
        ui->Pip_label->setText("P MAX");
//        if (ui->triggerModePressure->isChecked())
            miEstado.disparo = TRIGGER::Presion;
//        else
//            miEstado.disparo = TRIGGER::Flujo;

        sendState(miEstado);
    } else
        ui->VCAC->setStyleSheet(modoInactivo);
}

void MainWindow::on_VCSIMV_toggled(bool checked)
{
    if (checked){
        ui->VCSIMV->setStyleSheet(modoActivo);
        miEstado.modo = MODOS::VC_SIMV;

        ui->propTp_frame->setEnabled(true);
//        ui->ThresLvlFlow_frame->setEnabled(true);
        ui->ThresLvlPress_frame->setEnabled(true);
//        ui->triggerMode->setEnabled(true);

        ui->PSOP_frame->setEnabled(true);
        ui->Vt_label->setText("Vt");
        ui->Pip_label->setText("P MAX");
//        if (ui->triggerModePressure->isChecked())
            miEstado.disparo = TRIGGER::Presion;
//        else
//            miEstado.disparo = TRIGGER::Flujo;

        sendState(miEstado);
    } else
        ui->VCSIMV->setStyleSheet(modoInactivo);
}

//void MainWindow::on_triggerModePressure_toggled(bool checked)
//{
//    miEstado.disparo = TRIGGER::Presion;

//}

//void MainWindow::on_triggerModeFlow_toggled(bool checked)
//{
//    miEstado.disparo = TRIGGER::Flujo;
//}

void MainWindow::on_ThresLvlPress_minus_button_clicked()
{
    int16_t temp =  P_config[(int) PARAM_CONFIG::TrigLvl_Press];
    temp -= ThresLvlPress_inc;
    if (temp < ThresLvlPress_min) return;
    P_config[(int) PARAM_CONFIG::TrigLvl_Press]= temp;
    ui->ThresLvlPress_textEdit->setPlainText("---");
    ui->ThresLvlPress_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::TrigLvl_Press, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_ThresLvlPress_plus_button_clicked()
{
    int16_t temp =  P_config[(int) PARAM_CONFIG::TrigLvl_Press];
    temp += ThresLvlPress_inc;
    if (temp > ThresLvlPress_max) return;
    P_config[(int) PARAM_CONFIG::TrigLvl_Press]= temp;
    ui->ThresLvlPress_textEdit->setPlainText("---");
    ui->ThresLvlPress_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::TrigLvl_Press, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

/*void MainWindow::on_ThresLvlFlow_minus_button_clicked()
{
    int16_t temp =  P_config[(int) PARAM_CONFIG::TrigLvl_Flow];
    temp -= ThresLvlFlow_inc;
    if (temp < ThresLvlFlow_min) return;
    P_config[(int) PARAM_CONFIG::TrigLvl_Flow]= temp;
    ui->ThresLvlFlow_textEdit->setPlainText("---");
    ui->ThresLvlFlow_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::TrigLvl_Flow, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_ThresLvlFlow_plus_button_clicked()
{
    int16_t temp =  P_config[(int) PARAM_CONFIG::TrigLvl_Flow];
    temp += ThresLvlFlow_inc;
    if (temp > ThresLvlFlow_max) return;
    P_config[(int) PARAM_CONFIG::TrigLvl_Flow]= temp;
    ui->ThresLvlFlow_textEdit->setPlainText("---");
    ui->ThresLvlFlow_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::TrigLvl_Flow, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}*/

void MainWindow::on_propTp_minus_button_clicked()
{
    int16_t temp =  P_config[(int) PARAM_CONFIG::porTp];
    temp -= propTp_inc;
    if (temp < propTp_min) return;
    P_config[(int) PARAM_CONFIG::porTp]= temp;
    ui->propTp_textEdit->setPlainText("---");
    ui->propTp_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::porTp, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_propTp_plus_button_clicked()
{
    int16_t temp =  P_config[(int) PARAM_CONFIG::porTp];
    temp += propTp_inc;
    if (temp > propTp_max) return;
    P_config[(int) PARAM_CONFIG::porTp]= temp;
    ui->propTp_textEdit->setPlainText("---");
    ui->propTp_textEdit->setAlignment(Qt::AlignCenter);
    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::porTp, (uint16_t) temp);
    emit sendDataFrame(data, (quint16) 4);
}

void MainWindow::on_pushButton_calibracion_clicked()
{
    estadoMaquinaStruct tmp = {MachineState::CAL, miEstado.modo, miEstado.disparo};
    sendState(tmp);
    confDlg->setGeometry(0, 0, 341, 400);
    confDlg->exec();
    tmp = {MachineState::OFF, miEstado.modo, miEstado.disparo};
    sendState(tmp);
}

void MainWindow::on_configAlarms_clicked()
{
    alarmDlg->setGeometry(0,0,400, 400);
    alarmDlg->exec();
}


void MainWindow::sendValvPos(quint16 InsPos, quint16 EspPos){
    //ui->pizarra->appendPlainText("Vengo a sendValPos");
    QByteArray data;
    data.append((uint8_t) FrameType::CAL_PARAM);
    data.append(get_lsb(InsPos));
    data.append(get_msb(InsPos));
    data.append(get_lsb(EspPos));
    data.append(get_msb(EspPos));
    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::calibrationParamOk(QByteArray buffer, qint16 bytes_received){
//    ui->pizarra->appendPlainText("Llegó confirmación del MCU");
    return;
}

void MainWindow::calibrationDataReceived(QByteArray buffer, quint16 bytes_received){
    //u_int16_t tmp1, tmp2, tmp3;
    //tmp1 = getuint16fromint8(buffer.at(1), buffer.at(2));
    //tmp2 = getuint16fromint8(buffer.at(3), buffer.at(4));
    //tmp3 = getuint16fromint8(buffer.at(5), buffer.at(6));

    //ui->pizarra->appendPlainText("CALDATA: " + QString::number(tmp1)+ " " + QString::number(tmp2) + " " + QString::number(tmp3));
    emit calibrationDataToConfig(buffer, bytes_received);
}

void MainWindow::on_pushButtonNULA_clicked()
{
    sendAlarmSnd((uint8_t) TIPOALARMA::NULA);
//    QByteArray data;
//    data.append((uint8_t) FrameType::SET_ALARM);
//    data.append((uint8_t) TIPOALARMA::NULA);
//    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::on_pushButtonBAJA_clicked()
{
    sendAlarmSnd((uint8_t) TIPOALARMA::BAJA);
//    QByteArray data;
//    data.append((uint8_t) FrameType::SET_ALARM);
//    data.append((uint8_t) TIPOALARMA::BAJA);
//    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::on_pushButtonMEDIA_clicked()
{
    sendAlarmSnd((uint8_t) TIPOALARMA::MEDIA);
//    QByteArray data;
//    data.append((uint8_t) FrameType::SET_ALARM);
//    data.append((uint8_t) TIPOALARMA::MEDIA);
//    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::on_pushButtonALTA_clicked()
{
    sendAlarmSnd((uint8_t) TIPOALARMA::ALTA);
//    QByteArray data;
//    data.append((uint8_t) FrameType::SET_ALARM);
//    data.append((uint8_t) TIPOALARMA::ALTA);
//    emit sendDataFrame(data, (quint16) data.size());
}

void MainWindow::sendAlarmSnd(quint8 nivel) {
    QByteArray data;
    data.append((uint8_t) FrameType::SET_ALARM);
    data.append(nivel);
    emit sendDataFrame(data, (quint16) data.size());
}


void MainWindow::on_silencioAlarms_clicked()
{
    sendAlarmSnd((uint8_t) TIPOALARMA::NULA);
}

void MainWindow::on_newExpiration_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
        P_hardware[(int) PARAM_CONFIG::newAlgorithm] = 1;
    else
        P_hardware[(int) PARAM_CONFIG::newAlgorithm] = 0;

    QByteArray data = get_config_frame((uint8_t) PARAM_CONFIG::newAlgorithm,
                                       (uint16_t) P_hardware[(int) PARAM_CONFIG::newAlgorithm]);
    emit sendDataFrame(data, (quint16) 4);
}
