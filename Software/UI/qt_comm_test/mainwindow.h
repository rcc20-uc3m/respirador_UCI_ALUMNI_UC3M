#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>

#define PLOTLENGTH 1200

enum class FrameType{DATA, DATACYCLE, SENDPARAM, CHECKPARAM, Count};

struct sendDataStruct {
    uint16_t pressure; // pressure in mmH2O 1 mmH2O = 0,0980665 mbar approx 0,1 mbar
    uint16_t Insp_Flow; // flow in ml/s
    uint16_t Esp_Flow; // flow in ml/s
};

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

signals:
    void sendDataFrame(QByteArray buffer, quint16 size);
    void dataReceived(QByteArray data);

private:
    Ui::MainWindow *ui;
    void fillPortsInfo();
    void recibir_datos(QByteArray buffer, quint16 bytes_received);
    void recibir_datosciclo(QByteArray buffer, quint16 bytes_received);
    QSerialPort *serial;
    sendDataStruct misDatos = {0, 0, 0};


    void MakeCanvas();
    void ClearCanvas();
    void updateGrafico(sendDataStruct datos);

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


  /// === Config Display ===
    int pressurePlotLimitDown = -5;
    int pressurePlotLimitUp = 60;
    int flowPlotLimitDown = -100;
    int flowPlotLimitUp = 100;

private slots:
    void sendData();
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void sendData(QByteArray data);
    void handleError(QSerialPort::SerialPortError error);
};
#endif // MAINWINDOW_H
