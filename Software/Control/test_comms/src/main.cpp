#include <Arduino.h>
#include "comunicaciones.h"
#include <../.pio/libdeps/due/DueTimer/DueTimer.h>
#include "respirador.h"


bool llamadaCiclo = false;
extern estadoStruct miEstadoRespiracion;
extern estadoMaquinaStruct miEstadoMaquina;


extern sendDataStruct datosEnviar;

void ejecucionPeriodica() {
    if (miEstadoMaquina.estado == MachineState::ON)
        llamadaCiclo = true;
}

void setup() {
    Serial.begin(115200);
    Timer1.setPeriod(TICK*1000); //in microseconds
    Timer1.attachInterrupt(ejecucionPeriodica);  // funcion a llamar
    Timer1.start();
    miEstadoMaquina = {MachineState::RESET};
    miEstadoRespiracion  = {0, 0.05, 5, State::INSPIRATION}; // R5C50 en mmH2O / (ml/s); y en ml / mmH2O (0,05; 5)
    datosEnviar = {(uint8_t) FrameType::DATA, 0, 0, 0 };
}

void loop() {
    if (llamadaCiclo){
        llamadaCiclo = false;
        cicloControl();
    };
}

