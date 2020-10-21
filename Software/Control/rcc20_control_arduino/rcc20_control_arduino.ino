/*
  Modificado el 08/09/20
  Le añado una bandera de inspiracion/espiracion para informar al PC
  Le añado el protocolo de comunicaciones con el PC
  Hacemos un Look up table para ajuste de los sensores de flujo. 
  En la version V18Bd ya funcionaba el trigger de presión
  En esta versión vamos a implementar el trigger de flujo
  Programa para el manejo del respirador V16. TODO OK.
  CON LA ELECTRONICA NUEVA ***************************************************************
  CON LA VALVULA DE ESPIRACION CON SERVO 
  Derivado del programa de la consola vad
   
  Identificador del dispositivo: "RSP" (Respirador)).
  Utiliza un procesador Arduino Due
  Linea serie a 115.000 baudios para comunicar con PC

  Utiliza dos salidas digitales: la 13 para la EV de Inspiracion y la 12 para la de espiración

  COMANDOS:

  RSPP            Para el respirador
  RSPM            Pone en marcha el respirador
  RSPFnnn         Establece frecuencia respiratoria
  RSPSnnn         Establece el % de inspiracion
  RSPAnnn         Establece el tiempo de apertura de la valv inspiratoria en ms * 10
  RSPEnnn         Establece el nivel de PEEP en cuentas
  RSPHnnn         Establece la presión máxima inspiratoria en cuentas
  RSPVnnn         Establece el volumen tidal en ml

  Utilizamos el pin 6 EVMOT_ESP_HOME para enviar la posicion al servo

  Se implementa el poder modificar los coeficientes del control de la espiración 
  dependiendo de la complianza del pulmón
*/

#include <Servo.h>
#include "PID_v1.h"  // PID Library from https://github.com/br3ttb/Arduino-PID-Library/
//PID Library description http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

#define PRS_CTRL 0            // Define el modo de control por presión
#define VOL_CTRL 1            // Define el modo de control por volumen
#define FLW_CTRL 2            // Define el modo de control por flujo del volumen
#define TRIG_PR 1             // Trigger (disparo) por presión
#define TRIG_VO 2             // Trigger (disparo) por volumen
#define SIN_TRIG 0            // Sin trigger

Servo myservo;

// ***************************************** ASIGNACION DE PINES ***************************************
// El control del servo se asocia al pin 6 que antes era el pin de fin de carrera del paso a paso
int ANINSP = 5;               // Salida PWM para EV analógica de inspiración
int SENSPR = A2;              // Entrada analógica para el sensor de presión
int SENSFI = A1;              // Entrada analógica para el sensor de flujo Inspiratorio
int SENSFE = A0;              // Entrada analógica para el sensor de flujo Espiratorio
int SENSO2 = A3;              // Entrada analógica para el sensor de O2

// ******************************* Alarmas **************************************************************
unsigned int Alarma = 0;
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

// ******************************* Variables de control del ciclo respiratorio **************************
int Periodo =1000;            // Duración del ciclo en milisegundos
int DurEsp = 500;             // Duracion de la Espiración en mseg
int DurIns;                   // Duración de la Inspiración en mseg
int Freq = 15;                // Frecuencia respiratoria. Respiraciones por minuto
int PorcEsp = 70;             // Tanto por ciento de espiracion
float peakFlow;               // Flujo Inspiratorio en el control por volumen
int Tinsp;                    // Tiempo de apertura de la valvula inspiratoria en el control por flujo (mseg)
int Complianza;               // Complianza introducida por el usuario (baja = 0, media = 1, alta = 2)
boolean bandInsp = false;     // Bandera de inspiracion para que el PC conozca el estado del ciclo.

// **************************** Variables de control de Flujo y Presión *********************************
float Press_H2O = 0;          // Presión en cm H2O
float PEEP_H2O = 5.0;         // PEEP en cm H2O
float inspFlow_lm = 0;        // Flujo inspiratorio en l/min
float expFlow_lm = 0;         // Flujo espiratorio en l/min
float inspVol_ml = 0;         // Volumen inspirado en ml
float expVol_ml = 0;          // Volumen espirado en ml
float FiO2 = 0;               // Valor de FiO2 calculada
unsigned long sensP = 0;      // Valor del sensor de presión en cuentas
unsigned long sensFI = 0;     // Valor del sensor de flujo Inspiratorio en cuentas
unsigned long maxsensFI = 0;  // Valor máximo en cuentas leido por el sensor
unsigned long sensFE = 0;     // Valor del sensor de flujo Espiratorio en cuentas
unsigned long sensO2 = 0;     // Valor del sensor de O2 en cuentas

// *********************************** Variables de control del trigger *********************************
float pressAnt;               // Medida anterior de presión
float incPress;               // Incremento de presión para detectar trigger
float flowAnt;                // Medida anterior de volumen
float incFlow;                // Incremento de volumen para detectar el trigger
int TriggerMode = 0;          // Se inicia sin trigger
int inspValue = 0;            // Valor de apertura de la válvula Inspiratoria dependiendo del trigger

// ******************************* Variables de control del flujo de programa ***************************
boolean Marcha = false;       // Bandera de marcha de la bomba
unsigned long tact;
unsigned long tant;
unsigned long paso;
unsigned long contmseg;       // contador de milisegundos
unsigned long contcseg;       // contador de centesimas de segundo
unsigned long contdseg;       // contador de decimas de segundo
unsigned long contseg;        // contador de segundos
unsigned long contPaso;       // contador para el periodo de latido
unsigned long contSens;       // contador de sensor
unsigned long contO2;         // contador del sensor de O2

// ***************************************+ LOOK UP TABLE PARA EL FLUJO ***********************************************************
float flow[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.0187,0.072,
0.1253,0.1786,0.2319,0.2852,0.3385,0.3918,0.4451,0.4984,0.5517,0.605,0.6583,0.7116,0.7649,0.8182,0.8715,0.9248,0.9781,1.0314,
1.0847,1.138,1.1913,1.2446,1.2979,1.3512,1.4045,1.4578,1.5111,1.5644,1.6177,1.671,1.7243,1.7776,1.8309,1.8842,1.9375,1.9908,
2.0441,2.0974,2.1507,2.204,2.2573,2.3106,2.3639,2.4172,2.4705,2.5238,2.5771,2.6304,2.6837,2.737,2.7903,2.8436,2.8969,2.9502,
3.0035,3.0568,3.1101,3.1634,3.2167,3.27,3.3233,3.3766,3.4299,3.4832,3.5365,3.5898,3.6431,3.6964,3.7497,3.803,3.8563,3.9096,
3.9629,4.0162,4.0695,4.1228,4.1761,4.2294,4.2827,4.336,4.3893,4.4426,4.4959,4.5492,4.6025,4.6558,4.7091,4.7624,4.8157,4.869,
4.9223,4.9756,5.0289,5.0822,5.1355,5.1888,5.2421,5.2954,5.3487,5.402,5.4553,5.5086,5.5619,5.6152,5.6685,5.7218,5.7751,5.8284,
5.8817,5.935,5.9883,6.0416,6.0949,6.1482,6.2015,6.2548,6.3081,6.3614,6.4147,6.468,6.5213,6.5746,6.6279,6.6812,6.7345,6.7878,
6.8411,6.8944,6.9477,7.001,7.0543,7.1076,7.1609,7.2142,7.2675,7.3208,7.3741,7.4274,7.4807,7.534,7.5873,7.6406,7.6939,7.7472,
7.8005,7.8538,7.9071,7.9604,8.0137,8.067,8.1203,8.1736,8.2269,8.2802,8.3335,8.3868,8.4401,8.4934,8.5467,8.6,8.6533,8.7066,
8.7599,8.8132,8.8665,8.9198,8.9731,9.0264,9.0797,9.133,9.1863,9.2396,9.2929,9.3462,9.3995,9.4528,9.5061,9.5594,9.6127,9.666,
9.7193,9.7726,9.8259,9.8792,9.9325,9.9858,10.0391,10.0924,10.1457,10.199,10.2523,10.3056,10.3589,10.4122,10.4655,10.5188,
10.5721,10.6254,10.6787,10.732,10.7853,10.8386,10.8919,10.9452,10.9985,11.0518,11.1051,11.1584,11.2117,11.265,11.3183,11.3716,
11.4249,11.4782,11.5315,11.5848,11.6381,11.6914,11.7447,11.798,11.8513,11.9046,11.9579,12.0112,12.0645,12.1178,12.1711,12.2244,
12.2777,12.331,12.3843,12.4376,12.4909,12.5442,12.5975,12.6508,12.7041,12.7574,12.8107,12.864,12.9173,12.9706,13.0239,13.0772,
13.1305,13.1838,13.2371,13.2904,13.3437,13.397,13.4503,13.5036,13.5569,13.6102,13.6635,13.7168,13.7701,13.8234,13.8767,13.93,
13.9833,14.0366,14.0899,14.1432,14.1965,14.2498,14.3031,14.3564,14.4097,14.463,14.5163,14.5696,14.6229,14.6762,14.7295,14.7828,
14.8361,14.8894,14.9427,14.996,15.0493,15.1026,15.1559,15.2092,15.2625,15.3158,15.3691,15.4224,15.4757,15.529,15.5823,15.6356,
15.6889,15.7422,15.7955,15.8488,15.9021,15.9554,16.0087,16.062,16.1153,16.1686,16.2219,16.2752,16.3285,16.3818,16.4351,16.4884,
16.5417,16.595,16.6483,16.7016,16.7549,16.8082,16.8615,16.9148,16.9681,17.0214,17.0747,17.128,17.1813,17.2346,17.2879,17.3412,
17.3945,17.4478,17.5011,17.5544,17.6077,17.661,17.7143,17.7676,17.8209,17.8742,17.9275,17.9808,18.0341,18.0874,18.1407,18.194,
18.2473,18.3006,18.3539,18.4072,18.4605,18.5138,18.5671,18.6204,18.6737,18.727,18.7803,18.8336,18.8869,18.9402,18.9935,19.0468,
19.1001,19.1534,19.2067,19.26,19.3133,19.3666,19.4199,19.4732,19.5265,19.5798,19.6331,19.6864,19.7397,19.793,19.8463,19.8996,
19.9529,20.0062,20.0595,20.1128,20.1661,20.2194,20.2727,20.326,20.3793,20.4326,20.4859,20.5392,20.5925,20.6458,20.6991,20.7524,
20.8057,20.859,20.9123,20.9656,21.0189,21.0722,21.1255,21.1788,21.2321,21.2854,21.3387,21.392,21.4453,21.4986,21.5519,21.6052,
21.6585,21.7118,21.7651,21.8184,21.8717,21.925,21.9783,22.0316,22.0849,22.1382,22.1915,22.2448,22.2981,22.3514,22.4047,22.458,
22.5113,22.5646,22.6179,22.6712,22.7245,22.7778,22.8311,22.8844,22.9377,22.991,23.0443,23.0976,23.1509,23.2042,23.2575,23.3108,
23.3641,23.4174,23.4707,23.524,23.5773,23.6306,23.6839,23.7372,23.7905,23.8438,23.8971,23.9504,24.0037,24.057,24.1103,24.1636,
24.2169,24.2702,24.3235,24.3768,24.4301,24.4834,24.5367,24.59,24.6433,24.6966,24.7499,24.8032,24.8565,24.9098,24.9631,25.0531,
25.1804,25.3077,25.435,25.5623,25.6896,25.8169,25.9442,26.0715,26.1988,26.3261,26.4534,26.5807,26.708,26.8353,26.9626,27.0899,
27.2172,27.3445,27.4718,27.5991,27.7264,27.8537,27.981,28.1083,28.2356,28.3629,28.4902,28.6175,28.7448,28.8721,28.9994,29.1267,
29.254,29.3813,29.5086,29.6359,29.7632,29.8905,30.0178,30.1451,30.2724,30.3997,30.527,30.6543,30.7816,30.9089,31.0362,31.1635,
31.2908,31.4181,31.5454,31.6727,31.8,31.9273,32.0546,32.1819,32.3092,32.4365,32.5638,32.6911,32.8184,32.9457,33.073,33.2003,
33.3276,33.4549,33.5822,33.7095,33.8368,33.9641,34.0914,34.2187,34.346,34.4733,34.6006,34.7279,34.8552,34.9825,35.1098,35.2371,
35.3644,35.4917,35.619,35.7463,35.8736,36.0009,36.1282,36.2555,36.3828,36.5101,36.6374,36.7647,36.892,37.0193,37.1466,37.2739,
37.4012,37.5285,37.6558,37.7831,37.9104,38.0377,38.165,38.2923,38.4196,38.5469,38.6742,38.8015,38.9288,39.0561,39.1834,39.3107,
39.438,39.5653,39.6926,39.8199,39.9472,40.0745,40.2018,40.3291,40.4564,40.5837,40.711,40.8383,40.9656,41.0929,41.2202,41.3475,
41.4748,41.6021,41.7294,41.8567,41.984,42.1113,42.2386,42.3659,42.4932,42.6205,42.7478,42.8751,43.0024,43.1297,43.257,43.3843,
43.5116,43.6389,43.7662,43.8935,44.0208,44.1481,44.2754,44.4027,44.53,44.6573,44.7846,44.9119,45.0392,45.1665,45.2938,45.4211,
45.5484,45.6757,45.803,45.9303,46.0576,46.1849,46.3122,46.4395,46.5668,46.6941,46.8214,46.9487,47.076,47.2033,47.3306,47.4579,
47.5852,47.7125,47.8398,47.9671,48.0944,48.2217,48.349,48.4763,48.6036,48.7309,48.8582,48.9855,49.1128,49.2401,49.3674,49.4947,
49.622,49.7493,49.8766,50.0039,50.3202,50.5425,50.7648,50.9871,51.2094,51.4317,51.654,51.8763,52.0986,52.3209,52.5432,52.7655,
52.9878,53.2101,53.4324,53.6547,53.877,54.0993,54.3216,54.5439,54.7662,54.9885,55.2108,55.4331,55.6554,55.8777,56.1,56.3223,
56.5446,56.7669,56.9892,57.2115,57.4338,57.6561,57.8784,58.1007,58.323,58.5453,58.7676,58.9899,59.2122,59.4345,59.6568,59.8791,
60.1014,60.3237,60.546,60.7683,60.9906,61.2129,61.4352,61.6575,61.8798,62.1021,62.3244,62.5467,62.769,62.9913,63.2136,63.4359,
63.6582,63.8805,64.1028,64.3251,64.5474,64.7697,64.992,65.2143,65.4366,65.6589,65.8812,66.1035,66.3258,66.5481,66.7704,66.9927,
67.215,67.4373,67.6596,67.8819,68.1042,68.3265,68.5488,68.7711,68.9934,69.2157,69.438,69.6603,69.8826,70.1049,70.3272,70.5495,
70.7718,70.9941,71.2164,71.4387,71.661,71.8833,72.1056,72.3279,72.5502,72.7725,72.9948,73.2171,73.4394,73.6617,73.884,74.1063,
74.3286,74.5509,74.7732,74.9955,75.3022,75.6849,76.0676,76.4503,76.833,77.2157,77.5984,77.9811,78.3638,78.7465,79.1292,79.5119,
79.8946,80.2773,80.66,81.0427,81.4254,81.8081,82.1908,82.5735,82.9562,83.3389,83.7216,84.1043,84.487,84.8697,85.2524,85.6351,
86.0178,86.4005,86.7832,87.1659,87.5486,87.9313,88.314,88.6967,89.0794,89.4621,89.8448,90.2275,90.6102,90.9929,91.3756,91.7583,
92.141,92.5237,92.9064,93.2891,93.6718,94.0545,94.4372,94.8199,95.2026,95.5853,95.968,96.3507,96.7334,97.1161,97.4988,97.8815,
98.2642,98.6469,99.0296,99.4123,99.795,100.1777,101.0876,101.8489,102.6102,103.3715,104.1328,104.8941,105.6554,106.4167,107.178,
107.9393,108.7006,109.4619,110.2232,110.9845,111.7458,112.5071,113.2684,114.0297,114.791,115.5523,116.3136,117.0749,117.8362,
118.5975,119.3588,120.1201,120.8814,121.6427,122.404,123.1653,123.9266,124.6879,125.4492,126.2105,126.9718,127.7331,128.4944,
129.2557,130.017,130.7783,131.5396,132.3009,133.0622,133.8235,134.5848,135.3461,136.1074,136.8687,137.63,138.3913,139.1526,
139.9139,140.6752,141.4365,142.1978,142.9591,143.7204,144.4817,145.243,146.0043,146.7656,147.5269,148.2882,149.0495,149.8108,
151.1852,152.6608,154.1364,155.612,157.0876,158.5632,160.0388,161.5144,162.99,164.4656,165.9412,167.4168,168.8924,170.368,
171.8436,173.3192,174.7948,176.2704,177.746,179.2216,180.6972,182.1728,183.6484,185.124,186.5996,188.0752,189.5508,191.0264,
192.502,193.9776,195.4532,196.9288,198.4044,199.88};

// ****************************** Variables para lectura de la linea serie *******************************
char ByteR;                   // Byte leido por la linea serie
String StrS = "";             // String para almacenar los datos entrantes
boolean SFin = false;         // Bandera de string completa
String Sm = "";               // String para construir el mensaje de salida con los datos
char sinfo[30];               // String para construir el mensaje de salida con los datos, junto con Sm

int bandenvio = 0;            // Bandera para enviar datos cada 20 mseg 
short operationMode;          // Modo de operación PRS_CTRL, VOL_CTRL ó FLW_CTRL

float Pset_H2O = 20.00;       // presion deseada para control por presion
float Pmax_H2O = 45.00;       // valor en el cual se levantaría la señal de alarma
float Vset_ml = 300.00;       // volumen deseado para control por volumen
float Vmax_ml = 700.00;       // por encima de este valor se levanta la alarma

//int esplimit = 180;         // Valor del límite de la valvula espiratoria
int esplimit = 45;            // Valor del límite de la valvula espiratoria con Complianza Baja  REVISARLO !!!!!!!!!!!
int esplow = 0;              // Valor del límite inferior de la valvula espiratoria

//******************************** Variables de los PID ****************************************************
//******* Set=Valor de referencia; In=Valor de entrada al PID; Out=Valor de salida de PID ******************
double pressSet, pressIn, pressOut;     // Presión
double volSet, volIn, volOut;           // Volumen
double presIn, angOut, peepSet;         // Angulo del servo de la Valvula de espiracion
double flowSet, flowIn, flowOut;        // Peak_Flow 

int angulo;                             // Angulo del servo de la Valvula de espiracion

// COEFICIENTES DEL PID DE PRESION *********************
double pressKp=100, pressKi=10, pressKd=1;

// COEFICIENTES DEL PID DE VOLUMEN *********************
double volKp=0.445, volKi=0, volKd=0;

// COEFICIENTES DEL PID DE FLUJO *********************
double flowKp=100, flowKi=10, flowKd=0;

// COEFICIENTES DEL PID DE LA VALVULA ESPIRATORIA ******
double serKp= 15, serKi = 0, serKd = 0;  // Para complianza normal
//double serKp= 15, serKi = 0, serKd = 15;  // Para complianza disminuida

//**********************************++++ Crea los controles PID ******************************************************************
PID pressPID(&pressIn, &pressOut, &pressSet, pressKp, pressKi, pressKd, DIRECT);
PID volPID(&volIn, &volOut, &volSet, volKp, volKi, volKd, DIRECT);
PID flowPID(&flowIn, &flowOut, &flowSet, flowKp, flowKi, flowKd, DIRECT);
PID servoPID(&presIn, &angOut, &peepSet, serKp, serKi, serKd, REVERSE);


// **********************************               Rutina de inicializacion           ****************************************************
void setup() {
  myservo.attach(6);                         // Inicializa el servo de la Valvula espiratoria en el PIN 6
  
  pinMode(SENSPR, INPUT);                    // Define el sensor de presión como entrada
  pinMode(SENSFI, INPUT);                    // Define el sensor de Flujo Inspiratorio como entrada
  pinMode(SENSFE, INPUT);                    // Define el sensor de Flujo Espiratorio como entrada
  pinMode(ANINSP, OUTPUT);                   // Define el pin de la valvula inspiratoria analógica como salida

  
  Serial.begin(115200);                     // Con 230400 baudios no funciona
  //Serial1.begin(115200);                     // Con 230400 baudios no funciona
  //Serial1.begin(230400);                   // No funciona el puerto de ubuntu solo llega a 115200

  StrS.reserve(50);                          // Reserva 50 caracteres para Strs
  Sm.reserve(100);                           // reserva 100 caracteres para Sm
  contmseg = 0;                              // Inicializa el contador de mseg
  tant = micros();                           // Inicializa tant
  contPaso = 0;                              // Inicializa el contador de pasos

  pressPID.SetMode(AUTOMATIC);               // Configura el PID de presión en automatico
  //pressPID.SetSampleTime(10);                // Tiempo de muestreo 10 mseg Para el Due
  pressPID.SetSampleTime(1);               // Tiempo de muestreo 1 mseg
  pressPID.SetOutputLimits(100,255);         // Limita la salida entre 100 y 255 (100 valvula insp cerrada, 255   

  volPID.SetMode(AUTOMATIC);                 // Configura el PID de volumen en automatico
  //volPID.SetSampleTime(10);                // Tiempo de muestreo 10 mseg
  volPID.SetSampleTime(1);                   // Tiempo de muestreo 1 mseg
  volPID.SetOutputLimits(100,255);           // Limita la salida entre 100 y 255 (100 valvula insp cerrada, 255 abierta del todo)

  servoPID.SetMode(AUTOMATIC);               // Configura el PID de valvula de espiración
  servoPID.SetSampleTime(1);                 // Tiempo de muestreo 1 mseg
  servoPID.SetOutputLimits(esplow,esplimit); // Limita la salida entre esplow y esplimit (esplow cerrada, esplimit abierta del todo)
  
  flowPID.SetMode(AUTOMATIC);                // Configura el PID del control del peakFlow
  flowPID.SetSampleTime(1);                  // Tiempo de muestreo 1 mseg
  flowPID.SetOutputLimits(100,255);          // Limita la salida entre 100 y 255 (100 valvula insp cerrada, 255 abierta del todo)
  
  Tinsp = 500;                               // Establece Tinsp a 500 mseg. Tiempo da apertura de la val insp en control peakflow

  Serial.println("COMIENZO");
  //Serial1.println("COMIENZO");
  analogWrite (ANINSP,0);                    // Cierra la válvula inspiratoria analógica
  
  operationMode = PRS_CTRL;                  // Establece el modo de operación en control por presión
  
  
  pressKp=300, pressKi=100, pressKd=1;                  // COEFICIENTES DEL PID DE PRESION *********************
  volKp=0.445, volKi=0, volKd=0;                        // COEFICIENTES DEL PID DE VOLUMEN *********************
  flowKp=100, flowKi=50, flowKd=5;                      // COEFICIENTES DEL PID DE FLUJO *********************
  //serKp= 100, serKi = 0, serKd = 0; esplimit = 15;      // Coeficientes del PID ESPIRACION para complianza Baja
  serKp= 15, serKi = 0, serKd = 0; esplimit = 180;     // Coeficientes del PID ESPIRACION para complianza alta
  //serKp= 15, serKi = 0, serKd = 15; esplimit = 180;
}
// *********************    FIN Inicializacion ***************************************

// ******************************************** INICIO DEL BUCLE INFINITO **************************************************************
void loop() {

  tact = micros();
  paso = tact - tant;
// ******************************* LEE LA LINEA SERIE ******************************    
  leeserie();   

  // ********************************************* CADA MILISEGUNDO ************************************************
    if (paso >= 1000) {    
      contmseg++;                                     // Incrementa el contador de milisegundos
      Periodo = 60000 / Freq;                         // Calcula el periodo
      DurEsp = PorcEsp * (Periodo / 100);             // Calcula la duracion de la espiración DurEsp
      DurIns = Periodo - DurEsp;
      peakFlow = (float)Vset_ml/(float)Tinsp*60.0;    // peakFlow in lpm
      
      // **************************** LEE LOS SENSORES ***************************************
      leesensores();
 
       if (Marcha) {  // *********************** COMIENZA MARCHA *************************************************************
        contPaso++;                                   // Incrementa el contador de pasos
        inspVol_ml += (inspFlow_lm  * 0.1667);          // Integra el flujo inspiratorio
        //if (inspVol_ml > 9000) inspVol_ml = 9000;
        // ************************************* Inicia ESPIRACION ***********************************************************
        if (contPaso < DurEsp){
          bandInsp = false;                                             // Señala al PC que entramos en espiración
          analogWrite(ANINSP,inspValue);                                // Cierra Valv Insp al valor dependiente del modo de trigger
          presIn = Press_H2O; peepSet = PEEP_H2O; servoPID.Compute();   // Calcula Posicion Valvula Servo con el PID
          angulo = (int) angOut;
          myservo.write(angulo);                                        // Actualiza el angulo del servo de la valv esp

          // ********************************** DETECCION DEL TRIGGER **********************************
          if(contPaso > DurEsp/3){                               // Sólo actua a partir de la tercera parte de la espiración
            if(abs(PEEP_H2O - Press_H2O) < 0.5){                 // Si la presión está proxima a la PEEP 
              switch(TriggerMode){                               // Actua dependiendo del modo de trigger
                case 0: {                                        // SIN TRIGGER
                    inspValue = 0;                               // Valvula Inspiratoria cerrada
                break; } 
                case 1: {                                        // TRIGGER POR PRESIÓN
                  inspValue = 0;                                 // Valvula Inspiratoria cerrada
                  // if(abs(incPress) > 0.5) contPaso = DurEsp;  // Si hay una diferencia de 0.5 cmH2O, fuerza inspiracion
                   if(abs(incPress) > 2) contPaso = DurEsp;      // Si hay una diferencia de 2 cmH2O, fuerza inspiracion
                break; } 
                case 2: {                                         // TRIGGER POR VOLUMEN  HACERLO POR DIF. ENTRE INSP Y ESP
                  inspValue = 110;                                // Valvula Inspiratoria ligeramente abierta ??????
                    if(abs(incFlow) > 3) contPaso = DurEsp;       // Si hay una diferencia de 3 cc, fuerza inspiracion
                break; } 
              }                 // Fin de TriggerMode
            }                   // Fin de si la presión está próxima a la PEEP
            if (Press_H2O < PEEP_H2O-2.0) BIT_SET(Alarma, 2); //Alarma de peep más bajo de un 1cm del set
            if ( (operationMode == VOL_CTRL) && (inspVol_ml < Vset_ml-10) ) BIT_SET(Alarma, 3); // Alarma de Vtidal menor de 10 ml del set
          }                     // Fin de que solo acúa en la tercera fase de la inspiracion
        }                       // Fin de la ESPIRACION
        
        // ************************************* Inicia INSPIRACION ***********************************************************
        if (contPaso >= DurEsp) {
          bandInsp = true;
          myservo.write(esplow);                                      // Cierra Válvula Espiratoria
                 //if (inspVol_ml > Vmax_ml)   RS{setAlarm();}        // Si se alcanza el volumen maximo absoluto da alarma
                 //if(Press_mmHg > Pmax_H2O)  {setAlarm(); }          // Si se supera la presión máxima Da alarma
          if (inspVol_ml > Vmax_ml)   BIT_SET(Alarma, 0);  // Si se alcanza el volumen maximo absoluto da alarma
          if (Press_H2O > Pmax_H2O)   BIT_SET(Alarma, 1);   // Si se supera la presión máxima Da alarma
          // SWITCH MODO DE OPERACION *****************************************************************************************         
          switch(operationMode){   
             // CONTROL POR PRESIÓN *********************************************************
             case PRS_CTRL: {  
                if (Press_H2O >= Pset_H2O ){analogWrite(ANINSP,0);}   // Si alcanza el set de presión cierra la válvula inspiratoria
                else {                                                // Si no se ha alcanzado Pset calcula PID
                  pressSet = Pset_H2O; pressIn = Press_H2O; pressPID.Compute();               
                  analogWrite(ANINSP,pressOut); }                     // Actualiza la salida de la valvula
             break;}   
             // FIN DE CONTROL POR PRESION ****************************************************
 
             // CONTROL POR VOLUMEN ***********************************************************
             case VOL_CTRL: {   
                 if(inspVol_ml >= Vset_ml){analogWrite(ANINSP,0);}    // Si alcanza el set de volumen cierra la válvula inspiratoria 
                 else {                                               // Si no se ha alcanzado Vset Calcula PID
                    volSet = Vset_ml;  volIn = inspVol_ml;  volPID.Compute();
                    analogWrite(ANINSP,volOut); }                     // Actualiza la salida de la valvula
             break; }    
             // FIN DE CONTROL POR VOLUMEN  **************************************************
             
             // CONTROL POR FLUJO DEL VOLUMEN ***********************************************************
             case FLW_CTRL: {   
                 if(inspVol_ml >= Vset_ml){analogWrite(ANINSP,0);}    // Si alcanza el set de volumen Cierra válv. inspiratoria 
                 else {                                               // Si no se ha alcanzado Vset Calcula PID
                    flowSet = peakFlow;  flowIn = inspFlow_lm;  flowPID.Compute();
                    analogWrite(ANINSP,flowOut);}                     // Actualiza la salida de la valvula
             break; }    
             // FIN DE CONTROL POR FLUJO  **************************************************
          }
        }
        // Fin del Periodo **************************************************************
        if (contPaso > Periodo){ 
          contPaso = 0;
          myservo.write(esplimit);          // Abre la válvula espiratoria 
          inspVol_ml = 0;
        }
      } // **********************************  Termina if Marcha ******************************** 
      else {analogWrite(ANINSP,0);            // Cierra valvula inspiratoria
            myservo.write(esplimit);          // Abre la válvula espiratoria
      }
      paso = 0;
      tant = micros();
      
      
    } // *************************** Fin de cada mseg ****************************

    // ***************************** Cada centesima de segundo *******************
    if (contmseg >= 10) {
      contcseg++;  
      enviadatos();   // ********************** ENVIA DATOS AL PC *********************************************
      contmseg = 0; }
 
    // *****************************   Cada decima de segundo  *******************
    if (contcseg >= 10) { 
      contdseg++; 

      // CALCULA LOS INCREMENTOS DE PRESIÓN Y FLUJO PARA LOS TRIGGERS
      incPress = Press_H2O - pressAnt;
      pressAnt = Press_H2O;
      // incFlow lo debe calcular como la diferencia entre expFlow e insFlow
      incFlow = expFlow_lm - flowAnt;
      flowAnt = expFlow_lm;
     
      if (contO2 < 10) {  // Mide la FiO2 cada décima de segundo, evita ruido. 
        contO2++;
        int so = analogRead(SENSO2);   sensO2 = sensO2 + so ; 
      }

      if (contO2 >= 10){  // Cada segundo calcula la media de la FiO2
        sensO2 = sensO2/10; 
       FiO2 = 0.0001*sensO2*sensO2 - 0.0649*sensO2 + 29.475;
        contO2 = 0;
      }
      contcseg = 0; }

    // *****************************        Cada segundo       *******************
    if (contdseg >= 10) { 
      contseg++; 
      // No hace nada
      contdseg = 0;  }
}
//*************************** fin de loop *************************************

//************************************** LEE LA LINEA SERIE **************************************
void leeserie(){
  int a, b, c, d, e;
  
  if (Serial.available() > 0) {
      ByteR = (char)Serial.read();      // lee el byte entrante:
      StrS += ByteR;
      if (ByteR == '\n') SFin = true;
  }
  if (SFin == true) {                  // Cadena leida, StrS completa

    if (StrS[0] == 'R' && StrS[1] == 'S' && StrS[2] == 'P') {
       switch(StrS[3]){
          case 'B': {  //lee MAX PIP **********************************************************
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);
            d = a + b + c;    Pmax_H2O = d;  
          break; }
          case 'C': {  //lee max Vt **********************************************************
            a = (int(StrS[4]) - 48) * 1000;  b = (int(StrS[5]) - 48) * 100; c = (int(StrS[6]) - 48)*10;
            e = (int(StrS[7]) - 48);
            d = a + b + c + e;    Vmax_ml = d;  
          break; }
          
          case 'F': {  //lee frecuencia **********************************************************
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);
            d = a + b + c;    Freq = d;  
            //Serial.print("FREQ=");  Serial.println(Freq);
          break; }
          case 'S': {  //  "RSPSnnn" % Inspiracion ***********************************************
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);        
            d = a + b + c;  PorcEsp = 100 - d; 
          break; }
          case 'A': { // Lee el modo de TRIGGER SIN=0, Pres= 1, Vol=2 ****************************
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);        
            d = a + b + c;  TriggerMode = d; 
          break; }
          case 'E': {  // lee PEEP ***************************************************************
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);
            d = a + b + c;    PEEP_H2O = (float)d / 10;  
          break; }
          case 'G': {  // lee PIP *****************************************************************
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);
            d = a + b + c;    Pset_H2O = d;  
          break;}
          case 'V': { // lee Vt
            a = (int(StrS[4]) - 48) * 1000;  b = (int(StrS[5]) - 48) * 100; c = (int(StrS[6]) - 48)*10;        
            e = (int(StrS[7]) - 48);
            d = a + b + c + e;  Vset_ml = d;
          break; }
          case 'L': { // Cambio de modo 
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);
            d = a + b + c;    operationMode = d;
          break; }
          case 'M': { 
            Marcha = true; 
          break; }
          case 'P': { 
            Marcha = false; 
            analogWrite(ANINSP,0);            // Cierra la válvula inspiratoria
            myservo.write(esplimit);          // Abre la válvula espiratoria
            contPaso = 0; 
            bandInsp = false;
          break; }
          case 'I': {                         // tiempo de apertura de la valv insp para el control del peakflow
            a = (int(StrS[4]) - 48) * 1000;  b = (int(StrS[5]) - 48) * 100; c = (int(StrS[6]) - 48)*10;        
            e = (int(StrS[7]) - 48);
            d = a + b + c + e;  Tinsp = d;
            if(Tinsp > (DurIns-100))Tinsp = DurIns -100;
            if(Tinsp < 100)Tinsp = 100;
          break; }
          case 'K': {                         // Cambio de la complianza
            a = (int(StrS[4]) - 48) * 100;  b = (int(StrS[5]) - 48) * 10; c = (int(StrS[6]) - 48);
            d = a + b + c;    Complianza = d;
          break; }
          case 'X': {
            Alarma = 0;
            break; }
       }
       StrS = "";
       SFin = false;
    }
  }
}
// *************************************** FIN LEE LA LINEA SERIE ***********************************************

// ******************** LEE LOS SENSORES (Los actualiza cada 10 mseg) *****************************************
// El sensor de O2 lo lee más lentamente cada 100 mseg
void leesensores(){
      if (contSens < 10) {
        contSens++;
        int s = analogRead(SENSPR);     sensP = sensP + s ;   
        s = analogRead(SENSFI);         sensFI = sensFI + s ;   
        s = analogRead(SENSFE);         sensFE = sensFE + s ;
       }
      if (contSens >= 10){      // Cada 10 lecturas hace la media y calcula los valores
        sensP = sensP / 10; sensFI = sensFI / 10;  sensFE = sensFE / 10;   //sensO2 = sensO2/10;
        contSens = 0;
        Press_H2O = ((((float)sensP)*0.0749)-7.5463);   // Calcula la presión 
        if (Press_H2O > 100) Press_H2O = 100;
        if (Press_H2O < -20) Press_H2O = -20;
        inspFlow_lm = flow[sensFI] * 1.24137;           // Calcula el flujo inspiratorio
         if (inspFlow_lm < 0) inspFlow_lm = 0;
        expFlow_lm = flow[sensFE];                      // Calcula el flujo espiratorio
       } 
}

// **************************** ENVIA DATOS AL PC ************************************************************
void enviadatos(){    // Envía datos cada 20 mseg dependiendo de bandenvio
        sprintf(sinfo, "%01d%01d%01d%02d", Marcha, operationMode, TriggerMode, Freq); 
        Sm.concat(sinfo);
        sprintf(sinfo, "%02d%04d", (int) Pmax_H2O, (int) Vmax_ml);
        Sm.concat(sinfo);
        sprintf(sinfo, "%02d%02d%02d", 100-PorcEsp, (int)PEEP_H2O, (int)Pset_H2O); 
        Sm.concat(sinfo);
        sprintf(sinfo, "%04d%03d%03d", (int)Vset_ml, (int)Press_H2O*10, (int)inspFlow_lm*10); 
        Sm.concat(sinfo);
        sprintf(sinfo, "%03d%04d%03d", (int)expFlow_lm*10, (int)inspVol_ml, (int)FiO2); 
        Sm.concat(sinfo);
        sprintf(sinfo, "%04d%01d%c", (int)Tinsp, bandInsp, (char) Alarma); 
        Sm.concat(sinfo);
        if(Sm.length() == 43){ Serial.println (Sm); }
        Sm = "";
}
 
