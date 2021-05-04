//
// Created by diego on 8/2/21.
//

#ifndef T2_UTILIDADES_H
#define T2_UTILIDADES_H

#include <stdint.h>

uint8_t get_lsb(uint16_t valor);
uint8_t get_msb(uint16_t valor);
uint16_t getuint16fromint8(uint8_t mi_lsb, uint8_t mi_msb);

#endif //T2_UTILIDADES_H
