//
// Created by diego on 12/2/21.
//
#include "Arduino.h"        //borrar en cuanto quitemos mensajes de depuración
#include "comunicaciones.h" //borrar en cuanto quitemos mensajes de depuración
#include "disparo.h"
#include "stdint.h"
#include "modos.h"
#include "parametros.h"
#include "respirador.h"
#include "inspiracion.h"
#include "valvInspiratoria.h"
#include "valvEspiratoria.h"

#define TRIGGER_BASE_FLOW P_config[(int) PARAM_CONFIG::Trig_base_flow]
#define RETRASO_TRIGGER_FLOW    128

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];
extern uint16_t P_control[];
extern estadoMaquinaStruct miEstadoMaquina;
extern estadoRespiracionStruct miEstadoRespiracion;

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];
extern uint16_t P_control[];
extern estadoMaquinaStruct miEstadoMaquina;
extern estadoRespiracionStruct miEstadoRespiracion;
extern ModoVentilador misModos[];

extern int16_t pf_log[][3] ;
extern int16_t pf_log_ptr ;

extern double volInsp;

uint16_t t0disparo = 0;

void (*disparo_ciclado_funciones[(int) DISPAROS::Count])(void);

void disparo_control(void){
    if (miEstadoMaquina.disparo == TRIGGER::Flujo) {
        pid_control_press(P_control[(int) PARAM_CONTROL::PressMs], P_ciclo[(int) PARAM_CICLO::peepM]);
    }
}

void disparo_ciclo(void){
    DISPAROS miDisparo;
    if(miEstadoRespiracion.mandatoria)
        miDisparo = misModos[(int) miEstadoMaquina.modo].Mandatoria.disparo;
    else
        miDisparo = misModos[(int) miEstadoMaquina.modo].Soporte.disparo;
    disparo_ciclado_funciones[(int) miDisparo]();
}

void disparo_tiempo(void){
    if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Pr]){
        miEstadoRespiracion.estado = State::INSPIRATION;
        miEstadoRespiracion.disparado = false; // trigger tiempo
        miEstadoRespiracion.mandatoria = true; // respiración mandatoria
        String mensaje = "Cambiado a Inspiración";
        sendDebugString(mensaje);
        reset_inspiracion();
    }
}

void disparo_unaVentana(void){
    if (miEstadoMaquina.disparo == TRIGGER::Presion){
        uint16_t valPress, valPressL, triggerLvl;
        valPress = P_control[(int)PARAM_CONTROL::PressMs];
        valPressL = get_LongPress();
        triggerLvl = P_config[(int)PARAM_CONFIG::TrigLvl_Press];
        if (valPress + triggerLvl < valPressL){
            miEstadoRespiracion.estado = State::INSPIRATION;
            miEstadoRespiracion.disparado = true; // trigger por presión
            miEstadoRespiracion.mandatoria = true; // una sola ventana -> todas las respiraciones son mandatorias
            reset_inspiracion();
        }
    } else if (miEstadoMaquina.disparo == TRIGGER::Flujo){
        if (miEstadoRespiracion.tiempo - t0disparo >= RETRASO_TRIGGER_FLOW) {
            uint16_t valFlow, valFlowL, triggerLvl;
            valFlow = P_control[(int) PARAM_CONTROL::FEMs];
            valFlowL = get_LongFE();
            triggerLvl = P_config[(int) PARAM_CONFIG::TrigLvl_Flow];

            if (valFlow + triggerLvl <= valFlowL) {
                String mens = "TriggerFlow: valFlow = " + String(valFlow) + " valFlowL = " + String(valFlowL) +
                              "  suma = " + String(valFlow + triggerLvl);
                sendDebugString(mens);
                miEstadoRespiracion.estado = State::INSPIRATION;
                miEstadoRespiracion.disparado = true; // trigger por flujo
                miEstadoRespiracion.mandatoria = true; // una sola ventana -> todas las respiraciones son mandatorias
                reset_inspiracion();
            }
        }
    }
    // Siempre se tiene el backup del disparo por tiempo
    disparo_tiempo();
}

void disparo_dosVentana(void){
    String mens;
    if (miEstadoMaquina.disparo == TRIGGER::Presion){
        uint16_t valPress, valPressL, triggerLvl;
        valPress = P_control[(int)PARAM_CONTROL::PressMs];
        valPressL = get_LongPress();
        triggerLvl = P_config[(int)PARAM_CONFIG::TrigLvl_Press];
        if (valPress+triggerLvl <= valPressL){
            mens = "valPress = " + String(valPress) + "  triggerLvl = " + String(triggerLvl) +
                   "  valPressL = " + String(valPressL);
            sendDebugString(mens);
            miEstadoRespiracion.estado = State::INSPIRATION;
            miEstadoRespiracion.disparado = true; // trigger por presión
            if (miEstadoRespiracion.tiempo >= P_calculados[(int)PARAM_CALCULADOS::Tedm])
                miEstadoRespiracion.mandatoria = true; // respiración mandatoria siempre en segunda ventana
            else
                miEstadoRespiracion.mandatoria = false; // respiración espontánea en primera ventana
            reset_inspiracion();
        }
    } else {
        if (miEstadoRespiracion.tiempo - t0disparo >= RETRASO_TRIGGER_FLOW) {
            uint16_t valFlow, valFlowL, triggerLvl;
            valFlow = P_control[(int) PARAM_CONTROL::FEMs];
            valFlowL = get_LongFE();
            triggerLvl = P_config[(int) PARAM_CONFIG::TrigLvl_Flow];
            if (valFlow + triggerLvl <= valFlowL) {
                miEstadoRespiracion.estado = State::INSPIRATION;
                miEstadoRespiracion.disparado = true; // trigger por flujo
                if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Tedm])
                    miEstadoRespiracion.mandatoria = true; // respiración mandatoria siempre en segunda ventana
                else
                    miEstadoRespiracion.mandatoria = false; // respiración espontánea en primera ventana
                reset_inspiracion();
            }
        }
    }
    // Siempre se tiene el backup del disparo por tiempo
    disparo_tiempo();
}

void init_disparo(void){
    disparo_ciclado_funciones[(int) DISPAROS::Tiempo] = disparo_tiempo;
    disparo_ciclado_funciones[(int) DISPAROS::UnaVentana] = disparo_unaVentana;
    disparo_ciclado_funciones[(int) DISPAROS::DosVentanas] = disparo_dosVentana;
}


void reset_disparo(){
//    if (miEstadoMaquina.disparo == TRIGGER::Flujo){
////        pid_set_fast_press();
//        ValvInspResetPID();
//        t0disparo = miEstadoRespiracion.tiempo;
//        ValvEspSetPos(TRIGGER_BASE_FLOW);
//    } else {
        t0disparo = miEstadoRespiracion.tiempo;
        ValvEspSetPos(0);
//    }
}


int16_t get_LongPress(){
    int16_t i;
    i = pf_log_ptr - 2;
    if (i < 0) i+= MAX_PF_LOG;
    return (pf_log[i][0]);
}

int16_t get_LongFE(){
    int16_t i;
    i = pf_log_ptr - 2;
    if (i < 0) i+= MAX_PF_LOG;
    return (pf_log[i][2]);
}

