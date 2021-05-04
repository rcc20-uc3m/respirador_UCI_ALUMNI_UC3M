#ifndef CALIBRACION_H
#define CALIBRACION_H

#include <qglobal.h>

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

#endif // CALIBRACION_H
