#include <Arduino.h>
#include "comunicaciones.h"
#include "DueTimer.h"
#include "respirador.h"
#include "parametros.h"
#include "board.h"


bool llamadaCiclo = false;
bool llamadaCalibracion = false;

extern estadoRespiracionStruct miEstadoRespiracion;
extern estadoMaquinaStruct miEstadoMaquina;
extern sendDataStruct datosEnviar;
extern uint16_t P_config[];

void ejecucionPeriodica() {
    if (miEstadoMaquina.estado == MachineState::ON)
        llamadaCiclo = true;
    else if (miEstadoMaquina.estado == MachineState::CAL)
        llamadaCalibracion = true;
}

void startTimer() {
    Timer1.setPeriod(P_config[(int)PARAM_CONFIG::Pm] * 1000); //in microseconds
    Timer1.attachInterrupt(ejecucionPeriodica);  // funcion a llamar
    Timer1.start();
}

void setup() {
    Serial.begin(115200);
    pinMode(ALARMSND, OUTPUT);
    miEstadoMaquina = {false, false, 0, MachineState::RESET, MODOS::PC_CMV, TRIGGER::None };
    miEstadoRespiracion  = {0, 0.05, 5, false, false, State::INSPIRATION}; // R5C50 en mmH2O / (ml/s); y en ml / mmH2O (0,05; 5)
    datosEnviar = {0, 0, 0, 0};
}

void loop() {
    String mens;
    if (llamadaCiclo){
        llamadaCiclo = false;
        llamadaCalibracion = false;
        cicloControl();
    };
    if (llamadaCalibracion){
        llamadaCalibracion = false;
        llamadaCiclo = false;
        cicloCalibracion();
    }
}

