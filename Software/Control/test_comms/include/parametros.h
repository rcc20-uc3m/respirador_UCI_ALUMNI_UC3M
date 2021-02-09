//
// Created by diego on 6/2/21.
//

#ifndef T2_PARAMETROS_H
#define T2_PARAMETROS_H
#include "stdint.h"


// Fr	BPM	Frecuencia respiratoria	10 – 25
// IEx10	Num	Prop Insp/Espiración	5, 10, 20, 30, 40
// Peep	mmH2O	Presión final de espiración	0 – 150
// PIP	mmH2O	Presión 	0 – 500
// Vt	ml	Volumen maximo respiración	200 – 800
// FiO2	--	Fracción O2 Inspirada	20 -100
// Psop	mmH2O	Presión Soporte

enum class PARAM_OPERACION {Fr, iex10, Peep, Pip, Psop, Vt, FiO2, Count};


// Parámetros Calculados	UDS	Formula	Comentario
// Pr	ms	60000/RR	Periodo respiración
// Ti	ms	Pr*1/(1+IE)	Tiempo total de inspiración
// Te	ms	Pr-Ti	Tiempo total de espiración
// Tie	ms	Ti*(1-%Tp)	Tiempo inspiración efectivo
// Tea	ms	Ti+Te*(1-TRIG_WND)	Tiempo absoluto espiración
// PeakFlow	ml/s	Vt/Tie	Flujo para Control por Volumen
enum class PARAM_CALCULADOS {Pr, Ti, Te, Tie, Tea, PeakFlow, Count};


//Parámetros Configuración
//Tri	ms	Tiempo de rampa de inspiración	120 ms
//%Tp	--	Porcentaje de tiempo de pausa	30%
//Pm	ms	Periodo de muestreo 1 ms	1 ms
//N		muestras para promedio 2**N, Si N = 4, n_samples=2**4 = 16
//Pc	ms	Periodo de comunicaciones	10 ms
//P_alarm	mmH2O	Presión disparo de alarma	700
//V_alarm	ml	Volumen disparo de alarma	900
//FlowChange	%	Para ciclado por flujo	25%

enum class PARAM_CONFIG {tRI, porTp, Pm, N, Pc, Palarm, Valarm, FlowChange, Trig_Wnd, Count};

// Parámetros calculados ciclo
// Ppico	mmH2O
//Pplateau	mmH2O
//Vol_insp	ml
//peepM	mmH2O

enum class PARAM_CICLO {Ppico, Pplateau, Vinsp, peepM, Complianza, Resistencia, Count};


#endif //T2_PARAMETROS_H
