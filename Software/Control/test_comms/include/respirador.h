//
// Created by diego on 4/2/21.
//

#ifndef T2_RESPIRADOR_H
#define T2_RESPIRADOR_H
#include "stdint.h"

#define TICK 1 // tick in ms for calculating the time increment
#define COMMS_PERIOD 10 // data sent each COMMS_PERIOD TICKs

#define DEBUG 1

// Estado de la respiración
enum class State{INSPIRATION, EXPIRATION, TRIGGER_WINDOW};
// Estado del respirador como máquina
enum class MachineState{RESET, OFF, ON};

// estructura de datos para el estado de la respiración
struct estadoStruct {
    uint16_t tiempo;
    float R;
    float C;
    enum State estado;
}__attribute__((packed));;

// estructura de datos para el estado de la máquina
struct estadoMaquinaStruct{
    enum MachineState estado;
}__attribute__((packed));;

// estructura de datos para el envio de los datos instantáneos
struct sendDataStruct {
    uint8_t comando;
    uint16_t pressure; // pressure in mmH2O 1 mmH2O = 0,0980665 mbar approx 0,1 mbar
    uint16_t Insp_Flow; // flow in ml/s
    uint16_t Esp_Flow; // flow in ml/s
}__attribute__((packed));;

// estructura de datos para el envio de los parámetros del ciclo
struct sendDataCycleStruct{
    uint8_t commando;
    uint16_t Ppico;
    uint16_t  Pplateau;
    uint16_t Vinsp;
    uint16_t peepM;
    uint16_t Comp;
    uint16_t Res;
}__attribute__((packed));;


// funciones de control del respirador
void cicloControl();
void actualizar_tiempos();
void leerSensores();
void generaSeniales();
void comprobarCambioEstado();
void calcularParametrosCiclo();
void hacerControl();
void enviarDatos();
void enviarDatosCiclo();

void reset_inspiracion();

#endif //T2_RESPIRADOR_H
