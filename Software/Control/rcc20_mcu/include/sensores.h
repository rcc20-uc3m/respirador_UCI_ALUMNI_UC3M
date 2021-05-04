//
// Created by diego on 10/2/21.
//

#ifndef T2_SENSORES_H
#define T2_SENSORES_H


#define MAX_CALIBRATION_POINTS 32

struct CALIBRACION {
    uint8_t n_puntos;
    uint16_t XFI[MAX_CALIBRATION_POINTS];
    uint16_t XFE[MAX_CALIBRATION_POINTS];
    uint16_t YFX[MAX_CALIBRATION_POINTS];
    uint16_t zeroPress;
    uint16_t zeroFI;
    uint16_t zeroFE;
    uint16_t coeffPress;
};

void init_sensores(void);
float getValorPresion();
float getValorFlujoEsp();
float getValorFlujoIns();
void actualizarValoresControl();
void actualizarValoresCalibracion();
#endif //T2_SENSORES_H
