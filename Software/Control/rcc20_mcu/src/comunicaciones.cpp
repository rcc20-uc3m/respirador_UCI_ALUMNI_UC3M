#include <Arduino.h>
#include <Arduhdlc.h>
#include <comunicaciones.h>
#include <parametros.h>
#include <respirador.h>
#include <utilidades.h>
#include <disparo.h>
#include <valvInspiratoria.h>
#include <valvEspiratoria.h>
#include "sensores.h"
#include "alarmas.h"

extern estadoMaquinaStruct miEstadoMaquina;
extern estadoRespiracionStruct miEstadoRespiracion;
extern void startTimer();

void set_alarm_snd(const uint8_t *framebuffer, uint16_t framelength);

extern uint16_t P_config[];
extern uint16_t P_ciclo[];
extern uint16_t P_calculados[];
extern uint16_t P_operacion[];
extern uint16_t P_control[];
extern uint16_t P_hardware[];
extern uint16_t P_alarmas[];
extern uint16_t N_CONST;
extern float coefVol;

extern uint16_t pressCalibracion;
extern uint16_t FICalibracion;
extern uint16_t FECalibracion;

extern float cn1, cp1;

extern CALIBRACION miCalibracion;

/* How long (bytes) is the longest HDLC frame? */
#define MAX_HDLC_FRAME_LENGTH 256

uint8_t tx_buffer[MAX_HDLC_FRAME_LENGTH];

#define SENDDATASIZE 8
#define SENDPARAMSIZE 4
#define SENDANSSTATE 4
#define SEND_OK 1
#define SENDCALPARAMOK 5
#define SENDDATACYCLESIZE 21

sendParamStruct misParametros;

Arduhdlc hdlc(&send_character, &hdlc_command_router, MAX_HDLC_FRAME_LENGTH);

uint16_t getuint16frombytes(uint8_t mi_lsb, uint8_t mi_msb)
{
    return (mi_lsb | (mi_msb << 8 )) & 0xFFFF;
}

void sendCycleData(sendDataCycleStruct misDatos){
    tx_buffer[0] = (int) FrameType::DATACYCLE;
    tx_buffer[1] = get_lsb(misDatos.Ppico);
    tx_buffer[2] = get_msb(misDatos.Ppico);
    tx_buffer[3] = get_lsb(misDatos.Pplateau);
    tx_buffer[4] = get_msb(misDatos.Pplateau);
    tx_buffer[5] = get_lsb(misDatos.Vinsp);
    tx_buffer[6] = get_msb(misDatos.Vinsp);
    tx_buffer[7] = get_lsb(misDatos.peepM);
    tx_buffer[8] = get_msb(misDatos.peepM);
    tx_buffer[9] = get_lsb(misDatos.Comp);
    tx_buffer[10] = get_msb(misDatos.Comp);
    tx_buffer[11] = get_lsb(misDatos.Res);
    tx_buffer[12] = get_msb(misDatos.Res);
    tx_buffer[13] = get_lsb(misDatos.FImax);
    tx_buffer[14] = get_msb(misDatos.FImax);
    tx_buffer[15] = get_lsb(misDatos.FEmax);
    tx_buffer[16] = get_msb(misDatos.FEmax);
    tx_buffer[17] = get_lsb(misDatos.MinVol);
    tx_buffer[18] = get_msb(misDatos.MinVol);
    tx_buffer[19] = get_lsb(misDatos.Alarma);
    tx_buffer[20] = get_msb(misDatos.Alarma);
    hdlc.sendFrame(tx_buffer, SENDDATACYCLESIZE);
}

void sendData(sendDataStruct datos){
    tx_buffer[0] = (int) FrameType::DATA;
    tx_buffer[1] = get_lsb(datos.pressure);
    tx_buffer[2] = get_msb(datos.pressure);
    tx_buffer[3] = get_lsb(datos.Insp_Flow);
    tx_buffer[4] = get_msb(datos.Insp_Flow);
    tx_buffer[5] = get_lsb(datos.Esp_Flow);
    tx_buffer[6] = get_msb(datos.Esp_Flow);
    tx_buffer[7] = get_lsb(datos.FiO2);
    tx_buffer[8] = get_msb(datos.FiO2);
//    tx_buffer[9] = get_lsb(datos.alarma);
//    tx_buffer[10] = get_msb(datos.alarma);
    hdlc.sendFrame(tx_buffer, 9);
}

void sendCalibrationData(void){
    tx_buffer[0] = (int) FrameType::CAL_DATA;
    tx_buffer[1] = get_lsb(pressCalibracion);
    tx_buffer[2] = get_msb(pressCalibracion);
    tx_buffer[3] = get_lsb(FICalibracion);
    tx_buffer[4] = get_msb(FICalibracion);
    tx_buffer[5] = get_lsb(FECalibracion);
    tx_buffer[6] = get_msb(FECalibracion);
    tx_buffer[7] = get_lsb(P_control[(int)PARAM_CONTROL::PressMs]);
    tx_buffer[8] = get_msb(P_control[(int)PARAM_CONTROL::PressMs]);
    tx_buffer[9] = get_lsb(P_control[(int)PARAM_CONTROL::FIMs]);
    tx_buffer[10] = get_msb(P_control[(int)PARAM_CONTROL::FIMs]);
    tx_buffer[11] = get_lsb(P_control[(int)PARAM_CONTROL::FEMs]);
    tx_buffer[12] = get_msb(P_control[(int)PARAM_CONTROL::FEMs]);
    hdlc.sendFrame(tx_buffer, 13);
}

void sendDebugString(String mensaje){
    tx_buffer[0] = (int) FrameType::SENDDEBUG;
    for (uint i = 0; i < mensaje.length(); i++)
        tx_buffer[i+1] = (uint8_t) mensaje.charAt(i);
    hdlc.sendFrame(tx_buffer, (uint16_t) (mensaje.length() +1));
}

/* Function to send out one 8bit character */
void send_character(uint8_t data) {
    Serial.print((char)data);
}

void recalcularParametros(void){
    P_calculados[(int)PARAM_CALCULADOS::Pr] = 60000.0/(float) P_operacion[(int) PARAM_OPERACION::Fr];
    P_calculados[(int)PARAM_CALCULADOS::Ti] = P_calculados[(int)PARAM_CALCULADOS::Pr]*10.0/(10.0 + P_operacion[(int)PARAM_OPERACION::iex10]);
    P_calculados[(int)PARAM_CALCULADOS::Te] = P_calculados[(int)PARAM_CALCULADOS::Pr] - P_calculados[(int)PARAM_CALCULADOS::Ti];
    P_calculados[(int)PARAM_CALCULADOS::Tie] = P_calculados[(int)PARAM_CALCULADOS::Ti] -
            (float) (P_calculados[(int)PARAM_CALCULADOS::Pr]*P_config[(int)PARAM_CONFIG::porTp])/100.0 - P_config[(int)PARAM_CONFIG::Tri];
    P_calculados[(int)PARAM_CALCULADOS::Tea] = P_calculados[(int)PARAM_CALCULADOS::Ti]+
                                               P_calculados[(int)PARAM_CALCULADOS::Te]*(100.0-P_config[(int)PARAM_CONFIG::Trig_Wnd])/100.0;
    P_calculados[(int)PARAM_CALCULADOS::Tedm] = P_calculados[(int)PARAM_CALCULADOS::Ti]+
                                               P_calculados[(int)PARAM_CALCULADOS::Te]*(100.0-P_config[(int)PARAM_CONFIG::Trig_Mnd])/100.0;
    P_calculados[(int)PARAM_CALCULADOS::PeakFlow] = (600.0*P_operacion[(int)PARAM_OPERACION::Vt]) / (float)P_calculados[(int)PARAM_CALCULADOS::Tie];
    coefVol = P_config[(int)PARAM_CONFIG::Pm]/600.0/2.0;
    String mensaje;
    mensaje = "Calculados \n";
    for(int i = 0; i < (int) PARAM_CALCULADOS::Count; i++){
        mensaje = mensaje + " " + String(P_calculados[i]);
    }
    sendDebugString(mensaje);
}

void receive_config(const uint8_t *framebuffer, uint16_t framelength){
    uint8_t index = framebuffer[1];
    uint16_t val = getuint16frombytes(framebuffer[2], framebuffer[3]);
    P_config[index] = val;
    if ((index == (int) PARAM_CONFIG::porTp) ||
        (index == (int) PARAM_CONFIG::Trig_Wnd) ||
        (index == (int) PARAM_CONFIG::Trig_Mnd))
        recalcularParametros();
    tx_buffer[0] = (uint8_t) FrameType::CONFIG_OK;
    tx_buffer[1] = (uint8_t) index;
    tx_buffer[2] = (uint8_t) get_lsb(P_config[index]);
    tx_buffer[3] = (uint8_t) get_msb(P_config[index]);
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDPARAMSIZE);
}

void receive_calibration_param(const uint8_t* framebuffer, uint16_t framelength){
    uint16_t posInsp, posEsp;
    posInsp = getuint16frombytes(framebuffer[1], framebuffer[2]);
    posEsp =  getuint16frombytes(framebuffer[3], framebuffer[4]);
    ValvInspSetPos(posInsp);
    ValvEspSetPos(posEsp);
    tx_buffer[0] = (uint8_t) FrameType::CAL_PARAM_OK;
    tx_buffer[1] = (uint8_t) get_lsb(posInsp);
    tx_buffer[2] = (uint8_t) get_msb(posInsp);
    tx_buffer[3] = (uint8_t) get_lsb(posEsp);
    tx_buffer[4] = (uint8_t) get_msb(posEsp);
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDCALPARAMOK);
}

void receive_param(const uint8_t *framebuffer, uint16_t framelength){
    uint8_t index = framebuffer[1];
    uint16_t val = getuint16frombytes(framebuffer[2], framebuffer[3]);
    P_operacion[index] = val;
    switch(index){
        case (int) PARAM_OPERACION::Fr: recalcularParametros(); break;
        case (int) PARAM_OPERACION::iex10: recalcularParametros(); break;
        case (int) PARAM_OPERACION::Vt: recalcularParametros(); break;
    }
    tx_buffer[0] = (uint8_t) FrameType::PARAM_OK;
    tx_buffer[1] = (uint8_t) index;
    tx_buffer[2] = (uint8_t) get_lsb(P_operacion[index]);
    tx_buffer[3] = (uint8_t) get_msb(P_operacion[index]);
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDPARAMSIZE);
}

void receive_alarms_parameters(const uint8_t* framebuffer, uint16_t framelength){
    int i = 0;
    uint16_t tmp;
    while(i < (int) PARAM_ALARM::Count){
        tmp = getuint16frombytes(framebuffer[2*i+1], framebuffer[2*i+2]);
        P_alarmas[i] = tmp;
        i++;
    }
    String mensaje;
    mensaje = "Alarmas \n";
    for(int j = 0; j < (int) PARAM_ALARM::Count; j++){
        mensaje = mensaje + " " + String(P_alarmas[j]);
    }
    sendDebugString(mensaje);
    delay(200);

    tx_buffer[0] = (uint8_t) FrameType::PALARM_OK;
    hdlc.sendFrame((uint8_t *) tx_buffer, SEND_OK);
}
void receive_param_hardware(const uint8_t *framebuffer, uint16_t framelength){
    int i = 0;
    uint16_t tmp;
    while(i < (int) PARAM_HARDW::Count){
        tmp = getuint16frombytes(framebuffer[2*i+1], framebuffer[2*i+2]);
        P_hardware[i] = tmp;
        i++;
    }

    String mensaje;
    mensaje = "Hardware \n";
    for(int j = 0; j < (int) PARAM_HARDW::Count; j++){
        mensaje = mensaje + " " + String(P_hardware[j]);
    }
    sendDebugString(mensaje);
    delay(200);

    tx_buffer[0] = (uint8_t) FrameType::HW_OK;
    hdlc.sendFrame((uint8_t *) tx_buffer, SEND_OK);
}

void receive_calibration_points(const uint8_t* framebuffer, uint16_t framelength) {
    uint ntotal;
    miCalibracion.n_puntos = framebuffer[1];
    ntotal = 10+6*miCalibracion.n_puntos;
    if (framelength != ntotal){
        String mens = "No coincide ntotal " + String(ntotal) + "  con framelength " + framelength;
        sendDebugString(mens);
    }
    for (int i = 0; i < miCalibracion.n_puntos; i++){
        miCalibracion.XFI[i] = getuint16frombytes(framebuffer[2+6*i], framebuffer[3+6*i]);
        miCalibracion.XFE[i] = getuint16frombytes(framebuffer[4+6*i], framebuffer[5+6*i]);
        miCalibracion.YFX[i] = getuint16frombytes(framebuffer[6+6*i], framebuffer[7+6*i]);
    }
    miCalibracion.zeroPress = getuint16frombytes(framebuffer[ntotal-8], framebuffer[ntotal-7]);
    miCalibracion.zeroFI = getuint16frombytes(framebuffer[ntotal-6], framebuffer[ntotal-5]);
    miCalibracion.zeroFE = getuint16frombytes(framebuffer[ntotal-4], framebuffer[ntotal-3]);
    miCalibracion.coeffPress = getuint16frombytes(framebuffer[ntotal-2], framebuffer[ntotal-1]);

    String mensaje;
    mensaje = "Calibracion n =  " + String(miCalibracion.n_puntos) + "\n";
    for(int j = 0; j < (int) miCalibracion.n_puntos; j++){
        mensaje = mensaje + " " + String(miCalibracion.XFI[j]) + " " + String(miCalibracion.XFE[j]) + " "
                + String(miCalibracion.YFX[j]) + "\n";
    }
    mensaje = mensaje+"Zeros: "+String(miCalibracion.zeroPress)+" "+String(miCalibracion.zeroFI)+" "+
            String(miCalibracion.zeroFE)+" :: "+String(miCalibracion.coeffPress);
    sendDebugString(mensaje);
    delay(200);

    tx_buffer[0] = (uint8_t) FrameType::CAL_POINTS_OK;
    hdlc.sendFrame((uint8_t *) tx_buffer, SEND_OK);
}

void answer_state(const uint8_t *framebuffer, uint16_t framelength){
    tx_buffer[0] = (uint8_t) FrameType::ANS_STATE;
    tx_buffer[1] = (uint8_t) miEstadoMaquina.estado;
    tx_buffer[2] = (uint8_t) miEstadoMaquina.modo;
    tx_buffer[3] = (uint8_t) miEstadoMaquina.disparo;
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDANSSTATE);
}

void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength) {
    enum FrameType command = static_cast<FrameType>(framebuffer[0]);
    switch(command) {
        case FrameType::SENDPARAM: receive_param(framebuffer, framelength); break;
        case FrameType::ASK_STATE: answer_state(framebuffer, framelength); break;
        case FrameType::SEND_PCONFIG: get_configuration_parameters(framebuffer, framelength); break;
        case FrameType::SEND_POPER: get_operation_parameters(framebuffer, framelength); break;
        case FrameType::SEND_STATE: set_machineState(framebuffer, framelength); break;
        case FrameType::SENDCONFIG: receive_config(framebuffer, framelength); break;
        case FrameType::CAL_PARAM: receive_calibration_param(framebuffer, framelength); break;
        case FrameType::SEND_HW: receive_param_hardware(framebuffer, framelength); break;
        case FrameType::SEND_CAL_POINTS: receive_calibration_points(framebuffer, framelength); break;
        case FrameType::SET_ALARM: set_alarm_snd(framebuffer, framelength); break;
        case FrameType::SEND_PALARM: receive_alarms_parameters(framebuffer, framelength); break;
        case FrameType::PALARM_OK:
        case FrameType::DATA:
        case FrameType::PARAM_OK:
        case FrameType::DATACYCLE:
        case FrameType::DATA_OK:
        case FrameType::DATACYCLE_OK:
        case FrameType::ANS_STATE:
        case FrameType::POPER_OK:
        case FrameType::PCONFIG_OK:
        case FrameType::STATE_OK:
        case FrameType::SENDDEBUG:
        case FrameType::CONFIG_OK:
        case FrameType::CAL_DATA:
        case FrameType::CAL_PARAM_OK:
        case FrameType::HW_OK:
        case FrameType::CAL_POINTS_OK:
        case FrameType::Count:
            break;
    }
}

void set_alarm_snd(const uint8_t *framebuffer, uint16_t framelength) {
    TIPOALARMA estadoSnd = static_cast<TIPOALARMA>(framebuffer[1]);
    switch (estadoSnd) {
        case TIPOALARMA::NULA: reset_snd_alarma(); break;
        case TIPOALARMA::BAJA: lanzar_snd_alarma_baja(); break;
        case TIPOALARMA::MEDIA: lanzar_snd_alarma_media(); break;
        case TIPOALARMA::ALTA: lanzar_snd_alarma_alta(); break;
    }
}

void set_machineState(const uint8_t *framebuffer, uint16_t framelength){
    MachineState nuevoEstado = static_cast<MachineState>(framebuffer[1]);
    MODOS nuevoModo = static_cast<MODOS>(framebuffer[2]);
    TRIGGER nuevoDisparo = static_cast<TRIGGER>(framebuffer[3]);
    if (nuevoEstado == MachineState::ON){
        init_respirador();
        miEstadoMaquina.primero = true;
        miEstadoMaquina.ultimo =  false;
        reset_machine();
    } else if (nuevoEstado == MachineState::OFF){
        miEstadoMaquina.primero = false;
        miEstadoMaquina.ultimo =  true;
        reset_machine();
    } else if (nuevoEstado == MachineState::RESET){
        init_respirador();
    } else if (nuevoEstado == MachineState::CAL){
        init_respirador();
    }
    miEstadoMaquina.estado = nuevoEstado;
    if ((nuevoModo != miEstadoMaquina.modo) || (nuevoDisparo != miEstadoMaquina.disparo)){
        miEstadoMaquina.modo = nuevoModo;
        miEstadoMaquina.disparo = nuevoDisparo;
    }
    tx_buffer[0] = (uint8_t) FrameType::STATE_OK;
    tx_buffer[1] = (uint8_t) miEstadoMaquina.estado;
    tx_buffer[2] = (uint8_t) miEstadoMaquina.modo;
    tx_buffer[3] = (uint8_t) miEstadoMaquina.disparo;
    hdlc.sendFrame((uint8_t *) tx_buffer, SENDANSSTATE);
}

void get_operation_parameters(const uint8_t *framebuffer, uint16_t framelength){
    int i = 0;
    String mensaje;
    while(i < (int)PARAM_OPERACION::Count){
        P_operacion[i] = getuint16frombytes(framebuffer[2*i+1], framebuffer[2*i+2]);
        i++;
    }
    tx_buffer[0] = (uint8_t) FrameType::POPER_OK;
    hdlc.sendFrame((uint8_t *) tx_buffer, SEND_OK);
    recalcularParametros();
}

void get_configuration_parameters(const uint8_t *framebuffer, uint16_t framelength){
    int i = 0;
    float n;
    bool changed[(int) PARAM_CONFIG::Count];
    changed[(int)PARAM_CONFIG::Pm] = false;
    changed[(int)PARAM_CONFIG::N] = false;
    uint16_t tmp;
    while(i < (int) PARAM_CONFIG::Count){
        tmp = getuint16frombytes(framebuffer[2*i+1], framebuffer[2*i+2]);
        if (P_config[i] != tmp) {
            P_config[i] = tmp;
            changed[i] = true;
        }
        i++;
    }

    String mensaje;
    mensaje = "Config \n";
    for(int i = 0; i < (int) PARAM_CONFIG::Count; i++){
        mensaje = mensaje + " " + String(P_config[i]);
    }
    sendDebugString(mensaje);

    tx_buffer[0] = (uint8_t) FrameType::PCONFIG_OK;
    hdlc.sendFrame((uint8_t *) tx_buffer, SEND_OK);
    recalcularParametros();

    if (changed[(int)PARAM_CONFIG::Pm]){
        startTimer();
    }
    if (changed[(int)PARAM_CONFIG::N]){
        n = P_config[(int)PARAM_CONFIG::N];
        N_CONST = P_config[(int)PARAM_CONFIG::N];
        cn1 = 1.0/(n);
        cp1 = 1.0 - cn1;
    }
}

void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        hdlc.charReceiver(inChar);
    }
}