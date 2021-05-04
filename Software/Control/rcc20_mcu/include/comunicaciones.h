#include <stdint.h>
#include "respirador.h"

#ifndef T2_COMUNICACIONES_H
#define T2_COMUNICACIONES_H

void sendData(sendDataStruct datos);
void sendCalibrationData(void);
void sendCycleData(sendDataCycleStruct misDatos);
void send_character(uint8_t data);
void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength);
void receive_param(const uint8_t *framebuffer, uint16_t framelength);
void receive_config(const uint8_t *framebuffer, uint16_t framelength);
void receive_calibration_param(const uint8_t* framebuffer, uint16_t framelength);
void receive_calibration_points(const uint8_t* framebuffer, uint16_t framelength);
void receive_alarms_parameters(const uint8_t* framebuffer, uint16_t framelength);
void answer_state(const uint8_t *framebuffer, uint16_t framelength);
void get_configuration_parameters(const uint8_t *framebuffer, uint16_t framelength);
void get_operation_parameters(const uint8_t *framebuffer, uint16_t framelength);
void set_machineState(const uint8_t *framebuffer, uint16_t framelength);
void set_alarm_snd(const uint8_t *framebuffer, uint16_t framelength);
void recalcularParametros(void);
void sendDebugString(String mensaje);


//enum class FrameType{DATA, DATACYCLE, DATA_OK, SENDPARAM, PARAM_OK, Count};
enum class FrameType{DATA, DATA_OK, DATACYCLE, DATACYCLE_OK, SENDPARAM, PARAM_OK, ASK_STATE, ANS_STATE,
    SEND_POPER, POPER_OK, SEND_PCONFIG, PCONFIG_OK, SEND_STATE, STATE_OK, SENDDEBUG, SENDCONFIG, CONFIG_OK,
    CAL_DATA, CAL_PARAM, CAL_PARAM_OK, SEND_HW, HW_OK, SEND_CAL_POINTS, CAL_POINTS_OK, SET_ALARM,
    SEND_PALARM, PALARM_OK, Count};

struct sendParamStruct {
    uint8_t comando;
    uint8_t index;
    uint16_t valor;
    uint8_t alarma;
}__attribute__((packed));;

#endif //T2_COMUNICACIONES_H
