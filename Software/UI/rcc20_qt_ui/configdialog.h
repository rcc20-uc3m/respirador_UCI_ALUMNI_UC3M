#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

signals:
    void setValvPos(quint16, quint16);

private slots:
    void on_VInsp_minus_button_clicked();

    void on_VInsp_plus_button_clicked();

    void on_VEsp_minus_button_clicked();

    void on_VEsp_plus_button_clicked();

public slots:

    void setValues(QByteArray buffer, qint16 bytes_received);

private:
    Ui::ConfigDialog *ui;

    void displayValues();

    uint16_t InspPos;
    uint16_t EspPos;
    uint16_t PressVal;
    uint16_t FIVal;
    uint16_t FEVal;
    uint16_t PressCAL;
    uint16_t FICAL;
    uint16_t FECAL;
};

#endif // CONFIGDIALOG_H
