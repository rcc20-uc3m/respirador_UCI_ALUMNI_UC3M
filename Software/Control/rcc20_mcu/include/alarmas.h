//
// Created by diego on 1/3/21.
//

#ifndef T2_ALARMAS_H
#define T2_ALARMAS_H

#include <stdint.h>
#include <parametros.h>

extern uint16_t P_alarmas[];

#define ALARM_PMAX              0
#define ALARM_HIGH_PEEP         1
#define ALARM_LOW_PEEP          2
#define ALARM_HIGH_VOLUME       3
#define ALARM_LOW_VOLUME        4
#define ALARM_HIGH_MV           5
#define ALARM_LOW_MV            6
#define ALARM_GAS_SUPPLY        7
#define ALARM_POWER_SUPPLY      8
#define ALARM_LOW_BATTERY       9
#define ALARM_DISCONNECTION     10

#define VALOR_PMAX      P_alarmas[(int)PARAM_ALARM::ps_max]
#define PEEP_MAX(a)     (float) a * (1.0+(float)(P_alarmas[(int)PARAM_ALARM::delta_peep]/100.0))
#define PEEP_MIN(a)     (float) a * (1.0-(float)(P_alarmas[(int)PARAM_ALARM::delta_peep]/100.0))
#define VT_MAX(a)       (float) a * (1.0+(float)(P_alarmas[(int)PARAM_ALARM::delta_vt]/100.0))
#define VT_MIN(a)       (float) a * (1.0-(float)(P_alarmas[(int)PARAM_ALARM::delta_vt]/100.0))
#define MV_MAX          P_alarmas[(int)PARAM_ALARM::mv_max]
#define MV_MIN          P_alarmas[(int)PARAM_ALARM::mv_min]
#define N_DISCONNECT    P_alarmas[(int)PARAM_ALARM::n_disconnect]
#define PIP_DISCONNECT  P_alarmas[(int)PARAM_ALARM::pip_disconnect]

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

enum class TIPOALARMA {NULA, BAJA, MEDIA, ALTA};

void reset_snd_alarma(void);
void lanzar_snd_alarma_alta(void);
void lanzar_snd_alarma_media(void);
void lanzar_snd_alarma_baja(void);
void run_snd_alarmas(void);

void set_alarm_pmax(uint16_t *alarma);
void set_alarm_low_peep(uint16_t *alarma);
void set_alarm_high_peep(uint16_t *alarma);
void set_alarm_high_volume(uint16_t *alarma);
void set_alarm_low_volume(uint16_t *alarma);
void set_alarm_high_mv(uint16_t *alarma);
void set_alarm_low_mv(uint16_t *alarma);
void set_alarm_gas_supply(uint16_t *alarma);
void set_alarm_power_supply(uint16_t *alarma);
void set_alarm_low_battery(uint16_t *alarma);
void set_alarm_disconnection(uint16_t *alarma);
void reset_alarm_pmax(uint16_t *alarma);
void reset_alarm_low_peep(uint16_t *alarma);
void reset_alarm_high_peep(uint16_t *alarma);
void reset_alarm_high_volume(uint16_t *alarma);
void reset_alarm_low_volume(uint16_t *alarma);
void reset_alarm_high_mv(uint16_t *alarma);
void reset_alarm_low_mv(uint16_t *alarma);
void reset_alarm_gas_supply(uint16_t *alarma);
void reset_alarm_power_supply(uint16_t *alarma);
void reset_alarm_low_battery(uint16_t *alarma);
void reset_alarm_disconnection(uint16_t *alarma);

#endif //T2_ALARMAS_H
