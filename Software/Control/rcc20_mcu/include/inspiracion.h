//
// Created by diego on 12/2/21.
//

#ifndef T2_INSPIRACION_H
#define T2_INSPIRACION_H

#define VOL_MIN_LOG 3

void init_inspiracion(void);
void reset_espiracion();
void inspiracion_ciclo(void);
void inspiracion_control(void);
void reset_inspiracion();
void ajusteMinCuadradosCyR();
float calc_resistencia();
float calc_complianza();

#endif //T2_INSPIRACION_H
