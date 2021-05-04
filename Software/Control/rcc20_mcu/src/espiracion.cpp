//
// Created by diego on 12/2/21.
//
#include "Arduino.h"        //borrar en cuanto quitemos mensajes de depuración
#include "comunicaciones.h" //borrar en cuanto quitemos mensajes de depuración
#include "stdint.h"
#include <cmath>
#include "modos.h"
#include "parametros.h"
#include "respirador.h"
#include "espiracion.h"
#include "disparo.h"
#include "valvInspiratoria.h"
#include "valvEspiratoria.h"

extern uint16_t P_hardware[];

// si diferencia entre peepM y peep es mayor o menor que este valor, se incrementa o decrementa peep_close
#define PS_EPS P_hardware[(int)PARAM_HARDW::PS_EPS]
// cantidad mínima que se incrementa o decrementa el peep_close
#define PS_INC P_hardware[(int)PARAM_HARDW::PS_INC]
// Si el RC es muy alto (gran resistencia, tengo que poner un tiempo entre llegar al punto de cierre (peep_close) y
// el momento de cerrar. La P medida puede llegar incluso a 0, pero eso no quiere decir que la peep haya llegado a
// cero en el pulmón. Por cada mmH2O medido de más, se añaden 40 ms (en realidad son ciclos de muestreo)
#define PS_INC_TIMER P_hardware[(int)PARAM_HARDW::PS_INC_TIMER]
// Para calcular el incremento o decremento del peep_close, uso el valor a continuación
// se trata de mmH2O a subir o bajar en funciónn de los mmH2O de diferencia
#define PS_INC_PEEP_CLOSE (float) (P_hardware[(int)PARAM_HARDW::PS_INC_PEEP_CLOSE]/100.0)
// Si el RC es muy bajo, se vacía muy rápidamente el pulmón, y tengo que ir subiendo el punto en el cual se cierra
// la válvula, peep_close sube hasta el valor de la presión Pip o plateau, menos una cierta cantidad que se establece
// en PS_PORDEBAJOPIP
#define PS_PORDEBAJOPIP P_hardware[(int)PARAM_HARDW::PS_PORDEBAJODEPIP]
// El valor de peep_close se inicializa al valor del peep, menos la siguiente cantidad
#define PS_PEEP_MENOS P_hardware[(int) PARAM_HARDW::PS_PEEP_MENOS]
// Valor del retraso para determinar cuando deja de caer la presión en la espiración después de cerrar la Vesp
#define PS_CAIDA_TIMER P_hardware[(int)PARAM_HARDW::PS_CAIDA_TIMER]
// Valor del retraso para determinar cuando deja de subir la presión en la espiración después de cerrar la Vesp
#define PS_SUBIDA_TIMER P_hardware[(int)PARAM_HARDW::PS_SUBIDA_TIMER]
// Valor mínimo del peep. Como el ajuste es 0, 5 -> continuo, menos de 2.5 se considera peep nula (no ha sido capaz
// el sistema de regular la peep (es un RC bajo)
#define PS_PEEP_CLOSE_MIN P_hardware[(int)PARAM_HARDW::PS_PEEP_CLOSE_MIN]
// valor de Dt/DP en la espiración para determinar cuando pasamos de RCBAJO a normal
#define PS_LIMITE_RCBAJO (float)(P_hardware[(int) PARAM_HARDW::PS_LIMITE_RCBAJO])

extern int16_t pf_log[][3] ;
extern int16_t pf_log_ptr ;

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];
extern uint16_t P_control[];
extern estadoMaquinaStruct miEstadoMaquina;
extern estadoRespiracionStruct miEstadoRespiracion;
extern ModoVentilador misModos[];

int16_t peep_close, peep_time, peep_timer_stop;
int16_t caida_timer, subida_timer;
uint16_t oldpress, minpress, FEatminpress, maxpress;
uint16_t t1, p1, t0, p0;
bool flagF = false;

enum ESTADOESPIRACION estadoEspiracion;
enum TIPOESPIRACION tipoEspiracion;

void (*espiracion_ciclado_funciones[(int) ESPERAS::Count])(void);

void espiracion_control(void){
    if (P_config[(int)PARAM_CONFIG::NEWALGORITHM]==1){
        pid_control_expiration(P_control[(int)PARAM_CONTROL::PressMs],
                               P_operacion[(int)PARAM_OPERACION::Peep]);
    } else {
        switch (tipoEspiracion) {
            case TIPOESPIRACION::RCBAJO:
            case TIPOESPIRACION::RCNORMAL:
                control_rc_normal();
                break;
            case TIPOESPIRACION::RCALTO:
                control_rc_alto();
                break;
        }
    }
}

void actualizar_peep(){
    String mens = "Actualizo Peep";
    sendDebugString(mens);
    switch (tipoEspiracion) {
        case TIPOESPIRACION::RCBAJO:
        case TIPOESPIRACION::RCNORMAL:
            actualizo_peep_rc_normal();
            break;
        case TIPOESPIRACION::RCALTO:
            actualizo_peep_rc_alto();
            break;
    }
}

void actualizo_peep_rc_normal(){
    String mens;
    int16_t diferencia;
    float tmp;
    uint16_t p_referencia;

    if (miEstadoMaquina.modo == MODOS::PC_CMV || miEstadoMaquina.modo == MODOS::PC_SIMV || miEstadoMaquina.modo == MODOS::PC_AC)
        p_referencia = P_operacion[(int)PARAM_OPERACION::Pip];
    else
        p_referencia = P_ciclo[(int)PARAM_CICLO::Pplateau];

    if ((P_ciclo[(int)PARAM_CICLO::peepM] < PS_PEEP_CLOSE_MIN) &&
        (P_operacion[(int)PARAM_OPERACION::Peep] > 0) &&
        (peep_close>0) &&
        (tipoEspiracion != TIPOESPIRACION::RCBAJO)){
        mens = "PASO A RCBAJO**";
        sendDebugString(mens);
        tipoEspiracion = TIPOESPIRACION::RCBAJO;
        peep_close = P_operacion[(int)PARAM_OPERACION::Peep] - PS_PEEP_MENOS;
        return;
    } else if ((P_ciclo[(int)PARAM_CICLO::peepM] < PS_PEEP_CLOSE_MIN) &&
               (P_operacion[(int)PARAM_OPERACION::Peep] > 0) &&
               (peep_close>0) &&
               (tipoEspiracion == TIPOESPIRACION::RCBAJO)){
        ValvEspDecOpenF();
    }
    diferencia = P_ciclo[(int)PARAM_CICLO::peepM] - P_operacion[(int)PARAM_OPERACION::Peep];

    if ( (diferencia > PS_EPS) && (peep_close == PS_PEEP_CLOSE_MIN)) {
        tipoEspiracion = TIPOESPIRACION::RCALTO; // cierro en P = 0, y lo que recupero es mayor que el peep
        mens = "PASO A RCALTO";
        sendDebugString(mens);
        peep_timer_stop = diferencia*PS_INC_TIMER;
    } else if (diferencia > PS_EPS){
        tmp = PS_INC_PEEP_CLOSE*diferencia;
        peep_close = peep_close - max(tmp, PS_INC);
        mens = "quito peep_close: tmp = " + String(tmp) + "  :peep_close = " + String(peep_close);
        sendDebugString(mens);
        if (peep_close <= PS_PEEP_CLOSE_MIN) {
            peep_close = PS_PEEP_CLOSE_MIN;
            tipoEspiracion = TIPOESPIRACION::RCALTO;
            peep_timer_stop = diferencia*PS_INC_TIMER;
        }
    } else if (diferencia < - PS_EPS) {
        tmp = - PS_INC_PEEP_CLOSE*diferencia;
        peep_close =peep_close + max(tmp, PS_INC);
        mens = "pongo peep_close: tmp = " + String(tmp) + "  :peep_close = " + String(peep_close);
        sendDebugString(mens);
        if (peep_close >= p_referencia - PS_PORDEBAJOPIP) {
            peep_close = p_referencia - PS_PORDEBAJOPIP;
            if (tipoEspiracion != TIPOESPIRACION::RCBAJO){
                tipoEspiracion = TIPOESPIRACION::RCBAJO;
                peep_close = P_operacion[(int)PARAM_OPERACION::Peep] - PS_PEEP_MENOS;
            } else {
                ValvEspDecOpenF();
            }

        }
    }
}

void actualizo_peep_rc_alto(){
    String mens;
    int16_t diferencia;
    float tmp;

    diferencia = P_ciclo[(int) PARAM_CICLO::peepM] - P_operacion[(int) PARAM_OPERACION::Peep];

    if (diferencia > PS_EPS) {
        tmp = diferencia *PS_INC_TIMER;
        peep_timer_stop += tmp;
        mens = "pongo peep_timer_stop: tmp = " + String(tmp) + "  :peep_timer_stop = " + String(peep_timer_stop);
        sendDebugString(mens);
        // TODO poner un límite superior.
        // Si el tiempo absoluto es mayor que Tie, entonces podría ver si estoy
        // en apertura N (70). Si no, poner a N. En caso contrario quizá pueda
        // abrir más la válvula espiratoria (subir de 70).
    } else if (diferencia < -PS_EPS) {
        float tmp = diferencia *PS_INC_TIMER;
        peep_timer_stop += tmp; // tmp es negativo ya que diferencia es negativa
        mens = "quito peep_timer_stop: tmp = " + String(tmp) + "  :peep_timer_stop = " + String(peep_timer_stop);
        sendDebugString(mens);
        if (peep_timer_stop <= 0) {
            peep_timer_stop = 0;
            tipoEspiracion = TIPOESPIRACION::RCNORMAL;
            // TODO comprobar que la apertura de la válvula es normal(70)
            // si he estado antes en RC bajo y es menor, ponerlo a Normal
            mens = "PASO A RCNORMAL";
            sendDebugString(mens);
            peep_close = P_operacion[(int) PARAM_OPERACION::Peep] - PS_PEEP_MENOS;
        }
    }
}

void control_rc_normal(){
    String mens;
    switch (estadoEspiracion) {
        case ESTADOESPIRACION::Espera:
            if (P_control[(int)PARAM_CONTROL::PressMs] <= peep_close){
                if (tipoEspiracion == TIPOESPIRACION::RCBAJO){
                    t1 = miEstadoRespiracion.tiempo - t0;
                    p1 = p0 - P_control[(int)PARAM_CONTROL::PressMs];
                    float tmp = t1/p1;
                    if (tmp>PS_LIMITE_RCBAJO) {
                        tipoEspiracion = TIPOESPIRACION::RCNORMAL;
                        mens = "lim pasa rc bajo = " + String(tmp);
                        mens += "\nPASO A RCNORMAL por pendiente???";
                        sendDebugString(mens);
                    }
                }
                ValvEspSetClosed();
                estadoEspiracion = ESTADOESPIRACION::Estable;
                mens = "Cerré en P = " +String(P_control[(int)PARAM_CONTROL::PressMs]);
                sendDebugString(mens);
            }
            break;
        case ESTADOESPIRACION::Estable:
            break;
    }
}

void control_rc_alto(){
    String mens;
    switch (estadoEspiracion) {
        case ESTADOESPIRACION::Espera:
            if (P_control[(int) PARAM_CONTROL::PressMs] < PS_PEEP_CLOSE_MIN) {
//                mens = "Bajinis -  " + String(peep_time)+ " " + String(peep_timer_stop);
//                sendDebugString(mens);
                if (peep_time >= peep_timer_stop) {
                    ValvEspSetClosed();
                    peep_time = 0;
                    estadoEspiracion = ESTADOESPIRACION::Estable;
                    mens = "####Cerré en P = " +String(P_control[(int) PARAM_CONTROL::PressMs]);
                    sendDebugString(mens);
                } else peep_time++;
            }
            break;
        case ESTADOESPIRACION::Estable:
            break;
    }
}


void espiracion_ciclo(void){
    ESPERAS miEspera;
    if(miEstadoRespiracion.mandatoria)
        miEspera = misModos[(int) miEstadoMaquina.modo].Mandatoria.espera;
    else
        miEspera = misModos[(int) miEstadoMaquina.modo].Soporte.espera;
    espiracion_ciclado_funciones[(int) miEspera]();
}

/*
 * Espera basada únicamente en el tiempo. En Parámetros está definido Tea = Ti+(1-TRIG_WDW)*Te;
 * TRIG_WDW es la ventana para el disparo. Por ejemplo si TRIG_WDW = 0.75, A partir del 25% del tiempo de espiración
 * se pasa a estado de disparo. Tea se mide en tiempo absoluto desde el principio de la respiración
 * Si la válvula es lenta o hay mucha resistencia, el valor de TRIG_WDW debería de ser más pequeño.
 */
void espera_tiempo(void) {
    if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Tea]){
        miEstadoRespiracion.estado = State::TRIGGER_WINDOW;
        String mensaje = "Cambiado a Disparo";
        sendDebugString(mensaje);
        reset_disparo();
    }
}

void espera_tiempoEstabilizado(void){
    bool cond1 = (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Tea]);
    bool cond2 = (estadoEspiracion == ESTADOESPIRACION::Estable);
    if (P_config[(int)PARAM_CONFIG::NEWALGORITHM]==1){
        if ( abs(P_control[(int)PARAM_CONTROL::PressMs] - P_operacion[(int)PARAM_OPERACION::Peep]) < 5 &&
                P_control[(int)PARAM_CONTROL::FEMs]<10)
            cond2 = true;
    }
    bool cond3 = (miEstadoRespiracion.tiempo >= P_calculados[(int)PARAM_CALCULADOS::Tedm]);
    if ( (cond1 && cond2) || cond3 ) {
        miEstadoRespiracion.estado = State::TRIGGER_WINDOW;
        String mensaje = "Cambiado a Disparo: C1= " + String(cond1) + " C2= "+String(cond2) + " C3= " + String(cond3);
        sendDebugString(mensaje);
        reset_disparo();
    }
}

void init_espiracion(void){
    espiracion_ciclado_funciones[(int) ESPERAS::Tiempo] = espera_tiempo;
    espiracion_ciclado_funciones[(int) ESPERAS::TiempoEstable] = espera_tiempoEstabilizado;
    if (P_operacion[(int)PARAM_OPERACION::Peep] == 0) peep_close = 0;
    else peep_close = P_operacion[(int) PARAM_OPERACION::Peep] - PS_PEEP_MENOS;
    peep_time = 0;
    peep_timer_stop = 0;
    tipoEspiracion = TIPOESPIRACION::RCNORMAL;
    minpress = 9999;
    maxpress = 0;
}


void reset_espiracion() {
    String mens;
    ValvInspSetClosed();
    ValvEspResetPID();
    if (P_config[(int)PARAM_CONFIG::NEWALGORITHM]==1){
        mens = "NUEVO";
        if(P_ciclo[(int)PARAM_CICLO::Complianza] > P_hardware[(int)PARAM_HARDW::ESPKI3]) {
            ValvEspSetValueN();
            ValvEspSetOpenedN();
            flagF = false;
        }
        else {
            ValvEspSetValueF(P_hardware[(int)PARAM_HARDW::ESPKD3]);
            ValvEspSetOpenedF();
            flagF = true;
        }
        if(P_ciclo[(int)PARAM_CICLO::Resistencia] > P_hardware[(int)PARAM_HARDW::ESPKP3] && !flagF) {
            if (tipoEspiracion != TIPOESPIRACION::RCALTO){
                tipoEspiracion = TIPOESPIRACION::RCALTO;
                pid_set_two_expiration();
            }
        } else {
            if (tipoEspiracion != TIPOESPIRACION::RCNORMAL) {
                tipoEspiracion = TIPOESPIRACION::RCNORMAL;
                pid_set_one_expiration();
            }
        }

    } else {
        mens = "ANTIGUO";
        p0 = P_control[(int) PARAM_CONTROL::PressMs];
        t0 = miEstadoRespiracion.tiempo;
        switch (tipoEspiracion) {
            case TIPOESPIRACION::RCALTO:
            case TIPOESPIRACION::RCNORMAL:
                ValvEspSetOpenedN();
                break;
            case TIPOESPIRACION::RCBAJO:
                ValvEspSetOpenedF();
                break;
        }
        estadoEspiracion = ESTADOESPIRACION::Espera;
        peep_time = 0;
    }
    sendDebugString(mens);
}