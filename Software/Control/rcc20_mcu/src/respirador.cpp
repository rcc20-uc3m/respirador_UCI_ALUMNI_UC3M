#include <Arduino.h>
#include "respirador.h"
#include "comunicaciones.h"
#include "parametros.h"
#include "sensores.h"
#include "inspiracion.h"
#include "espiracion.h"
#include "disparo.h"
#include "valvEspiratoria.h"
#include "valvInspiratoria.h"
#include "alarmas.h"



// Estados de máquina y respiración
estadoMaquinaStruct miEstadoMaquina;
estadoRespiracionStruct miEstadoRespiracion;

// estructuras de datos a enviar
sendDataStruct datosEnviar;
sendDataCycleStruct datosCicloEnviar;

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];
extern uint16_t P_control[];
extern float coefVol;
extern uint16_t pressCalibracion;
extern uint16_t FICalibracion;
extern uint16_t FECalibracion;

uint8_t anterior = 0;
uint8_t actual = 0;
uint16_t flujo_anterior = 0;
double volInsp = 0;

volatile int16_t pf_log[MAX_PF_LOG][3] ;
volatile int16_t pf_log_ptr ;


void init_respirador(){
    init_modos();
    init_sensores();
    ValvInsp_init();
    ValvEsp_init();
    init_inspiracion();
    init_espiracion();
    init_disparo();
    reset_snd_alarma();
}

void actualizar_tiempos(void){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    miEstadoRespiracion.tiempo += P_config[(int) PARAM_CONFIG::Pm];
}

void generaSeniales(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    float omega, tmp;
    float Tc, Ti, Pip, Peep, Tea;
    Tc = (float) miEstadoRespiracion.tiempo;
    Ti = (float) P_calculados[(int) PARAM_CALCULADOS::Ti];
    Tea = (float) P_calculados[(int) PARAM_CALCULADOS::Tea];
    Pip = (float) P_operacion[(int) PARAM_OPERACION::Pip];
    Peep = (float) P_operacion[(int) PARAM_OPERACION::Peep];
    if (miEstadoMaquina.primero){
        String mensaje = "Ti = " + String(Ti) + ": Pip = " + String(Pip) + ": Peep = "+ String(Peep);
        sendDebugString(mensaje);
        miEstadoMaquina.primero = false;
    }
    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
           omega = M_PI_2*Tc/Ti;
           tmp = (Pip - Peep)*sin(omega)+Peep;
           datosEnviar.pressure = (uint16_t) tmp;
           datosEnviar.Esp_Flow = 0;
           datosEnviar.Insp_Flow = 40.0*sin(2*omega);
           break;
        case State::EXPIRATION:
            if (Tc< Tea){
                omega = M_PI_2*(float)(Tc-Ti)/(Tea-Ti);
                tmp = (Pip - Peep)*cos(omega)+Peep;
                datosEnviar.pressure = (uint16_t) tmp;
                datosEnviar.Esp_Flow = 40.0*sin(2*omega);
                datosEnviar.Insp_Flow = 0;
            } else {
                datosEnviar.pressure = Peep;
                datosEnviar.Esp_Flow = 0.0;
                datosEnviar.Insp_Flow = 0;
            }
            break;
        case State::TRIGGER_WINDOW:
            datosEnviar.pressure = Peep;
            datosEnviar.Esp_Flow = 0;
            datosEnviar.Insp_Flow = 0;
    }
    P_control[(int) PARAM_CONTROL::PressMs] = datosEnviar.pressure;
    P_control[(int) PARAM_CONTROL::FIMs] = datosEnviar.Insp_Flow;
    P_control[(int) PARAM_CONTROL::FEMs] = datosEnviar.Esp_Flow;
}

void comprobarCambioEstado(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
            inspiracion_ciclo();
            break;
        case State::EXPIRATION:
            espiracion_ciclo();
            break;
        case State::TRIGGER_WINDOW:
            disparo_ciclo();
            break;
    }
}

void realizarIntegracion(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    uint16_t tmp;
    String mensaje;
    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
            ajusteMinCuadradosCyR();
            tmp = P_control[(int) PARAM_CONTROL::FIMs];
            if (tmp > 0){
                volInsp += (flujo_anterior+tmp)*coefVol;
                flujo_anterior = tmp;
            }
            tmp = P_control[(int)PARAM_CONTROL::FIMs];
            if (tmp > P_ciclo[(int)PARAM_CICLO::FI_max]) {
                P_ciclo[(int) PARAM_CICLO::FI_max] = tmp;
            }
            break;
        case State::EXPIRATION:
            tmp = P_control[(int)PARAM_CONTROL::FEMs];
            if (tmp > P_ciclo[(int)PARAM_CICLO::FE_max]) P_ciclo[(int)PARAM_CICLO::FE_max] = tmp;
            break;
        case State::TRIGGER_WINDOW:
            break;
    }
}

void calcularParametrosCiclo(){
    // Aqui se calcula Ppico y Pmeseta
    // Vinsp, FI_max y FE_max se calculan en realizarIntegracion
    // peepM se calcula en reset_inspiracion
    // Comp, y Res, se calculan iterativamente en control
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    static bool primero = true;
    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
            if (P_control[(int) PARAM_CONTROL::PressMs] >= P_ciclo[(int)PARAM_CICLO::Ppico])
                P_ciclo[(int)PARAM_CICLO::Ppico] = P_control[(int) PARAM_CONTROL::PressMs];
            break;
        case State::EXPIRATION:
            if (primero){
                P_ciclo[(int)PARAM_CICLO::Pplateau] = P_control[(int) PARAM_CONTROL::PressMs];
                if (P_ciclo[(int)PARAM_CICLO::Pplateau] >  P_ciclo[(int)PARAM_CICLO::Ppico])
                    P_ciclo[(int)PARAM_CICLO::Ppico] =  P_ciclo[(int)PARAM_CICLO::Pplateau];
                P_ciclo[(int)PARAM_CICLO::Resistencia] = calc_resistencia();
                P_ciclo[(int)PARAM_CICLO::Complianza] = calc_complianza();
                primero = false;
            }
            break;
        case State::TRIGGER_WINDOW:
            primero = true;
            break;
    }
}

void hacerControl(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    switch (miEstadoRespiracion.estado) {
        case State::INSPIRATION:
            inspiracion_control();
            break;
        case State::EXPIRATION:
            espiracion_control();
            break;
        case State::TRIGGER_WINDOW:
            disparo_control();
            break;
    }
    if (miEstadoMaquina.primero) miEstadoMaquina.primero = false;
}

void enviarDatos(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    actual = miEstadoRespiracion.tiempo / P_config[(int) PARAM_CONFIG::Pc];
    if (actual != anterior){
        datosEnviar.pressure = P_control[(int) PARAM_CONTROL::PressMs];
        datosEnviar.Insp_Flow = P_control[(int) PARAM_CONTROL::FIMs];
        datosEnviar.Esp_Flow = P_control[(int) PARAM_CONTROL::FEMs];
        datosEnviar.FiO2 = P_control[(int) PARAM_CONTROL::FiO2];
//        datosEnviar.alarma = P_control[(int) PARAM_CONTROL::Alarma];
        sendData(datosEnviar);
        anterior = actual;
    }
}

void escribirLog(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    if((miEstadoRespiracion.tiempo & PF_LOG_MASK) == 0) { // 32ms
        if(pf_log_ptr >= MAX_PF_LOG) pf_log_ptr = 0 ;
        pf_log[pf_log_ptr][0] = P_control[(int)PARAM_CONTROL::PressMs];
        pf_log[pf_log_ptr][1] = P_control[(int)PARAM_CONTROL::FIMs];
        pf_log[pf_log_ptr][2] = P_control[(int)PARAM_CONTROL::FEMs];
        pf_log_ptr += 1 ; // next position to write
    }
}

void enviarDatosCiclo(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    datosCicloEnviar.Ppico =  P_ciclo[(int) PARAM_CICLO::Ppico];
    datosCicloEnviar.Pplateau = P_ciclo[(int) PARAM_CICLO::Pplateau];
    datosCicloEnviar.Vinsp = volInsp;
    datosCicloEnviar.peepM = P_ciclo[(int) PARAM_CICLO::peepM];
    datosCicloEnviar.Comp = P_ciclo[(int) PARAM_CICLO::Complianza];
    datosCicloEnviar.Res = P_ciclo[(int) PARAM_CICLO::Resistencia];
    datosCicloEnviar.FImax = P_ciclo[(int) PARAM_CICLO::FI_max];
    datosCicloEnviar.FEmax = P_ciclo[(int) PARAM_CICLO::FE_max];
    datosCicloEnviar.MinVol = P_ciclo[(int) PARAM_CICLO::MinVol];
    datosCicloEnviar.Alarma = P_ciclo[(int) PARAM_CICLO::Alarma];
    sendCycleData(datosCicloEnviar);
}

void reset_machine(){
    ValvInspSetClosed();
    ValvEspSetOpenedN();
    miEstadoRespiracion.estado = State::INSPIRATION;
    miEstadoRespiracion.tiempo = 0;
    // Aquí hay que inicializar Ppico, Pmes, Vinsp, FI_max, FE_max, pero se tienen que guardar los valores de
    // peepM, Complianza y Resistencia
    P_ciclo[(int) PARAM_CICLO::Ppico] = 0;
    P_ciclo[(int) PARAM_CICLO::Pplateau] = 0;
    P_ciclo[(int) PARAM_CICLO::Vinsp] = 0;
    volInsp = 0.0;
    P_ciclo[(int) PARAM_CICLO::FI_max] = 0;
    P_ciclo[(int) PARAM_CICLO::FE_max] = 0;
    String mens = "RESET MACHINE";
    sendDebugString(mens);
    reset_snd_alarma();
}



void cicloControl(){
    if (miEstadoMaquina.estado == MachineState::OFF) return;
    if (miEstadoMaquina.primero) ValvEspSetClosed();
    run_snd_alarmas();
    actualizar_tiempos();
    actualizarValoresControl();
#if DEBUG
    generaSeniales();
#endif
    escribirLog();
    realizarIntegracion();
    calcularParametrosCiclo();
    comprobarCambioEstado();
    hacerControl();
    enviarDatos();
}

void cicloCalibracion(){
    actualizarValoresCalibracion();
    sendCalibrationData();
}