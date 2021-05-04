//
// Created by diego on 1/3/21.
//

#include "Arduino.h"
#include "alarmas.h"
#include "comunicaciones.h"
#include "board.h"

uint8_t pulsos = 0;
uint8_t trenes = 0;

enum ESTADO {TREN, ESPERA};
enum NIVEL {CERO, BAJO, MEDIO, ALTO};

ESTADO miEstadoSonido;
NIVEL miNivel;

bool tono = false;
uint32_t limite_ciclo;
uint32_t contador_ciclo;
uint32_t limite_tren;
uint32_t contador_tren;
uint32_t limite_espera;
uint32_t contador_espera;

void setNivel(NIVEL nuevoNivel){
    miNivel = nuevoNivel;
//    String mens = "Nuevo nivel = "+ String(miNivel);
//    sendDebugString(mens);
}

void setEstadoSonido(ESTADO nuevoEstado){
    miEstadoSonido = nuevoEstado;
}

void init_espera(uint32_t cuantos){
    limite_espera = cuantos;
    contador_espera = 0;
}

bool check_espera(){
    contador_espera++;
    if (contador_espera>=limite_espera){
        contador_espera = 0;
        return true;
    }
    return false;
}

void init_ciclo(uint32_t cuantos){
    limite_ciclo = cuantos;
    contador_ciclo = 0;
}

bool check_ciclo(){
    contador_ciclo++;
    if (contador_ciclo >= limite_ciclo) {
        contador_ciclo = 0;
        return true;
    }
    return false;
}

void init_tren(uint32_t cuantos){
    limite_tren = 2*cuantos;
    tono = false;
    contador_tren = 0;
    miEstadoSonido = TREN;
    init_ciclo(200); // 200 ms
}

bool check_tren(){
    if (check_ciclo()){
        contador_tren++;
        tono = !tono;
        digitalWrite(ALARMSND, tono);
        if (contador_tren >= limite_tren){
            contador_tren = 0;
            return true;
        }
    }
    return false;
}

void run_snd_alarmas(){
    String mens;
//    mens = "check alarma";
//    sendDebugString(mens);
    if (miNivel != CERO) {
//        mens = "Tendría que sonar";
//        sendDebugString(mens);
        if (miEstadoSonido == TREN) {
//            mens = "Esoy en tren";
//            sendDebugString(mens);
            if (check_tren()) {
                miEstadoSonido = ESPERA;
                init_espera(1000);
            }
        } else {
            if (check_espera()) {
                miEstadoSonido = TREN;
                switch (miNivel) {
                    case BAJO:
                        init_tren(1);
                        break;
                    case MEDIO:
                        init_tren(3);
                        break;
                    case ALTO:
                        init_tren(5);
                        break;
                    case CERO:
                        break;
                }
            }
        }
    }
}

void reset_snd_alarma(void){
    tono = false;
    setNivel(CERO);
    digitalWrite(ALARMSND, LOW);
    init_tren(0);
    String mens = "NULA";
    sendDebugString(mens);
}

void lanzar_snd_alarma_alta(void){
    tono = false;
    setNivel(ALTO);
    init_tren(5);
    String mens = "ALTA";
    sendDebugString(mens);
}


void lanzar_snd_alarma_media(void){
    tono = false;
    setNivel(MEDIO);
    init_tren(3);
    String mens = "MEDIA";
    sendDebugString(mens);
}


void lanzar_snd_alarma_baja(void){
    tono = false;
    setNivel(BAJO);
    init_tren(1);
    String mens = "BAJA";
    sendDebugString(mens);
}

void set_alarm_pmax(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_PMAX);
}

void set_alarm_low_peep(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_LOW_PEEP);
}
void set_alarm_high_peep(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_HIGH_PEEP);
}
void set_alarm_high_volume(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_HIGH_VOLUME);
}
void set_alarm_low_volume(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_LOW_VOLUME);
}
void set_alarm_high_mv(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_HIGH_MV);
}
void set_alarm_low_mv(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_LOW_MV);
}
void set_alarm_gas_supply(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_GAS_SUPPLY);
}
void set_alarm_power_supply(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_POWER_SUPPLY);
}
void set_alarm_low_battery(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_LOW_BATTERY);
}
void set_alarm_disconnection(uint16_t *alarma){
    BIT_SET(*alarma,ALARM_DISCONNECTION);
}

void reset_alarm_pmax(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_PMAX);
}

void reset_alarm_low_peep(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_LOW_PEEP);
}
void reset_alarm_high_peep(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_HIGH_PEEP);
}
void reset_alarm_high_volume(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_HIGH_VOLUME);
}
void reset_alarm_low_volume(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_LOW_VOLUME);
}
void reset_alarm_high_mv(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_HIGH_MV);
}
void reset_alarm_low_mv(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_LOW_MV);
}
void reset_alarm_gas_supply(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_GAS_SUPPLY);
}
void reset_alarm_power_supply(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_POWER_SUPPLY);
}
void reset_alarm_low_battery(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_LOW_BATTERY);
}
void reset_alarm_disconnection(uint16_t *alarma){
    BIT_CLEAR(*alarma,ALARM_DISCONNECTION);
}

