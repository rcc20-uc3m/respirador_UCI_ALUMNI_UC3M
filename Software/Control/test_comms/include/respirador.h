//
// Created by diego on 4/2/21.
//

#ifndef T2_RESPIRADOR_H
#define T2_RESPIRADOR_H
#include "stdint.h"

#define TICK 1 // tick in ms for calculating the time increment
#define COMMS_PERIOD 10 // data sent each COMMS_PERIOD TICKs

struct sendDataStruct {
    uint16_t pressure; // pressure in mmH2O 1 mmH2O = 0,0980665 mbar approx 0,1 mbar
    uint16_t Insp_Flow; // flow in ml/s
    uint16_t Esp_Flow; // flow in ml/s
}__attribute__((packed));;

enum class State{INSPIRATION, EXPIRATION, TRIGGER_WINDOW};


struct estadoStruct {
    uint16_t tiempo;
    enum State estado;
};

void cicloControl();

#endif //T2_RESPIRADOR_H
