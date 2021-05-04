#ifndef PARAMETROS_H
#define PARAMETROS_H

#include "stdint.h"


// Fr	BPM	Frecuencia respiratoria	10 – 25
// IEx10	Num	Prop Insp/Espiración	5, 10, 20, 30, 40
// Peep	mmH2O	Presión final de espiración	0 – 150
// PIP	mmH2O	Presión 	0 – 500
// Vt	ml	Volumen maximo respiración	200 – 800
// FiO2	--	Fracción O2 Inspirada	20 -100
// Psop	mmH2O	Presión Soporte

#define FR_DEFAULT      12  // frecuencia respiratoria por defecto
#define IEX10_DEFAULT   2   // razón I:E por defecto. Hay una tabla esta corresponde a 1:2
#define PIP_DEFAULT     150 // Valor de la pip por defecto
#define VT_DEFAULT      700 // valor VT defecto
#define FIO2_DEFAULT    21  // Valor FI02
#define PSOP_DEFAULT    100  // valor presión de soporte para SIMV
#define PEEP_DEFAULT    50  // Valor de la peep

//Parámetros Configuración
//Tri	ms	Tiempo de rampa de inspiración
//%Tp	--	Porcentaje de tiempo de pausa
//Pm	ms	Periodo de muestreo 1 ms
//N		muestras para promedio (1, 2, 4, 8, 16 ..)
//Pc	ms	Periodo de comunicaciones
//P_alarm	mmH2O	Presión disparo de alarma
//V_alarm	ml	Volumen disparo de alarma
//FlowChange	%	Para ciclado por flujo
//Trig_Wnd	%	ventana para disparo
//Trig_Mnd	%	Ventana para disparo mandatorio

#define TRI_DEFAULT         120
#define PROPTPAUSA_DEFAULT  0
#define PM_DEFAULT          1
#define N_DEFAULT           10
#define PC_DEFAULT          10
#define PALARM_DEFAULT      700
#define VALARM_DEFAULT      1000
#define FLOW_DEFAULT        25
#define TRIGWND_DEFAULT     65
#define TRIGMND_DEFAULT     10
#define TRIGLVLPRESS_DEFAULT 10
#define TRIGLVLFLOW_DEFAULT 20
#define C0_DEFAULT          50
#define R0_DEFAULT          5
#define TRIGGER_FLOW        20
#define NEWALGORITHM        1


#define PS_EPS              5
#define PS_INC              2
#define PS_INC_TIMER        2
#define PS_PORDEBAJOPIP     10
#define PS_PEEP_MENOS       10
#define PS_CAIDA_TIMER      200
#define PS_SUBIDA_TIMER     200
#define PS_PEEP_CLOSE_MIN   25
#define ESPMIN              0
#define ESPMAXN             70
#define ESPMAXF0            60
#define ESPMAXF1            20
#define INSMIN              0
#define INSMAX              255
#define ESPKP1               300
#define ESPKI1               0
#define ESPKD1               0
#define ESPKP2               300
#define ESPKI2               0
#define ESPKD2               0
#define ESPKP3               300
#define ESPKI3               0
#define ESPKD3               0
#define PRESSKPF            200 //100
#define PRESSKIF            1000
#define PRESSKDF            10
#define PRESSKPS            200 // 200
#define PRESSKIS            200
#define PRESSKDS            20
#define VOLKPF              7000
#define VOLKIF              500
#define VOLKDF              0
#define VOLKPS              500
#define VOLKIS              500
#define VOLKDS              0
#define PS_INC_PEEP_CLOSE   60
#define PS_CAMBIO_PID       85
#define PS_LIMITE_RCBAJO    4



enum class PARAM_OPERACION {Fr, iex10, Peep, Pip, Psop, Vt, FiO2, Count};
uint16_t P_Operacion[(int) PARAM_OPERACION::Count];

// Parámetros Calculados	UDS	Formula	Comentario
// Pr	ms	60000/RR	Periodo respiración
// Ti	ms	Pr*1/(1+IE)	Tiempo total de inspiración
// Te	ms	Pr-Ti	Tiempo total de espiración
// Tie	ms	Ti*(1-%Tp)	Tiempo inspiración efectivo
// Tea	ms	Ti+Te*(1-TRIG_WND)	Tiempo absoluto espiración
// PeakFlow	ml/s	Vt/Tie	Flujo para Control por Volumen
enum class PARAM_CALCULADOS {Pr, Ti, Te, Tie, Tea, Teda, PeakFlow, Count};
uint16_t P_calculados[(int) PARAM_CALCULADOS::Count];

//Parámetros Configuración
//Tri	ms	Tiempo de rampa de inspiración	120 ms
//%Tp	--	Porcentaje de tiempo de pausa	30%
//Pm	ms	Periodo de muestreo 1 ms	1 ms
//N		muestras para promedio 2**N, Si N = 4, n_samples=2**4 = 16
//Pc	ms	Periodo de comunicaciones	10 ms
//P_alarm	mmH2O	Presión disparo de alarma	700
//V_alarm	ml	Volumen disparo de alarma	900
//FlowChange	%	Para ciclado por flujo	25%
//Trig_Wnd	%	ventana para disparo
//Trig_Mnd	%	Ventana para disparo mandatorio


enum class PARAM_CONFIG {Tri, porTp, Pm, N, Pc, Palarm, Valarm, FlowChange, Trig_Wnd, Trig_Mnd,
                         TrigLvl_Press, TrigLvl_Flow, C0, R0, Trigger_Flow, newAlgorithm, Count};
uint16_t P_config[(int) PARAM_CONFIG::Count];

// Parámetros calculados ciclo
// Ppico	mmH2O
//Pplateau	mmH2O
//Vol_insp	ml
//peepM	mmH2O

enum class PARAM_CICLO {Ppico, Pplateau, Vinsp, peepM, Complianza, Resistencia, FI_max, FE_max, MinVol, Alarma, Count};
uint16_t P_ciclo[(int) PARAM_CICLO::Count];


enum class PARAM_HARDW {ps_eps, ps_inc, ps_inc_timer, ps_inc_peep_close, ps_pordebajodepip, ps_peep_menos, ps_caida_timer,
    ps_subida_timer, ps_peep_close_min, ps_cambio_pid, espmin, espmaxN, espmaxF0, espmaxF1, espKp1, espKi1, espKd1,
                        espKp2, espKi2, espKd2, espKp3, espKi3, espKd3, insmin, insmax, pressKpF, pressKiF, pressKdF,
                        pressKpS, pressKiS, pressKdS, volKpF, volKiF, volKdF, volKpS, volKiS, volKdS, ps_limite_rcbajo,
                        Count};

uint16_t P_hardware[(int)PARAM_HARDW::Count];


enum class TIPOALARMA {NULA, BAJA, MEDIA, ALTA};

enum class PARAM_ALARM {ps_max, delta_peep, delta_vt, mv_max, mv_min, n_disconnect, pip_disconnect, Count};
uint16_t P_alarmas[(int)PARAM_ALARM::Count];

#endif // PARAMETROS_H
