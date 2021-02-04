#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hdlc_qt.h"
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("Qt to Arduino HDLC command router example", 3000);
    serial = new QSerialPort(this);
    this->fillPortsInfo();
    HDLC_qt* hdlc = HDLC_qt::instance();

    QObject::connect(hdlc, SIGNAL(hdlcTransmitByte(QByteArray)),
                     this, SLOT(putChar(QByteArray)));

    QObject::connect(hdlc, SIGNAL(hdlcTransmitByte(char)),
                     this, SLOT(putChar(char)));

    QObject::connect(hdlc, SIGNAL(hdlcValidFrameReceived(QByteArray, quint16)),
                     this, SLOT(HDLC_CommandRouter(QByteArray, quint16)));

    // All data coming from serial port (should go straight to HDLC validation)
    QObject::connect(this, SIGNAL(dataReceived(QByteArray)),
                     hdlc, SLOT(charReceiver(QByteArray)));


    QObject::connect(this, SIGNAL(sendDataFrame(QByteArray, quint16)),
                     hdlc, SLOT(frameDecode(QByteArray, quint16)));

    QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                     this, SLOT(openSerialPort()));

    QObject::connect(serial, SIGNAL(readyRead()),
                     this, SLOT(readData()));


    this->ptr = 0;
    MakeCanvas();

}

MainWindow::~MainWindow()
{
    delete ui;
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
    case FrameType::DATACYCLE: break;
    case FrameType::CHECKPARAM: break;
    case FrameType::SENDPARAM: break;
    }
}

void MainWindow::recibir_datos(QByteArray buffer, quint16 bytes_received){
    misDatos.pressure = (buffer.at(1) | (buffer.at(2) & 0xFFFF) << 8 );
    misDatos.Insp_Flow= (buffer.at(3) | (buffer.at(4) & 0xFFFF) << 8 );
    misDatos.Esp_Flow = (buffer.at(5) | (buffer.at(6) & 0xFFFF) << 8 );
    updateGrafico(misDatos);
}

void recibir_datosciclo(QByteArray buffer, quint16 bytes_received){
    return;
}

void MainWindow::sendData() {
    QByteArray data;
    emit sendDataFrame(data, (quint16)data.length());
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
            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(ui->serialPortDropdown->currentText())
                                       .arg(QSerialPort::Baud115200)
                                       .arg(QSerialPort::Data8)
                                       .arg(QSerialPort::NoParity)
                                       .arg(QSerialPort::OneStop)
                                       .arg(QSerialPort::NoFlowControl));
            ui->pushButton_connect->setText("Disconnect");
            disconnect( ui->pushButton_connect, SIGNAL(clicked()),0, 0);
            QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                             this, SLOT(closeSerialPort()));

    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        ui->statusBar->showMessage(tr("Open error"));
    }

}

void MainWindow::closeSerialPort()
{
    serial->close();
    ui->statusBar->showMessage(tr("Disconnected"));
    disconnect( ui->pushButton_connect, SIGNAL(clicked()),0, 0);
    QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                     this, SLOT(openSerialPort()));
    ui->pushButton_connect->setText("Connect");
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
    this->pressCurve[ptr] = datos.pressure;
    this->expFlowCurve[ptr] = -datos.Esp_Flow;
    this->inspFlowCurve[ptr] = datos.Insp_Flow;
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
    ui->pressPlot->replot();
    ui->flowPlot->replot();
    //ui->pressPlot->update();
}

void MainWindow::MakeCanvas()
{

    //airway pressure plot
    ui->pressPlot->addGraph();
    ui->pressPlot->graph(0)->setData(timeVect,pressCurve);
    ui->pressPlot->xAxis->setLabel("Time (ms) ");
    ui->pressPlot->yAxis->setLabel("Pressure (mmH2O)");
    ui->pressPlot->xAxis->setRange(0,12000);
    ui->pressPlot->yAxis->setRange(0,100);
    ui->pressPlot->replot();

    //expiration flow plot
    ui->flowPlot->addGraph();
    ui->flowPlot->graph(0)->setData(timeVect,expFlowCurve);
    ui->flowPlot->graph(0)->setName("Expiration");
    ui->flowPlot->graph(0)->setPen(QPen(QColor(250, 120, 0)));

    ui->flowPlot->addGraph();
    ui->flowPlot->graph(1)->setData(timeVect,inspFlowCurve);
    ui->flowPlot->graph(1)->setName("Inspiration");
    ui->flowPlot->graph(1)->setPen(QPen(QColor(0, 180, 60)));

    ui->flowPlot->xAxis->setLabel("Time (ms) ");
    ui->flowPlot->yAxis->setLabel("Exp Flow (ml/s)");
    ui->flowPlot->xAxis->setRange(0,12000);
    ui->flowPlot->yAxis->setRange(-120,120);
    ui->flowPlot->replot();

    this->timeVect.fill(0,PLOTLENGTH);
    this->pressCurve.fill(10,PLOTLENGTH);
    this->expFlowCurve.fill(0,PLOTLENGTH);
    this->inspFlowCurve.fill(0,PLOTLENGTH);

    for (int i=0; i<PLOTLENGTH; ++i)
    {
        this->timeVect[i] = i*10; //10 mseg
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
