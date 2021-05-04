//
// Created by diego on 15/2/21.
//
#include <Arduino.h>
#include <PID_v1.h>
#include <parametros.h>
#include <comunicaciones.h>
#include "valvEspiratoria.h"
#include "board.h"
#include <Servo.h>

Servo myservo;
uint16_t esp_estado;
uint16_t espmaxF;

extern uint16_t P_config[];
extern uint16_t P_hardware[];

#define Kp1 (float) (P_hardware[(int)PARAM_HARDW::ESPKP1]/100.0)
#define Ki1 (float) (P_hardware[(int)PARAM_HARDW::ESPKI1]/100.0)
#define Kd1 (float) (P_hardware[(int)PARAM_HARDW::ESPKD1]/100.0)
#define Kp2 (float) (P_hardware[(int)PARAM_HARDW::ESPKP2]/100.0)
#define Ki2 (float) (P_hardware[(int)PARAM_HARDW::ESPKI2]/100.0)
#define Kd2 (float) (P_hardware[(int)PARAM_HARDW::ESPKD2]/100.0)
#define Kp3 (float) (P_hardware[(int)PARAM_HARDW::ESPKP3]/100.0)
#define Ki3 (float) (P_hardware[(int)PARAM_HARDW::ESPKI3]/100.0)
#define Kd3 (float) (P_hardware[(int)PARAM_HARDW::ESPKD3]/100.0)
#define espmin P_hardware[(int)PARAM_HARDW::ESPMIN]
#define espmaxN P_hardware[(int)PARAM_HARDW::ESPMAXN]
#define espmaxF0 P_hardware[(int)PARAM_HARDW::ESPMAXF0]
#define espmaxF1 P_hardware[(int)PARAM_HARDW::ESPMAXF1]

double valvIn, valvOut, valvSet;
PID* espPID;

void pid_control_expiration(uint16_t newvalue, uint16_t setvalue) {
    valvIn = newvalue;
    valvSet = setvalue;
    espPID->Compute();
    ValvEspSetPos((uint16_t) valvOut);
    String mens = "VALESP = " + String(valvOut);
//    sendDebugString(mens);
}

void pid_set_one_expiration(){
    String mens = "PID-1";
    sendDebugString(mens);
    espPID->SetTunings(Kp1, Ki1, Kd1);
}

void pid_set_two_expiration(){
    String mens = "PID-2";
    sendDebugString(mens);
    espPID->SetTunings(Kp2, Ki2, Kd2);
}

/*void pid_set_three_expiration(){
    String mens = "PID-3";
    sendDebugString(mens);
    espPID->SetTunings(Kp3, Ki3, Kd3);
}*/
//double e, D_contribucion, I_contribucion, i_1, a, e_1, T;

/*void pid_control_peep(uint16_t newvalue, uint16_t setvalue){
    e= newvalue - setvalue;
    D_contribucion= (Kd/T)*(e-e_1);
    I_contribucion= i_1+(Ki*T)*e;
    a= Kp*e+I_contribucion+D_contribucion;
    if (a>180) a= 180;
    if (a<0) a=0;
    e_1= e;
    i_1= I_contribucion;

    ValvEspSetPos((uint16_t) a);
//    String mens = "e = "+ String(e) + " kp*e = " + String(Kp*e) + "  a = " + String(a)+" Ki = " +String(Ki);
//    sendDebugString(mens);
}*/

void ValvEsp_init(void){
    myservo.attach(ANESP);
    espmaxF = espmaxF0;
    espPID = new PID(&valvIn, &valvOut, &valvSet, Kp1, Ki1, Kd1, REVERSE);
    String mens = "KPID: " + String(Kp1) + ": " + String(Ki1) + ": " + String(Kd1);
    sendDebugString(mens);
    espPID->SetMode(AUTOMATIC);
    espPID->SetSampleTime((int) (P_config[(int) PARAM_CONFIG::Pm])*10);
    espPID->SetOutputLimits(0, espmaxN);
//    e = D_contribucion = I_contribucion = i_1 = a = e_1 = 0;
//    T = P_config[(int) PARAM_CONFIG::Pm];
}

void ValvEspSetPos(uint16_t pos){
    esp_estado = pos;
    myservo.write((int) esp_estado);
//    String mens = "ValvEspSet a "+ String(esp_estado);
//    sendDebugString(mens);
}

void ValvEspSetClosed(void){
    ValvEspSetPos(espmin);
}

void ValvEspSetOpenedN(void){
    String mens = "NNValvEspOpen a "+ String(espmaxN);
    sendDebugString(mens);
    ValvEspSetPos(espmaxN);
}

void ValvEspSetValueN(void){
    espPID->SetOutputLimits(0, espmaxN);
}


void ValvEspSetValueF(uint16_t valor){
    espmaxF = valor;
    espPID->SetOutputLimits(0, valor);
}

void ValvEspSetOpenedF(void){
    String mens = "FFValvEspOpen a "+ String(espmaxF);
    sendDebugString(mens);
    ValvEspSetPos(espmaxF);

}

void ValvEspDecOpenF(){
    espmaxF -= 5;
    if (espmaxF < espmaxF1) espmaxF = espmaxF1;
}

void ValvEspIncOpenF(){
    espmaxF += 5;
    if (espmaxF > espmaxF0 ) espmaxF = espmaxF0;
}
void ValvEspResetPID(){
    espPID->ResetOutputSum();
}