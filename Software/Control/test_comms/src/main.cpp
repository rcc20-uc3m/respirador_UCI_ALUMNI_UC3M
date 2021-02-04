#include <Arduino.h>
#include "comunicaciones.h"
#include <../.pio/libdeps/due/DueTimer/DueTimer.h>
#include "respirador.h"

bool llamadaCiclo = false;

void ejecucionPeriodica() {
    llamadaCiclo = true;
}

void setup() {
    Serial.begin(115200);
    Timer1.setPeriod(TICK*1000); //in microseconds
    Timer1.attachInterrupt(ejecucionPeriodica);  // funcion a llamar
    Timer1.start();
}

void loop() {
    if (llamadaCiclo){
        llamadaCiclo = false;
        cicloControl();
    };
}

