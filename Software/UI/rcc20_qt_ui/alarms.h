#ifndef ALARMS_H
#define ALARMS_H

#include <QtGlobal>
#include <QDateTime>
#include <QObject>
#include <QPlainTextEdit>

enum class TIPOSALARMA {PMAX, HIGH_PEEP, LOW_PEEP, HIGH_VOLUME, LOW_VOLUME, HIGH_MV, LOW_MV, GASS_SUPPLY,
                       POWER_SUPPLY, LOW_BATTERY, DISCONNECTION, Count};

enum NIVELALARMA {NULO, BAJO, MEDIO, ALTO};


#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

class alarms
{
    friend class SndState;
public:
    alarms();
    void setUmbrales(quint16 miumbral1, quint16 miumbral2, quint16 miumbral3, QString mens);
    void checkNewValue(bool nuevoEstado);
    bool getCambio(void) {return cambio;}
    quint8 getNivel(void){return nivel;}
    QString getSimbolo(void);


private:
    // miembros
    quint16 contador;
    quint16 umbral1;
    quint16 umbral2;
    quint16 umbral3;
    quint8 nivel;
    QString mensaje;
    bool sndOn;
    bool apagado;
    bool cambio;
    // ********

    bool getEstado(void) {
        return (contador>=1);
    }
};

class SndState: public QObject
{
    Q_OBJECT

signals:
    void setSendNewAlarmSnd(quint8 nivel);
public slots:
    void checkNewSndState(quint16 nuevaAlarma);
public:
    SndState(QPlainTextEdit *);

private:
    bool OnOff;
    uint8_t nivel;
    alarms misAlarmas[(int)TIPOSALARMA::Count];
    QPlainTextEdit *mensajesAlarma;

    quint8 maxNivelCambioOn(void);
};


#endif // ALARMS_H
