#include <Arduino.h>
#include "respirador.h"
#include "comunicaciones.h"
#include "parametros.h"

// Estados de máquina y respiración
estadoMaquinaStruct miEstadoMaquina;
estadoStruct miEstadoRespiracion;

// estructuras de datos a enviar
sendDataStruct datosEnviar;
sendDataCycleStruct datosCicloEnviar;

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];


uint8_t anterior = 0;
uint8_t actual = 0;

void actualizar_tiempos(void){
    miEstadoRespiracion.tiempo += TICK;
}

void leerSensores(){
    return;
}

void generaSeniales(){
    float omega, tmp;
    float Tc, Ti, Pip, Peep;
    Tc = (float) miEstadoRespiracion.tiempo;
    Ti = (float) P_calculados[(int) PARAM_CALCULADOS::Ti];
    Pip = (float) P_operacion[(int) PARAM_OPERACION::Pip];
    Peep = (float) P_operacion[(int) PARAM_OPERACION::Peep];

    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
           /* omega = M_PI_2*Tc/Ti;
            tmp = (Pip - Peep)*(sin(omega))+Peep;
            datosEnviar.pressure = (uint16_t) tmp;
            datosEnviar.Esp_Flow = 0;
            datosEnviar.Insp_Flow = 40.0*sin(2*omega);*/
           datosEnviar.pressure = 200;
           datosEnviar.Insp_Flow = 50;
           datosEnviar.Esp_Flow = 0;
           break;
        case State::EXPIRATION:
            if (Tc< 2*Ti){
                /*omega = M_PI_2*(float)(Tc-Ti)/Ti;
                tmp = (Pip - Peep)*(cos(omega))+Peep;
                datosEnviar.pressure = (uint16_t) tmp;
                datosEnviar.Esp_Flow = 40.0*sin(2*omega);
                datosEnviar.Insp_Flow = 0;*/
                datosEnviar.pressure = 50;
                datosEnviar.Insp_Flow = 0;
                datosEnviar.Esp_Flow = 50;
            } else {
                /*datosEnviar.pressure = Peep;
                datosEnviar.Esp_Flow = 0.0;*/
                datosEnviar.pressure = 0;
                datosEnviar.Insp_Flow = 0;
                datosEnviar.Esp_Flow = 0;
            }
            break;
        case State::TRIGGER_WINDOW:
            datosEnviar.pressure = 0;
            datosEnviar.Esp_Flow = 0;
            datosEnviar.Insp_Flow = 0;
    }
}

void reset_inspiracion() {
    miEstadoRespiracion.tiempo = 0;
    enviarDatosCiclo();
    P_ciclo[(int) PARAM_CICLO::Ppico] = 0;
    P_ciclo[(int) PARAM_CICLO::Pplateau] = 0;
    P_ciclo[(int) PARAM_CICLO::Vinsp] = 0;
    P_ciclo[(int) PARAM_CICLO::peepM] = 0;
    P_ciclo[(int) PARAM_CICLO::Complianza] = 0;
    P_ciclo[(int) PARAM_CICLO::Resistencia] = 0;
    //resp->valvEsp->setClosed();
}

void comprobarCambioEstado(){
    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
            if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Ti]){
                miEstadoRespiracion.estado = State::EXPIRATION;
                //reset_espiracion();
            }
            break;
        case State::EXPIRATION:
            if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Tea]){
                miEstadoRespiracion.estado = State::TRIGGER_WINDOW;
                //reset_trigger();
            }
            break;
        case State::TRIGGER_WINDOW:
            if (miEstadoRespiracion.tiempo >= P_calculados[(int) PARAM_CALCULADOS::Pr] ){
                miEstadoRespiracion.estado = State::INSPIRATION;
                reset_inspiracion();
            }
            break;
    }
}

void calcularParametrosCiclo(){
    return;
}

void hacerControl(){
    return;
}

void enviarDatos(){
    actual = miEstadoRespiracion.tiempo / COMMS_PERIOD;
    if (actual != anterior){
        sendData((uint8_t *) &datosEnviar);
        anterior = actual;
    }
}

void enviarDatosCiclo(){
    datosCicloEnviar.commando = (uint8_t) FrameType::DATACYCLE;
    datosCicloEnviar.Ppico =  P_ciclo[(int) PARAM_CICLO::Ppico];
    datosCicloEnviar.Pplateau = P_ciclo[(int) PARAM_CICLO::Pplateau];
    datosCicloEnviar.Vinsp = P_ciclo[(int) PARAM_CICLO::Vinsp];
    datosCicloEnviar.peepM = P_ciclo[(int) PARAM_CICLO::peepM];
    datosCicloEnviar.Comp = P_ciclo[(int) PARAM_CICLO::Complianza];
    datosCicloEnviar.Res = P_ciclo[(int) PARAM_CICLO::Resistencia];
}

void cicloControl(){
    actualizar_tiempos();
    leerSensores();
#if DEBUG
    generaSeniales();
#endif
    comprobarCambioEstado();
    calcularParametrosCiclo();
    hacerControl();
    enviarDatos();
}