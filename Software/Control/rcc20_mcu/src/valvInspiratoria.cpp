//
// Created by diego on 12/2/21.
//
#include <Arduino.h>
#include <PID_v1.h>
#include "comunicaciones.h" //borrar en cuanto quitemos mensajes de depuración
#include "valvInspiratoria.h"
#include "board.h"
#include "parametros.h"

//uint16_t insmax;
//uint16_t insmin;
uint16_t insp_estado;
uint16_t posTrigger; // posición que se tendrá que poner en función de si es trigger por presión -> 0, o dependiendo de la
// sensibilidad en trigger por flujo un valor u otro (quizá elegible por el control
extern uint16_t P_config[];
extern uint16_t P_hardware[];
extern estadoRespiracionStruct miEstadoRespiracion;

#define insmin P_hardware[(int) PARAM_HARDW::INSMIN]
#define insmax P_hardware[(int) PARAM_HARDW::INSMAX]
#define pressKpF (float) (P_hardware[(int) PARAM_HARDW::PRESSKPF]/100.0)
#define pressKiF (float) (P_hardware[(int) PARAM_HARDW::PRESSKIF]/100.0)
#define pressKdF (float) (P_hardware[(int) PARAM_HARDW::PRESSKDF]/100.0)
#define pressKpS (float) (P_hardware[(int) PARAM_HARDW::PRESSKPS]/100.0)
#define pressKiS (float) (P_hardware[(int) PARAM_HARDW::PRESSKIS]/100.0)
#define pressKdS (float) (P_hardware[(int) PARAM_HARDW::PRESSKDS]/100.0)
#define volKpF (float) (P_hardware[(int) PARAM_HARDW::VOLKPF]/100.0)
#define volKiF (float) (P_hardware[(int) PARAM_HARDW::VOLKIF]/100.0)
#define volKdF (float) (P_hardware[(int) PARAM_HARDW::VOLKDF]/100.0)
#define volKpS (float) (P_hardware[(int) PARAM_HARDW::VOLKPS]/100.0)
#define volKiS (float) (P_hardware[(int) PARAM_HARDW::VOLKIS]/100.0)
#define volKdS (float) (P_hardware[(int) PARAM_HARDW::VOLKDS]/100.0)

//***********************************************************************
double pressIn, pressOut, pressSet;
//double pressKpF, pressKiF, pressKdF, pressKpS, pressKiS, pressKdS;
double volIn, volOut, volSet;
//double volKpF, volKiF, volKdF, volKpS, volKiS, volKdS;

PID* pressPID;
PID* flowPID;
//***********************************************************************

void pid_control_press(uint16_t newvalue, uint16_t setvalue){
    pressIn = newvalue;
    pressSet = setvalue;
    pressPID->Compute();
    ValvInspSetPos((uint16_t) pressOut);
//    String mens = "New = "+String(newvalue) + " set = " + String(setvalue);
//    sendDebugString(mens);
}

void pid_control_flow(uint16_t newvalue, uint16_t setvalue){
    volIn = newvalue;
    volSet = setvalue;
    flowPID->Compute();
//    String mens = "Out pid "+String(volOut);
//    sendDebugString(mens);
    ValvInspSetPos((uint16_t) volOut);
}

void pid_set_fast_press(){
    String mens="Fast-PID";
    sendDebugString(mens);
    pressPID->SetTunings(pressKpF, pressKiF, pressKdF);
}

void pid_set_slow_press(){
    String mens="Slow-PID";
    sendDebugString(mens);
    pressPID->SetTunings(pressKpS, pressKiS, pressKdS);
}

void pid_set_fast_flow(){
    flowPID->SetTunings(volKpF, volKiF, volKdF);
}

void pid_set_slow_flow(){
    flowPID->SetTunings(volKpS, volKiS, volKdS);
}

void ValvInsp_init() {
    pinMode(ANINSP, OUTPUT);
    posTrigger = 0;
    pressPID = new PID(&pressIn, &pressOut, &pressSet, pressKpF, pressKiF, pressKdF, DIRECT);
    pressPID->SetMode(AUTOMATIC);
    pressPID->SetSampleTime((int) (P_config[(int) PARAM_CONFIG::Pm]*10));
    pressPID->SetOutputLimits(insmin, insmax);
    flowPID = new PID(&volIn, &volOut, &volSet, volKpF, volKiF, volKdF, DIRECT);
    flowPID->SetMode(AUTOMATIC);
    flowPID->SetSampleTime((int) (P_config[(int) PARAM_CONFIG::Pm]*10));
    flowPID->SetOutputLimits(insmin, insmax);
//    String mens = "KpF = " + String(pressKpF) + "\nKiF = " + String(pressKiF) + "\nKdF = " + String(pressKdF) +
//            "\nKpS = "+String(pressKpS) + "\nKiS = "+String(pressKiS)+"\nKdS = "+String(pressKdS)+
//            "\nvKpF"+String(volKpF)+"\nvKiF = " +String(volKiF)+"\nvKdF"+String(volKdF)+
//            "\nvKpS"+String(volKpS)+"\nvKiS = " +String(volKiS)+"\nvKdS"+String(volKdS);
//    sendDebugString(mens);
}

void ValvInspSetPos(uint16_t pos) {
    insp_estado = pos;
//    String mens = "Insp = "+String(insp_estado);
//    sendDebugString(mens);
    analogWrite(ANINSP, insp_estado);
}

void ValvInspSetClosed() {
    ValvInspSetPos(0);
}

//void ValvInspSetTriggerFlowValue(uint16_t value){
//    posTrigger = value;
//    ValvInspSetTriggerFlow();
//}
//
//void ValvInspSetTriggerFlow() {
////    if (miEstadoRespiracion.tiempo & 0x3F) {
////        String mens = "Abriendo BaseFlow: pos = " + String(posTrigger);
////        sendDebugString(mens);
////    }
//    ValvInspSetPos(posTrigger);
//}

void ValvInspResetPID(){
    pressPID->ResetOutputSum();
    flowPID->ResetOutputSum();
}