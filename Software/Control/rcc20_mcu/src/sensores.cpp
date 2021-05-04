//
// Created by diego on 10/2/21.
//
#include "Arduino.h"
#include "sensores.h"
#include "board.h"
#include "parametros.h"
#include "alarmas.h"

#include "comunicaciones.h"

extern uint16_t P_control[];
extern uint16_t P_ciclo[];

extern uint16_t pressCalibracion;
extern uint16_t FICalibracion;
extern uint16_t FECalibracion;


float cp1, cn1;


struct CALIBRACION miCalibracion;

//int n_puntos = 9;
//uint16_t xFI[] = {0, 1, 9, 76, 306, 458, 626, 698, 705};
//uint16_t xFE[] = {0, 2, 10, 81, 311, 446, 618, 696, 705};
//uint16_t yFX[] = {0, 1, 10, 60, 227, 375, 638, 880, 906};
//uint16_t zeroPress = 85;
//uint16_t zeroFI = 82;
//uint16_t zeroFE = 87;

uint16_t N_CONST = 0;

uint16_t interpLineal(int n, uint16_t pos, uint16_t* X, uint16_t* Y){
    for(int i = 0; i < miCalibracion.n_puntos-1; i++){
        if ((X[i] < pos) && (pos < X[i+1])){
            return Y[i] + (float)(pos - X[i])/(float)(X[i+1]-X[i])*(float)(Y[i+1]-Y[i]);
        }
    }
    return 0;
}

uint16_t getFI(uint16_t pos){
    int32_t tmp = interpLineal(miCalibracion.n_puntos, pos, miCalibracion.XFI, miCalibracion.YFX);
    if (tmp < 0) return 0;
    return tmp;
}

uint16_t getFE(uint16_t pos){
    int32_t tmp = interpLineal(miCalibracion.n_puntos, pos, miCalibracion.XFE, miCalibracion.YFX);
    if (tmp < 0) return 0;
    return tmp;
}


float getValorPresion() {
    uint32_t tmp = analogRead(SENSPR);
    int32_t res = (tmp - miCalibracion.zeroPress);
    if (res < 0) return 0;
    res *= (float) miCalibracion.coeffPress/100.0 ;
    return res;
}

float getValorFiO2(){
    uint32_t tmp = analogRead(SENSO2);
    return 0.0001*tmp*tmp - 0.0649*tmp + 29.475;
}

float getValorFlujoEsp() {
    uint32_t tmp = analogRead(SENSFE);
    return getFE(tmp - miCalibracion.zeroFE);
}

float getValorFlujoIns() {
    uint32_t tmp = analogRead(SENSFI);
    return getFI(tmp - miCalibracion.zeroFI);
}

void actualizarValoresControl(){
    float tmp1, tmp2, tmp3, tmp4;
    float tmp10, tmp11, tmp12, tmp13;
    tmp1 = getValorPresion();
    P_control[(int)PARAM_CONTROL::PressInst] = tmp1;
    tmp2 = getValorFlujoIns();
    tmp3 = getValorFlujoEsp();
    tmp10 = P_control[(int)PARAM_CONTROL::PressMs];
    tmp11 = P_control[(int)PARAM_CONTROL::FIMs];
    tmp12 = P_control[(int)PARAM_CONTROL::FEMs];
    tmp4 = getValorFiO2();
    tmp13 = P_control[(int)PARAM_CONTROL::FiO2];

    tmp10 = tmp10*cp1 + (float)tmp1*cn1;
    tmp11 = tmp11*cp1 + (float)tmp2*cn1;
    tmp12 = tmp12*cp1 + (float)tmp3*cn1;
    tmp13 = tmp13*cp1 + (float)tmp4*cn1;
    P_control[(int)PARAM_CONTROL::PressMs] = tmp10;
    P_control[(int)PARAM_CONTROL::FIMs] = tmp11;
    P_control[(int)PARAM_CONTROL::FEMs] = tmp12;
    P_control[(int)PARAM_CONTROL::FiO2] = tmp13;
    if (tmp1 >= VALOR_PMAX) set_alarm_pmax(&P_ciclo[(int)PARAM_CICLO::Alarma]);
    else reset_alarm_pmax(&P_ciclo[(int)PARAM_CICLO::Alarma]);

}

void actualizarValoresCalibracion(){
    uint16_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    tmp1 = getValorPresion();
    tmp4 = analogRead(SENSPR);
    tmp2 = getValorFlujoIns();
    tmp5 = analogRead(SENSFI);
    tmp3 = getValorFlujoEsp();
    tmp6 = analogRead(SENSFE);
    pressCalibracion = (float) (tmp4*cn1 + pressCalibracion*cp1);
    FICalibracion = (float) (tmp5*cn1 + FICalibracion*cp1);
    FECalibracion = (float) (tmp6*cn1 + FECalibracion*cp1);
    P_control[(int)PARAM_CONTROL::PressMs] = (float) (tmp1*cn1+P_control[(int)PARAM_CONTROL::PressMs]*cp1);
    P_control[(int)PARAM_CONTROL::FIMs] = (float) (tmp2*cn1+P_control[(int)PARAM_CONTROL::FIMs]*cp1);
    P_control[(int)PARAM_CONTROL::FEMs] = (float) (tmp3*cn1+P_control[(int)PARAM_CONTROL::FEMs]*cp1);
}

void init_sensores(){
    pinMode(BATMODE, INPUT);
    pinMode(UPS_READY, INPUT);
    pinMode(UPS_ALARM, INPUT);
    pinMode(PRESSALARM, INPUT);

    pinMode(SENSPR, INPUT);
    pinMode(SENSFI, INPUT);
    pinMode(SENSFE, INPUT);
//    zeroPress = zeroFE = zeroFI = 0;
    pressCalibracion = FICalibracion = FECalibracion = 0;
    P_control[(int)PARAM_CONTROL::PressMs] = 0;
    P_control[(int)PARAM_CONTROL::FIMs] = 0;
    P_control[(int)PARAM_CONTROL::FEMs] = 0;

    for(int i= 0; i < N_CONST; i++){
        miCalibracion.zeroPress += analogRead(SENSPR);
        miCalibracion.zeroFI += analogRead(SENSFI);
        miCalibracion.zeroFE += analogRead(SENSFE);
    }
    miCalibracion.zeroPress = (float) (miCalibracion.zeroPress / N_CONST);
    miCalibracion.zeroFI = (float) (miCalibracion.zeroFI / N_CONST);
    miCalibracion.zeroFE = (float) (miCalibracion.zeroFE / N_CONST);
    String mensaje = "Zeros= " + String(miCalibracion.zeroPress) + " " + String(miCalibracion.zeroFI) + " " +
            String(miCalibracion.zeroFE);
    sendDebugString(mensaje);
}