//
// Created by diego on 15/2/21.
//

#ifndef T2_VALVESPIRATORIA_H
#define T2_VALVESPIRATORIA_H
#include "stdint.h"

void ValvEsp_init(void);
void ValvEspSetPos(uint16_t);
void ValvEspSetClosed(void);
void ValvEspSetOpenedN(void);
void ValvEspSetOpenedF(void);
//void pid_control_peep(uint16_t newvalue, uint16_t setvalue);
void pid_control_expiration(uint16_t newvalue, uint16_t setvalue);
void ValvEspDecOpenF();
void ValvEspIncOpenF();
void pid_set_one_expiration();
void pid_set_two_expiration();
void ValvEspResetPID();
void ValvEspSetValueF(uint16_t valor);
void ValvEspSetValueN(void);
//void pid_set_three_expiration();
#endif //T2_VALVESPIRATORIA_H

