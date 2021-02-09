#include <stdint.h>

#ifndef T2_COMUNICACIONES_H
#define T2_COMUNICACIONES_H

void sendData(uint8_t * datos);
void send_character(uint8_t data);
void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength);
void receive_command(const uint8_t *framebuffer, uint16_t framelength);
void answer_state(const uint8_t *framebuffer, uint16_t framelength);

//enum class FrameType{DATA, DATACYCLE, DATA_OK, SENDPARAM, PARAM_OK, Count};
enum class FrameType{DATA, DATA_OK, DATACYCLE, DATACYCLE_OK, SENDPARAM, PARAM_OK, ASK_STATE, ANS_STATE,
    SEND_POPER, POPER_OK, SEND_PCONFIG, PCONFIG_OK, SEND_STATE, STATE_OK, Count};

struct sendParamStruct {
    uint8_t comando;
    uint8_t index;
    uint16_t valor;
}__attribute__((packed));;

#endif //T2_COMUNICACIONES_H
