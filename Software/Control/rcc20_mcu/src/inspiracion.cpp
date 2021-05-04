//
// Created by diego on 12/2/21.
//
#include "Arduino.h"        //borrar en cuanto quitemos mensajes de depuración
#include "comunicaciones.h" //borrar en cuanto quitemos mensajes de depuración
#include "stdint.h"
#include "modos.h"
#include "parametros.h"
#include "respirador.h"
#include "inspiracion.h"
#include "espiracion.h"
#include "valvInspiratoria.h"
#include "valvEspiratoria.h"
#include "alarmas.h"
#include "board.h"


extern uint16_t P_hardware[];

// VAlor que dice la proporción respecto del valor objetivo para cambiar el pid. Por ejemplo, cuando llegue al
// 70% del valor de la Pip, o al 70% del valor del Vt
#define PS_CAMBIO_PID (float) (P_hardware[(int)PARAM_HARDW::PS_CAMBIO_PID]/100.0)

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];
extern uint16_t P_control[];
extern estadoMaquinaStruct miEstadoMaquina;
extern estadoRespiracionStruct miEstadoRespiracion;
extern ModoVentilador misModos[];
extern double volInsp;

extern int16_t pf_log[][3] ;
extern int16_t pf_log_ptr ;

bool checkSlow = true;
bool lastUPS_State = false;
bool firstBatteryMode = false;
volatile int16_t minvol_log[VOL_MIN_LOG][2];
volatile int16_t minvol_log_ptr;
float A, B, C, D, E;



void (*inspiracion_ciclado_funciones[(int)CICLOS::Count])(void);
void (*inspiracion_control_funciones[(int)CONTROLES::Count])(void);

void inspiracion_control(void){
    CONTROLES miControl;
    if (miEstadoRespiracion.mandatoria)
        miControl = misModos[(int) miEstadoMaquina.modo].Mandatoria.control;
    else
        miControl = misModos[(int) miEstadoMaquina.modo].Soporte.control;
    inspiracion_control_funciones[(int) miControl]();
}

void inspiracion_ciclo(void){
    CICLOS miCiclo;
    if (miEstadoRespiracion.mandatoria)
        miCiclo = misModos[(int) miEstadoMaquina.modo].Mandatoria.ciclo;
    else
        miCiclo = misModos[(int) miEstadoMaquina.modo].Soporte.ciclo;
    inspiracion_ciclado_funciones[(int) miCiclo]();
}

void ciclado_tiempo(void){
    if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Ti]){
        miEstadoRespiracion.estado = State::EXPIRATION;
        String mensaje = "Cambiado a Espiración";
        sendDebugString(mensaje);
        reset_espiracion();
    }
}
void ciclado_flujo(void){
    if (P_control[(int)PARAM_CONTROL::FIMs] < P_config[(int)PARAM_CONFIG::FlowChange]*P_ciclo[(int)PARAM_CICLO::FI_max]/100.0){
        miEstadoRespiracion.estado = State::EXPIRATION;
        reset_espiracion();
    }
}

void control_presion(void){
    if (volInsp >= P_operacion[(int) PARAM_OPERACION::Vt]){
        ValvInspSetClosed();
        return;
    }
    if (P_control[(int)PARAM_CONTROL::PressMs] >= P_operacion[(int)PARAM_OPERACION::Pip]){
        ValvInspSetClosed();
        return;
    }
    if (P_control[(int)PARAM_CONTROL::PressMs] > PS_CAMBIO_PID * P_operacion[(int)PARAM_OPERACION::Pip]) {
        if (checkSlow) {
            pid_set_slow_press();
            checkSlow = false;
        }
    }
    pid_control_press(P_control[(int)PARAM_CONTROL::PressMs],
                      P_operacion[(int)PARAM_OPERACION::Pip]);
}

void control_presionSoporte(void){
    if (volInsp >= P_operacion[(int) PARAM_OPERACION::Vt]){
        ValvInspSetClosed();
        return;
    }
    if (P_control[(int)PARAM_CONTROL::PressMs] >= P_operacion[(int)PARAM_OPERACION::Psop]){
        ValvInspSetClosed();
        return;
    }
    if (P_control[(int)PARAM_CONTROL::PressMs] > PS_CAMBIO_PID * P_operacion[(int)PARAM_OPERACION::Psop])
        if (checkSlow){
            checkSlow=false;
            pid_set_slow_flow();
        }
    pid_control_press(P_control[(int)PARAM_CONTROL::PressMs],
                      P_operacion[(int)PARAM_OPERACION::Psop]);
//    String mens = "Psop";
//    sendDebugString(mens);
}

void control_volumen(void){
//    if (P_control[(int)PARAM_CONTROL::PressMs] > P_operacion[(int)PARAM_OPERACION::Pip] ){
//        P_calculados[(int)PARAM_CALCULADOS::PeakFlow] = (float)(P_calculados[(int)PARAM_CALCULADOS::PeakFlow]/ 2.0);
//        String mens = "PeakFlow = " + String(P_calculados[(int)PARAM_CALCULADOS::PeakFlow]);
//        sendDebugString(mens);
//    }
    if (volInsp < P_operacion[(int) PARAM_OPERACION::Vt]) {
        if (P_control[(int)PARAM_CONTROL::FIMs]>PS_CAMBIO_PID*P_calculados[(int)PARAM_CALCULADOS::PeakFlow])
            if (checkSlow){
                pid_set_slow_flow();
                checkSlow = false;
            }
        pid_control_flow(P_control[(int)PARAM_CONTROL::FIMs], P_calculados[(int)PARAM_CALCULADOS::PeakFlow]);
    } else {
        ValvInspSetClosed();
    }
}

void init_inspiracion(void){
    inspiracion_ciclado_funciones[(int)CICLOS::Tiempo] = ciclado_tiempo;
    inspiracion_ciclado_funciones[(int)CICLOS::Flujo] = ciclado_flujo;
    inspiracion_control_funciones[(int) CONTROLES::Presion] = control_presion;
    inspiracion_control_funciones[(int) CONTROLES::Volumen] = control_volumen;
    inspiracion_control_funciones[(int) CONTROLES::PresionSoporte] = control_presionSoporte;
//    P_ciclo[(int) PARAM_CICLO::Complianza] = P_config[(int)PARAM_CONFIG::C0];
//    P_ciclo[(int) PARAM_CICLO::Resistencia] = P_config[(int)PARAM_CONFIG::R0];
}

int16_t get_peep() {
    int16_t i ;
    if(miEstadoRespiracion.disparado) { // Ha habido un disparo producido por el paciente (press o flujo)
        i = pf_log_ptr - PF_LOG_PEEP_OFFSET ;
        if(i < 0) i += MAX_PF_LOG ;
        return(pf_log[i][0]) ;
    } else { // reach back
        return(P_control[(int)PARAM_CONTROL::PressMs]) ;
    }
}

void reset_inspiracion() {
    double tmp1, tmp2, tmp3;
    tmp1 = tmp2 = tmp3 = 0.0;
    if(minvol_log_ptr >= VOL_MIN_LOG) minvol_log_ptr = 0 ;
    minvol_log[minvol_log_ptr][0] = miEstadoRespiracion.tiempo;
    minvol_log[minvol_log_ptr][1] = volInsp;
    minvol_log_ptr++;

    for (int i=0; i < VOL_MIN_LOG; i++){
        tmp1 += minvol_log[i][0];
        tmp2 += minvol_log[i][1];
    }
    tmp3 = 60000.0*tmp2/tmp1;
    P_ciclo[(int) PARAM_CICLO::MinVol] = (uint16_t) tmp3;
    P_ciclo[(int) PARAM_CICLO::peepM] = get_peep();
    String mens = "peepM = " + String(P_ciclo[(int) PARAM_CICLO::peepM]);
    sendDebugString(mens);
    if (P_config[(int)PARAM_CONFIG::NEWALGORITHM] == 0)
        actualizar_peep();

    if (digitalRead(PRESSALARM))
        reset_alarm_gas_supply(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else
        set_alarm_gas_supply(&P_ciclo[(int)PARAM_CICLO::Alarma]);

    if (digitalRead(BATMODE))
        reset_alarm_power_supply(&P_ciclo[(int) PARAM_CICLO::Alarma]);
    else
        set_alarm_power_supply(&P_ciclo[(int) PARAM_CICLO::Alarma]);

    if (digitalRead(UPS_ALARM))
        set_alarm_low_battery(&P_ciclo[(int) PARAM_CICLO::Alarma]);
    else
        reset_alarm_low_battery(&P_ciclo[(int) PARAM_CICLO::Alarma]);

    if (P_ciclo[(int)PARAM_CICLO::Ppico] > VALOR_PMAX)
        set_alarm_pmax(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_pmax(&P_ciclo[(int)PARAM_CICLO::Alarma]);

    if (P_ciclo[(int) PARAM_CICLO::peepM]>PEEP_MAX(P_operacion[(int)PARAM_OPERACION::Peep]))
        set_alarm_high_peep(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_high_peep(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    if (P_ciclo[(int) PARAM_CICLO::peepM]<PEEP_MIN(P_operacion[(int)PARAM_OPERACION::Peep]))
        set_alarm_low_peep(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_low_peep(&P_ciclo[(int)PARAM_CICLO::Alarma]);

    if (volInsp>VT_MAX(P_operacion[(int)PARAM_OPERACION::Vt]))
        set_alarm_high_volume(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_high_volume(&P_ciclo[(int)PARAM_CICLO::Alarma]);

    if (P_ciclo[(int) PARAM_CICLO::MinVol]>MV_MAX)
        set_alarm_high_mv(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_high_mv(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    if (P_ciclo[(int) PARAM_CICLO::MinVol]<MV_MIN)
        set_alarm_low_mv(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_low_mv(&P_ciclo[(int)PARAM_CICLO::Alarma]);

    CONTROLES miControl;
    if (miEstadoRespiracion.mandatoria)
        miControl = misModos[(int) miEstadoMaquina.modo].Mandatoria.control;
    else
        miControl = misModos[(int) miEstadoMaquina.modo].Soporte.control;

    switch (miControl) {
        case CONTROLES::Presion: {
            if (P_ciclo[(int) PARAM_CICLO::Ppico] < P_operacion[(int) PARAM_OPERACION::Pip] * 0.8) {
                miEstadoMaquina.desconexiones++;
                if (miEstadoMaquina.desconexiones >= N_DISCONNECT)
                    set_alarm_disconnection(&P_ciclo[(int) PARAM_CICLO::Alarma]);
            } else {
                miEstadoMaquina.desconexiones = 0;
                reset_alarm_disconnection(&P_ciclo[(int) PARAM_CICLO::Alarma]);
            }
            break;
        }
        case CONTROLES::PresionSoporte: {
            if (P_ciclo[(int) PARAM_CICLO::Ppico] < P_operacion[(int) PARAM_OPERACION::Psop] * 0.8) {
                miEstadoMaquina.desconexiones++;
                if (miEstadoMaquina.desconexiones >= N_DISCONNECT)
                    set_alarm_disconnection(&P_ciclo[(int) PARAM_CICLO::Alarma]);
            } else {
                miEstadoMaquina.desconexiones = 0;
                reset_alarm_disconnection(&P_ciclo[(int) PARAM_CICLO::Alarma]);
            }
            break;
        }
        case CONTROLES::Volumen: {
            if (P_ciclo[(int) PARAM_CICLO::Ppico] < PIP_DISCONNECT) {
                miEstadoMaquina.desconexiones++;
                if (miEstadoMaquina.desconexiones >= N_DISCONNECT)
                    set_alarm_disconnection(&P_ciclo[(int) PARAM_CICLO::Alarma]);
            } else {
                miEstadoMaquina.desconexiones = 0;
                reset_alarm_disconnection(&P_ciclo[(int) PARAM_CICLO::Alarma]);
            }
            if (volInsp<VT_MIN(P_operacion[(int)PARAM_OPERACION::Vt]))
                set_alarm_low_volume(&P_ciclo[(int)PARAM_CICLO::Alarma]);
            else reset_alarm_low_volume(&P_ciclo[(int)PARAM_CICLO::Alarma]);

            break;
        }
        case CONTROLES::Count:
            break;
    }

/*
    mens = "Peep min = " + String(PEEP_MIN(P_operacion[(int)PARAM_OPERACION::Peep])) ;
    mens += "\nPeep max = " + String(PEEP_MAX(P_operacion[(int)PARAM_OPERACION::Peep])) ;
    mens += "\nVT max = " + String(VT_MAX(P_operacion[(int)PARAM_OPERACION::Vt])) ;
    mens += "\nVT min = " + String(VT_MIN(P_operacion[(int)PARAM_OPERACION::Vt])) ;
    mens += "\nMV max = " + String(MV_MAX) ;
    mens += "\nMV min = " + String(MV_MIN) ;
    mens += "\nPip desc = " + String(PIP_DISCONNECT) ;
    mens += "\nN desc = " + String(N_DISCONNECT) ;
    sendDebugString(mens);
*/

    miEstadoRespiracion.tiempo = 0;
    miEstadoMaquina.primero = true;
    miEstadoMaquina.ultimo = false;
    pid_set_fast_flow();
    pid_set_fast_press();
    checkSlow = true;

    enviarDatosCiclo();
    // Aquí hay que inicializar Ppico, Pmes, Vinsp, FI_max, FE_max, pero se tienen que guardar los valores de
    // peepM, Complianza y Resistencia
    P_ciclo[(int) PARAM_CICLO::Ppico] = 0;
    P_ciclo[(int) PARAM_CICLO::Pplateau] = 0;
    P_ciclo[(int) PARAM_CICLO::Vinsp] = 0;
    volInsp = 0.0;
    P_ciclo[(int) PARAM_CICLO::FI_max] = 0;
    P_ciclo[(int) PARAM_CICLO::FE_max] = 0;
    ValvEspSetClosed();
    ValvInspResetPID();
    A = B = C = D = E = 0;
}

void ajusteMinCuadradosCyR(void){
    A+=P_control[(int)PARAM_CONTROL::FIMs]*volInsp;
    B+=P_control[(int)PARAM_CONTROL::FIMs]*P_control[(int)PARAM_CONTROL::FIMs];
    C+=volInsp*volInsp;
    D+=P_control[(int)PARAM_CONTROL::FIMs]*(P_control[(int)PARAM_CONTROL::PressMs]-P_ciclo[(int)PARAM_CICLO::peepM]);
    E+=volInsp*(P_control[(int)PARAM_CONTROL::PressMs]-P_ciclo[(int)PARAM_CICLO::peepM]);
}

float calc_resistencia(){
    float tmp = 600.0*(D*C-E*A)/(B*C-A*A);
//    String mens = "A = "+String(A) + " B = " + String(B) + " C = "+String(C) + " D = "+String(D)+" E = "+String(E)
//            +"\nR = " + String(tmp);
//    sendDebugString(mens);
    return tmp;
}

float calc_complianza(){
    float tmp = 100.0*(A*A-B*C)/(D*A-E*B);
//    String mens = "A = "+String(A) + " B = " + String(B) + " C = "+String(C) + " D = "+String(D)+" E = "+String(E)
//                  +"\nC = " + String(tmp);
//    sendDebugString(mens);
    return tmp;
}
