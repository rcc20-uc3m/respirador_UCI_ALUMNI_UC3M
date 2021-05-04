#ifndef UTILIDADES_H
#define UTILIDADES_H
#include <stdint.h>
#include <QtCore>

uint8_t get_lsb(uint16_t valor);
uint8_t get_msb(uint16_t valor);
uint16_t getuint16fromint8(uint8_t mi_lsb, uint8_t mi_msb);
QByteArray get_param_frame(uint8_t command, uint16_t valor);
QByteArray get_config_frame(uint8_t command, uint16_t value);

#endif // UTILIDADES_H
