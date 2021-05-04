#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include <datos.h>
#include <configdialog.h>
#include <alarmsconfigdialog.h>
#include <alarms.h>

#define PLOTLENGTH 1200


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void putChar(QByteArray data);
    void putChar(char data);
    void HDLC_CommandRouter(QByteArray buffer, quint16 bytes_received);
    void setStatusbarText(const QString& text);
    void sendAlarmSnd(quint8 nivel);
    void send_alarms_param(quint16, quint16, quint16, quint16, quint16, quint16, quint16);

signals:
    void sendDataFrame(QByteArray buffer, quint16 size);
    void dataReceived(QByteArray data);
    void portOpened(void);
    void calibrationDataToConfig(QByteArray buffer, quint16 bytes_received);
    void alarmaRecibida(quint16 mialarma);

private:
    Ui::MainWindow *ui;
    ConfigDialog *confDlg;
    AlarmsConfigDialog *alarmDlg;
    SndState *miSndState;
    void fillPortsInfo();
    void recibir_datos(QByteArray buffer, quint16 bytes_received);
    void recibir_datosciclo(QByteArray buffer, quint16 bytes_received);
    void confirm_param(QByteArray buffer, qint16 bytes_received);
    void confirm_config(QByteArray buffer, qint16 bytes_received);
    void set_state(QByteArray buffer, qint16 bytes_received);
    void display_debug(QByteArray buffer, qint16 bytes_received);
    void send_params_config(void);
    void send_params_operacion(void);
    void send_params_alarmas(void);
    void send_params_hardware(void);
    void send_puntos_calibracion(void);
    void start_machine(void);
    void sendState(estadoMaquinaStruct estado);
    void calibrationParamOk(QByteArray buffer, qint16 bytes_received);
    void calibrationDataReceived(QByteArray buffer, quint16 bytes_received);


    QSerialPort *serial;
    sendDataStruct misDatos = {0, 0, 0, 0};
    sendDataCycleStruct misDatosCiclo;
    estadoMaquinaStruct miEstado;


    void MakeCanvas();
    void ClearCanvas();
    void updateGrafico(sendDataStruct datos);
    void init_parametros(void);
    void init_gui(void);
    void init_controller(void);
    void activate_gui(void);
    void deactivate_gui(void);

    void setFrText(quint16 val);
    void setEtoiText(quint16 val);
    void setPipText(quint16 val);
    void setVtText(quint16 val);
    void setPeepText(quint16 val);
    void setPsopText(quint16 val);
    void setTriText(qint16 val);
    void setPropTpText(qint16 val);
    void setTrigLvlPressText(qint16 val);
    void setTrigLvlFlowText(qint16 val);
    void set_params_ciclo_Text(sendDataCycleStruct misDatosCiclo);



    /// === Plot ===
    QVector<double> timeVect;
    QVector<double> pressCurve;
    QVector<double> expFlowCurve;
    QVector<double> inspFlowCurve;
    int ptr;


    /// === Style ===
    QString colorTextLabels = "#FAFAFA";
    QString colorBackgroundLabels = "#5762A7";
    QString colorModeONBackgorund = "#CEAB59";
    QString colorModeOFFBackgorund = "#BAB9B9";
    QString colorButtonsONBackgorund = "#DCDCDC";
    QString stringButtonON = "font:bold; color:" + colorBackgroundLabels +"; background-color:" + colorButtonsONBackgorund +";";
    QString stringDisplayButtonOFF = "font:bold; color:" + colorBackgroundLabels +"; background-color:" + colorButtonsONBackgorund +";border-style:none;";
    QString modoActivo = "color: rgb(52, 101, 164); background-color:white;";
    QString modoInactivo = "color:rgb(20,20,50); background-color: rgb(52, 101, 164);";
    QString modoDesactivado = "color:rgb(52, 101, 164); background-color: rgb(52, 101, 164);";


  /// === Config Display ===
    int pressurePlotLimitDown = -5;
    int pressurePlotLimitUp = 60;
    int flowPlotLimitDown = -100;
    int flowPlotLimitUp = 100;

  /// === min, max, increment in controls
    uint16_t Fr_min = 10;
    uint16_t Fr_max = 40;
    uint16_t Fr_inc = 1;
    uint16_t Etoi_min = 0;
    uint16_t Etoi_max = 4;
    uint16_t Etoi_inc = 1;
    uint16_t Etoi_vals[5] = {5, 10, 20, 30, 40};
    QStringList Etoi_labels = { "2:1", "1:1", "1:2", "1:3", "1:4"};
    uint16_t Vt_min = 240;
    uint16_t Vt_max = 2000;
    uint16_t Vt_inc = 10;
    uint16_t Pip_min = 0;
    uint16_t Pip_max = 600;
    uint16_t Pip_inc = 10;
    uint16_t Peep_min = 0;
    uint16_t Peep_max = 250;
    uint16_t Peep_inc = 10;
    uint16_t Psop_min = 0;
    uint16_t Psop_max = 400;
    uint16_t Psop_inc = 10;

    uint16_t Tri_min = 10;
    uint16_t Tri_max = 300;
    uint16_t Tri_inc = 10;
    uint16_t propTp_min = 0;
    uint16_t propTp_max = 20;
    uint16_t propTp_inc = 1;
    uint16_t ThresLvlPress_min = 0;
    uint16_t ThresLvlPress_max = 50;
    uint16_t ThresLvlPress_inc = 1;
    uint16_t ThresLvlFlow_min = 0;
    uint16_t ThresLvlFlow_max = 50;
    uint16_t ThresLvlFlow_inc = 5;


private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void sendData(QByteArray data);
    void handleError(QSerialPort::SerialPortError error);
    void on_Fr_plus_button_clicked();
    void on_Fr_minus_button_clicked();
    void on_Etoi_plus_button_clicked();
    void on_Etoi_minus_button_clicked();
    void on_Pip_plus_button_clicked();
    void on_Pip_minus_button_clicked();
    void on_Peep_plus_button_clicked();
    void on_Peep_minus_button_clicked();
    void on_Vt_plus_button_clicked();
    void on_Vt_minus_button_clicked();
    void on_Psop_plus_button_clicked();
    void on_Psop_minus_button_clicked();
    void marchaParo();
    void on_PCCMV_toggled(bool checked);
    void on_PCAC_toggled(bool checked);
    void on_PCSIMV_toggled(bool checked);
    void on_VCCMV_toggled(bool checked);
    void on_VCAC_toggled(bool checked);
    void on_VCSIMV_toggled(bool checked);
    void on_ThresLvlPress_minus_button_clicked();
    void on_ThresLvlPress_plus_button_clicked();
    void on_propTp_minus_button_clicked();
    void on_propTp_plus_button_clicked();
    void sendValvPos(quint16, quint16);
    void on_pushButton_calibracion_clicked();
    void on_pushButtonNULA_clicked();
    void on_pushButtonBAJA_clicked();
    void on_pushButtonMEDIA_clicked();
    void on_pushButtonALTA_clicked();
    void on_configAlarms_clicked();
    void on_silencioAlarms_clicked();
    void on_newExpiration_stateChanged(int arg1);
};
#endif // MAINWINDOW_H
