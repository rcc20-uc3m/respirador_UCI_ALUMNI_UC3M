#include "alarms.h"
#include <QDateTime>


SndState::SndState(QPlainTextEdit *miPizarra){
    OnOff = false;
    nivel = NULO;
    this->mensajesAlarma = miPizarra;
    misAlarmas[(int)TIPOSALARMA::PMAX].setUmbrales(0, 0, 0, "Presión mayor de la MÁXIMA"); // media después de 2, alta después de 4
    misAlarmas[(int)TIPOSALARMA::HIGH_PEEP].setUmbrales(2, 0, 3, "PEEP ALTA");
    misAlarmas[(int)TIPOSALARMA::LOW_PEEP].setUmbrales(2, 0, 3, "PEEP BAJA");
    misAlarmas[(int)TIPOSALARMA::HIGH_VOLUME].setUmbrales(2, 0, 3, "Volumen tidal ALTO");
    misAlarmas[(int)TIPOSALARMA::LOW_VOLUME].setUmbrales(2, 0, 3, "Volumen tidal BAJO");
    misAlarmas[(int)TIPOSALARMA::HIGH_MV].setUmbrales(2, 0, 3, "Volumen Minuto ALTO");
    misAlarmas[(int)TIPOSALARMA::LOW_MV].setUmbrales(2, 0, 3, "Volumen Minuto BAJO");
    misAlarmas[(int)TIPOSALARMA::GASS_SUPPLY].setUmbrales(0, 0, 0, "Fallo alimentación GAS"); // alta en la primera ocurrencia
    misAlarmas[(int)TIPOSALARMA::POWER_SUPPLY].setUmbrales(0, 255, 255, "Fallo alimentación ELÉCTRICA"); // media desde la primera hasta la 255
    misAlarmas[(int)TIPOSALARMA::LOW_BATTERY].setUmbrales(0, 0, 0, "Batería BAJA"); // alta en la primera
    misAlarmas[(int)TIPOSALARMA::DISCONNECTION].setUmbrales(0, 0, 0, "DESCONEXIÓN"); // alta en la primera
}

void SndState::checkNewSndState(quint16 nuevaAlarma){
    bool cambio = false;
    QString mens;
    quint8 nuevoNivel = 0;
    for (int i = 0; i < (int)TIPOSALARMA::Count; i++){
        misAlarmas[i].checkNewValue(BIT_CHECK(nuevaAlarma, i));
        if (misAlarmas[i].getCambio()) {
            mens = QDateTime::currentDateTime().toString("h:m:s - dd/MM/yy")+QString(": ") + misAlarmas[i].mensaje+misAlarmas[i].getSimbolo();
            this->mensajesAlarma->appendPlainText(mens);
            cambio = true;
        }
    }
    if (not cambio) return;
    nuevoNivel = maxNivelCambioOn();
    if (nuevoNivel != this->nivel) {
        this->nivel = nuevoNivel;
        emit setSendNewAlarmSnd(nuevoNivel) ;
    }
}

quint8 SndState::maxNivelCambioOn(void){
    quint8 maxnivel = 0;
    for (int i = 0; i < (int)TIPOSALARMA::Count; i++){
        if (misAlarmas[i].cambio){
            if (misAlarmas[i].sndOn || misAlarmas[i].apagado){
                quint8 tmp = misAlarmas[i].nivel;
                if (tmp>maxnivel) maxnivel = tmp;
            }
        }
    }
    return maxnivel;
}

alarms::alarms(){
    this->umbral1 = 0;
    this->umbral2 = 0;
    this->umbral3 = 0;
    this->contador = 0;
    this->nivel = 0;
    this->sndOn = false;
    this->apagado = false;
    this->cambio = false;
}


QString alarms::getSimbolo(){
    if (this->nivel == 1){
        return QString(" !");
    } else if (this->nivel == 2) {
        return QString(" !!");
    } else{
        return QString(" !!!");
    }

}

// Pone los umbrales y pone todo a cero.
void alarms::setUmbrales(quint16 miumbral1, quint16 miumbral2, quint16 miumbral3, QString mens)
{
    this->umbral1 = miumbral1+1;
    this->umbral2 = this->umbral1+miumbral2;
    this->umbral3 = this->umbral2+miumbral3;
    this->mensaje = mens;
    this->contador = 0;
    this->nivel = 0;
    this->sndOn = false;
    this->apagado = false;
    this->cambio = false;
}

// cada vez que llega una respiración, se comprueba el estado de cada alarma
void alarms::checkNewValue(bool nuevoEstado){
    if (not nuevoEstado && not getEstado()){
        contador = 0;
        cambio = false;
        return;
    } else if ( not nuevoEstado && getEstado()) { // estaba en on y ahora off. apago todo
        this->contador = 0; // inicialización gravedad
        this->nivel = NULO;    // el nivel a ninguno
        if (sndOn && nivel == ALTO){ // estaba sonando y era nivel ALTO ->
                                     // no se apaga hasta que lo haga el usuario

            this->apagado = false;
        } else if (sndOn) { // estaba sonando bajo o medio nivel -> apagado alarma
            sndOn = false;
            apagado = false;
        } else if (apagado){ // estaba apagado, por tanto sndOn false.
            apagado = false; // tengo que apagar el timer para evitar que vuelva a sonar
        } else {
            this->cambio = false;
            return;
        }
        this->cambio = true;
        return;
    } else {
        contador++;
        if (contador >= this->umbral3){
            if (nivel < ALTO){
                nivel = ALTO; sndOn = true; apagado = false; cambio = true;
            }
        } else if (contador >= this->umbral2){
            if (nivel < MEDIO){
                nivel = MEDIO; sndOn = true; apagado = false; cambio = true;
            }
        } else if (contador >= this->umbral1){
            if (nivel < BAJO) {
                nivel = BAJO; sndOn = true; apagado = false; cambio = true;
            }
        }
    }
}
