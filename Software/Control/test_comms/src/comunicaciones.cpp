#include <Arduino.h>
#include <Arduhdlc.h>
#include <comunicaciones.h>
#include <parametros.h>
#include <respirador.h>
#include <utilidades.h>

extern estadoMaquinaStruct miEstadoMaquina;

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];

/* How long (bytes) is the longest HDLC frame? */
#define MAX_HDLC_FRAME_LENGTH 32

uint8_t tx_buffer[MAX_HDLC_FRAME_LENGTH];

#define SENDDATASIZE 7
#define SENDPARAMSIZE 4
#define SENDANSSTATE 2

sendParamStruct misParametros;

Arduhdlc hdlc(&send_character, &hdlc_command_router, MAX_HDLC_FRAME_LENGTH);

uint16_t getuint16frombytes(uint8_t mi_lsb, uint8_t mi_msb)
{
    return (mi_lsb | (mi_msb << 8 )) & 0xFFFF;
}

void sendData(uint8_t *datos){
    hdlc.sendFrame(datos, SENDDATASIZE);
}



/* Function to send out one 8bit character */
void send_character(uint8_t data) {
    Serial.print((char)data);
}


void setParamOperacion(uint8_t index, uint16_t val)
{
    //Parámetros Calculados	UDS	Formula	Comentario
    //Pr	ms	60000/Fr	Periodo respiración
    //Ti	ms	Pr*1/(1+IE)	Tiempo total de inspiración
    //Te	ms	Pr-Ti	Tiempo total de espiración
    //Tie	ms	Ti*(1-%Tp)	Tiempo inspiración efectivo
    //Tea	ms	Ti+Te*(1-TRIG_WND)	Tiempo absoluto espiración
    //PeakFlow	ml/s	Vt/Tie	Flujo para Control por Volumen
    P_operacion[index] = val;
    if (index == (int) PARAM_OPERACION::Fr){
        P_calculados[(int)PARAM_CALCULADOS::Pr] = 60000/val;
        P_calculados[(int)PARAM_CALCULADOS::Ti] = P_calculados[(int)PARAM_CALCULADOS::Pr]*10.0/(10.0 + P_operacion[(int)PARAM_OPERACION::iex10]);
        P_calculados[(int)PARAM_CALCULADOS::Te] = P_calculados[(int)PARAM_CALCULADOS::Pr] - P_calculados[(int)PARAM_CALCULADOS::Ti];
        P_calculados[(int)PARAM_CALCULADOS::Tie] = P_calculados[(int)PARAM_CALCULADOS::Ti]*(1.0-P_config[(int)PARAM_CONFIG::porTp]);
        P_calculados[(int)PARAM_CALCULADOS::Tea] = P_calculados[(int)PARAM_CALCULADOS::Ti]+
                P_calculados[(int)PARAM_CALCULADOS::Te]*(1.0-P_config[(int)PARAM_CONFIG::Trig_Wnd]);
        P_calculados[(int)PARAM_CALCULADOS::PeakFlow] = P_operacion[(int)PARAM_OPERACION::Vt] / (float)P_calculados[(int)PARAM_CALCULADOS::Tie];
    } else if (index == (int)PARAM_OPERACION::iex10) {
        P_calculados[(int)PARAM_CALCULADOS::Ti] = P_calculados[(int)PARAM_CALCULADOS::Pr]*10.0/(10.0 + P_operacion[(int)PARAM_OPERACION::iex10]);
        P_calculados[(int)PARAM_CALCULADOS::Te] = P_calculados[(int)PARAM_CALCULADOS::Pr] - P_calculados[(int)PARAM_CALCULADOS::Ti];
        P_calculados[(int)PARAM_CALCULADOS::Tie] = P_calculados[(int)PARAM_CALCULADOS::Ti]*(1.0-P_config[(int)PARAM_CONFIG::porTp]);
        P_calculados[(int)PARAM_CALCULADOS::Tea] = P_calculados[(int)PARAM_CALCULADOS::Ti]+
                                                   P_calculados[(int)PARAM_CALCULADOS::Te]*(1.0-P_config[(int)PARAM_CONFIG::Trig_Wnd]);
        P_calculados[(int)PARAM_CALCULADOS::PeakFlow] = P_operacion[(int)PARAM_OPERACION::Vt] / (float)P_calculados[(int)PARAM_CALCULADOS::Tie];
    } else if (index == (int)PARAM_OPERACION::Vt) {
        P_calculados[(int)PARAM_CALCULADOS::PeakFlow] = P_operacion[(int)PARAM_OPERACION::Vt] / (float)P_calculados[(int)PARAM_CALCULADOS::Tie];
    }
}

void receive_command(const uint8_t *framebuffer, uint16_t framelength){
    uint8_t index = framebuffer[1];
    uint16_t val = getuint16frombytes(framebuffer[2], framebuffer[3]);
    switch(index){
        case (int) PARAM_OPERACION::Fr: setParamOperacion(index, val); break;
    }
    tx_buffer[0] = (uint8_t) FrameType::PARAM_OK;
    tx_buffer[1] = (uint8_t) index;
    tx_buffer[2] = (uint8_t) get_lsb(val);
    tx_buffer[3] = (uint8_t) get_msb(val);
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDPARAMSIZE);
}

void answer_state(const uint8_t *framebuffer, uint16_t framelength){
    tx_buffer[0] = (uint8_t) FrameType::ANS_STATE;
    tx_buffer[1] = (uint8_t) miEstadoMaquina.estado;
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDANSSTATE);
}

void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength) {

    enum FrameType command = static_cast<FrameType>(framebuffer[0]);
    switch(command) {
        case FrameType::SENDPARAM: receive_command(framebuffer, framelength); break;
        case FrameType::ASK_STATE: answer_state(framebuffer, framelength); break;
    }
    return;
}

void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        hdlc.charReceiver(inChar);
    }
}