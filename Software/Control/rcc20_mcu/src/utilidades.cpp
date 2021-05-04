//
// Created by diego on 8/2/21.
//

#include "utilidades.h"

uint8_t get_lsb(uint16_t valor){
    return (uint8_t) valor & 0xFFu;
}

uint8_t get_msb(uint16_t valor){
    return (uint8_t) (valor >> 8) & 0xFFu;
}

uint16_t getuint16fromint8(uint8_t mi_lsb, uint8_t mi_msb)
{
    return ( mi_lsb | (mi_msb << 8 )) & 0xFFFF;
}