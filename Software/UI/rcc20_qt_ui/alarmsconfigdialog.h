#ifndef ALARMSCONFIGDIALOG_H
#define ALARMSCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class AlarmsConfigDialog;
}

class AlarmsConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AlarmsConfigDialog(QWidget *parent = nullptr);
    ~AlarmsConfigDialog();
     void leeParametros();

private slots:
    void on_PMAX_minus_button_clicked();
    void on_PMAX_plus_button_clicked();
    void on_DELTAPEEP_minus_button_clicked();
    void on_DELTAPEEP_plus_button_clicked();
    void on_DELTAVT_minus_button_clicked();
    void on_DELTAVT_plus_button_clicked();
    void on_MVMIN_minus_button_clicked();
    void on_MVMIN_plus_button_clicked();
    void on_MVMAX_minus_button_clicked();
    void on_MVMAX_plus_button_clicked();
    void on_PIPDESC_minus_button_clicked();
    void on_PIPDESC_plus_button_clicked();
    void on_NDESC_minus_button_clicked();
    void on_NDESC_plus_button_clicked();

signals:
    void AlarmParamsReady(quint16, quint16, quint16, quint16, quint16, quint16, quint16);

private:
    Ui::AlarmsConfigDialog *ui;
    quint16 Pmax;
    quint16 DeltaPeep;
    quint16 DeltaVt;
    quint16 MV_min;
    quint16 MV_max;
    quint16 Pip_Desc;
    quint16 N_Desc;

    void DisplayParametros();

};

#endif // ALARMSCONFIGDIALOG_H
