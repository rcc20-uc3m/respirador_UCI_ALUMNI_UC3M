#include <Arduino.h>
#include <Arduhdlc.h>
#include <comunicaciones.h>

/* How long (bytes) is the longest HDLC frame? */
#define MAX_HDLC_FRAME_LENGTH 32

#define SENDDATASIZE 6

Arduhdlc hdlc(&send_character, &hdlc_command_router, MAX_HDLC_FRAME_LENGTH);


void sendData(uint16_t tiempo, uint8_t *datos){
    hdlc.sendFrame(datos, SENDDATASIZE);
}



/* Function to send out one 8bit character */
void send_character(uint8_t data) {
    Serial.print((char)data);
}

void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength) {
    return;
}

void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        hdlc.charReceiver(inChar);
    }
}