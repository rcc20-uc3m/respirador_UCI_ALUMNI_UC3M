#include <utilidades.h>
#include <datos.h>

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

QByteArray get_param_frame(uint8_t command, uint16_t value){
    QByteArray data;
    data.append((uint8_t) FrameType::SENDPARAM);
    data.append((uint8_t) command);
    data.append((uint8_t) get_lsb(value));
    data.append((uint8_t) get_msb(value));
    return data;
}

QByteArray get_config_frame(uint8_t command, uint16_t value){
    QByteArray data;
    data.append((uint8_t) FrameType::SENDCONFIG);
    data.append((uint8_t) command);
    data.append((uint8_t) get_lsb(value));
    data.append((uint8_t) get_msb(value));
    return data;
}
