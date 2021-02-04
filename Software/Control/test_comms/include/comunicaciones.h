#include <stdint.h>

#ifndef T2_COMUNICACIONES_H
#define T2_COMUNICACIONES_H

void sendData(uint16_t tiempo, uint8_t * datos);
void send_character(uint8_t data);
void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength);

enum class FrameType{DATA, DATACYCLE, SENDPARAM, CHECKPARAM, Count};
enum class State{INSPIRATION, EXPIRATION, TRIGGER_WINDOW};

#endif //T2_COMUNICACIONES_H
