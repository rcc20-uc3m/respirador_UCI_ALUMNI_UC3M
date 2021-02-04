#include <Arduino.h>
#include "respirador.h"
#include "comunicaciones.h"

estadoStruct miEstado = {0, State::INSPIRATION};
sendDataStruct datosEnviar = { 0, 0, 0 };
float omega = 0.0;
uint8_t anterior = 0;
uint8_t actual = 0;


void cicloControl(){
    miEstado.tiempo += TICK;
    omega = (float)(M_PI/1500.0)*miEstado.tiempo;
    datosEnviar.pressure = 60*sin(omega);
    if (datosEnviar.pressure < 0) datosEnviar.pressure = 0;
    datosEnviar.Insp_Flow = 100*sin(omega + M_PI_4);
    if (datosEnviar.Insp_Flow < 0) datosEnviar.Insp_Flow = 0;
    datosEnviar.Esp_Flow = 100*cos(omega);
    if (datosEnviar.Esp_Flow < 0) datosEnviar.Esp_Flow = 0;

    actual = miEstado.tiempo / COMMS_PERIOD;
    if (actual != anterior){
        sendData(miEstado.tiempo, (uint8_t *) &datosEnviar);
        anterior = actual;
    }
}