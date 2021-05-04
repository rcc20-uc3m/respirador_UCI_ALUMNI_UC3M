//
// Created by diego on 4/2/21.
//

#ifndef T2_RESPIRADOR_H
#define T2_RESPIRADOR_H
#include "stdint.h"
#include "modos.h"

#define TICK 1 // tick in ms for calculating the time increment
#define COMMS_PERIOD 10 // data sent each COMMS_PERIOD TICKs

#define DEBUG 0


// pressure and flow log - every 32ms - ~32 per second
#define MAX_PF_LOG 1200
#define PF_LOG_MASK 0x1F
#define PF_LOG_PEEP_OFFSET 4 // reach back 4 entries to get PEEP


// Estado de la respiración
enum class State{INSPIRATION, EXPIRATION, TRIGGER_WINDOW};
// Estado del respirador como máquina
enum class MachineState{RESET, OFF, ON, CAL};

// estructura de datos para el estado de la respiración
struct estadoRespiracionStruct {
    uint16_t tiempo;
    float R;
    float C;
    bool mandatoria;
    bool disparado;
    enum State estado;
};

// estructura de datos para el estado de la máquina
struct estadoMaquinaStruct{
    bool primero;
    bool ultimo;
    uint8_t desconexiones;
    enum MachineState estado;
    enum MODOS modo;
    enum TRIGGER disparo;
};

// estructura de datos para el envio de los datos instantáneos
struct sendDataStruct {
    uint16_t pressure; // pressure in mmH2O 1 mmH2O = 0,0980665 mbar approx 0,1 mbar
    uint16_t Insp_Flow; // flow in ml/s
    uint16_t Esp_Flow; // flow in ml/s
    uint16_t FiO2;
};

// estructura de datos para el envio de los parámetros del ciclo
struct sendDataCycleStruct{
    uint16_t Ppico;
    uint16_t Pplateau;
    uint16_t Vinsp;
    uint16_t peepM;
    uint16_t Comp;
    uint16_t Res;
    uint16_t FImax;
    uint16_t FEmax;
    uint16_t MinVol;
    uint16_t Alarma;
};

// funciones de control del respirador
void cicloControl();
void cicloCalibracion();
void actualizar_tiempos();
void generaSeniales();
void comprobarCambioEstado();
void calcularParametrosCiclo();
void realizarIntegracion();
void hacerControl();
void enviarDatos();
void enviarDatosCiclo();
void init_respirador();
void escribirLog();
void reset_machine();

#endif //T2_RESPIRADOR_H
