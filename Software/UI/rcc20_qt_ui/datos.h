#ifndef DATOS_H
#define DATOS_H
#include <stdint.h>

enum class FrameType{DATA, DATA_OK, DATACYCLE, DATACYCLE_OK, SENDPARAM, PARAM_OK, ASK_STATE, ANS_STATE,
                     SEND_POPER, POPER_OK, SEND_PCONFIG, PCONFIG_OK, SEND_STATE, STATE_OK, SENDDEBUG, SENDCONFIG, CONFIG_OK,
                     CAL_DATA, CAL_PARAM, CAL_PARAM_OK, SEND_HW, HW_OK, SEND_CAL_POINTS, CAL_POINTS_OK, SET_ALARM,
                     SEND_PALARM, PALARM_OK, Count};

struct sendDataStruct {
    uint16_t pressure; // pressure in mmH2O 1 mmH2O = 0,0980665 mbar approx 0,1 mbar
    uint16_t Insp_Flow; // flow in ml/s
    uint16_t Esp_Flow; // flow in ml/s
    uint16_t FiO2;
};


// estructura de datos para el envio de los parámetros del ciclo
struct sendDataCycleStruct{
    uint16_t Ppico;
    uint16_t  Pplateau;
    uint16_t Vinsp;
    uint16_t peepM;
    uint16_t Comp;
    uint16_t Res;
    uint16_t FImax;
    uint16_t FEmax;
    uint16_t MinVol;
    uint16_t Alarma;
};

enum class MachineState{RESET, OFF, ON, CAL};
enum class MODOS {PC_CMV, PC_AC, VC_CMV, VC_AC, PC_SIMV, VC_SIMV, Count};
enum class TRIGGER {None, Presion, Flujo, Count};

struct estadoMaquinaStruct{
    enum MachineState estado;
    enum MODOS modo;
    enum TRIGGER disparo;
};

#endif // DATOS_H
