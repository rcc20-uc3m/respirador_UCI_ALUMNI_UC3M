//
// Created by diego on 12/2/21.
//

#ifndef T2_VALVINSPIRATORIA_H
#define T2_VALVINSPIRATORIA_H
void ValvInspSetPos(uint16_t pos);
void ValvInspSetClosed(void);
//void ValvInspSetTriggerFlow();
void ValvInsp_init(void);
//void ValvInspSetTriggerFlowValue(uint16_t value);
void pid_control_flow(uint16_t newvalue, uint16_t setvalue);
void pid_control_press(uint16_t newvalue, uint16_t setvalue);
void pid_set_fast_press();
void pid_set_slow_press();
void pid_set_fast_flow();
void pid_set_slow_flow();
void ValvInspResetPID();

#endif //T2_VALVINSPIRATORIA_H
