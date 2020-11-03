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
float PressLI_H2O = 0;
float PEEP_H2O = 5.0;         // PEEP en cm H2O
float inspFlow_lm = 0;        // Flujo inspiratorio en l/min
float expFlow_lm = 0;         // Flujo espiratorio en l/min
float expFlowLI_lm =0;
float inspVol_ml = 0;         // Volumen inspirado en ml
float expVol_ml = 0;          // Volumen espirado en ml

float ant_inspFlow_lm = 0;        // Flujo inspiratorio en l/min. Valor anterior para la integración
float ant_expFlow_lm = 0;         // Flujo espiratorio en l/min. Valor anterior para la integración

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

// *************************** VARIABLES PARA PROMEDIOS ****************************************
float f_sensFI = 0.0;
float f_sensFE = 0.0;
float f_sensFELI = 0.0;
float f_sensP = 0.0;
float f_sensPLI = 0.0;
uint n_samples = 10;
uint n_ciclos = 10;
float alpha = 0.0;
float beta = 0.0;
float alphaLI = 0.0;
float betaLI = 0.0;



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
int esplimit = 100;            // Valor del límite de la valvula espiratoria con Complianza Baja  REVISARLO !!!!!!!!!!!
int esplow = 0;              // Valor del límite inferior de la valvula espiratoria

//******************************** Variables de los PID ****************************************************
//******* Set=Valor de referencia; In=Valor de entrada al PID; Out=Valor de salida de PID ******************
double pressSet, pressIn, pressOut;     // Presión
double volSet, volIn, volOut;           // Volumen
double presIn, angOut, peepSet;         // Angulo del servo de la Valvula de espiracion
double flowSet, flowIn, flowOut;        // Peak_Flow 

int angulo;                             // Angulo del servo de la Valvula de espiracion

// COEFICIENTES DEL PID DE PRESION *********************
double pressKp=90, pressKi=4, pressKd=2;
// COEFICIENTES DEL PID DE VOLUMEN *********************
double flowKp=3, flowKi=0, flowKd=0;
// COEFICIENTES DEL PID DE LA VALVULA ESPIRATORIA ******
double serKp= 30, serKi = 0, serKd = 0;  // Para complianza normal


//**********************************++++ Crea los controles PID ******************************************************************
PID pressPID(&pressIn, &pressOut, &pressSet, pressKp, pressKi, pressKd, DIRECT);
PID flowPID(&flowIn, &flowOut, &flowSet, flowKp, flowKi, flowKd, DIRECT);
PID servoPID(&presIn, &angOut, &peepSet, serKp, serKi, serKd, REVERSE);


int n_puntos = 9;
float xFI[] = {97, 124, 156, 440, 672, 773, 819, 846, 852};
float yFI[] = {0, 2.28, 4.66, 27.4, 56.5, 80.5, 97.2, 108.1, 110.0};
float xFE[] = {85, 110, 139, 413, 639, 738, 788, 805, 810, };

float interpLineal(int n, float pos, float* X, float* Y){
    for(int i = 0; i < n_puntos-1; i++){
        if ((X[i] < pos) && (pos < X[i+1])){
            return yFI[i] + (pos - X[i])/(X[i+1]-X[i])*(Y[i+1]-Y[i]);
        }
    }
    return 0;
}
float getFI(float pos){
    return interpLineal(n_puntos, pos, xFI, yFI);
}
float getFE(float pos){
    return interpLineal(n_puntos, pos, xFE, yFI);
}

float getP(float pos) {
  return pos * 0.07 - 3.8819;
}


// **********************************               Rutina de inicializacion           ****************************************************
void setup() {
  myservo.attach(6);                         // Inicializa el servo de la Valvula espiratoria en el PIN 6
  pinMode(22, OUTPUT);
  pinMode(SENSPR, INPUT);                    // Define el sensor de presión como entrada
  pinMode(SENSFI, INPUT);                    // Define el sensor de Flujo Inspiratorio como entrada
  pinMode(SENSFE, INPUT);                    // Define el sensor de Flujo Espiratorio como entrada
  pinMode(ANINSP, OUTPUT);                   // Define el pin de la valvula inspiratoria analógica como salida

  
  Serial.begin(115200);                     // Con 230400 baudios no funciona

  StrS.reserve(50);                          // Reserva 50 caracteres para Strs
  Sm.reserve(100);                           // reserva 100 caracteres para Sm
  contmseg = 0;                              // Inicializa el contador de mseg
  tant = micros();                           // Inicializa tant
  contPaso = 0;                              // Inicializa el contador de pasos

  pressPID.SetMode(AUTOMATIC);               // Configura el PID de presión en automatico
  pressPID.SetSampleTime(10);               // Tiempo de muestreo 1 mseg
  pressPID.SetOutputLimits(100,255);         // Limita la salida entre 100 y 255 (100 valvula insp cerrada, 255   

  flowPID.SetMode(AUTOMATIC);                // Configura el PID del control del peakFlow
  flowPID.SetSampleTime(10);                  // Tiempo de muestreo 1 mseg
  flowPID.SetOutputLimits(100,255);          // Limita la salida entre 100 y 255 (100 valvula insp cerrada, 255 abierta del todo)

  servoPID.SetMode(AUTOMATIC);               // Configura el PID de valvula de espiración
  servoPID.SetSampleTime(10);                 // Tiempo de muestreo 1 mseg
  servoPID.SetOutputLimits(esplow,esplimit); // Limita la salida entre esplow y esplimit (esplow cerrada, esplimit abierta del todo)
  
  Tinsp = 500;                               // Establece Tinsp a 500 mseg. Tiempo da apertura de la val insp en control peakflow

  Serial.println("COMIENZO");
  //Serial1.println("COMIENZO");
  analogWrite (ANINSP,0);                    // Cierra la válvula inspiratoria analógica
  for (uint i = 0; i < (n_ciclos-1)*n_samples; i++){
    f_sensPLI += (float) analogRead(SENSPR);
    f_sensFELI += (float) analogRead(SENSFE);
    delay(10);
  }
  for (uint i = 0; i < n_samples; i++){
      f_sensFE += (float) analogRead(SENSFE);
      f_sensFELI += (float) analogRead(SENSFE);
      f_sensFI += (float) analogRead(SENSFI);
      f_sensP  += (float) analogRead(SENSPR);
      f_sensPLI+= (float) analogRead(SENSPR);
      delay(10);
  }
  f_sensPLI/= (float) (n_samples*n_ciclos);
  f_sensFELI /= (float) (n_samples*n_ciclos);
  f_sensP  /= (float) n_samples;
  f_sensFI /= (float) n_samples;
  f_sensFE /= (float) n_samples;
  
  beta  = 1.0 / (float) n_samples;
  alpha = 1.0 - beta;
  betaLI = 1.0 / (float)(n_samples*n_ciclos);
  alphaLI = 1.0 - betaLI;

  
  
  operationMode = PRS_CTRL;                  // Establece el modo de operación en control por presión
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

      if (Alarma) {
        digitalWrite(22, HIGH);
      }
      
      f_sensFE = alpha*f_sensFE + beta*(float)analogRead(SENSFE);
      f_sensFI = alpha*f_sensFI + beta*(float)analogRead(SENSFI);
      f_sensP  = alpha*f_sensP  + beta*(float)analogRead(SENSPR);
      f_sensPLI = alphaLI*f_sensPLI  + betaLI*(float)analogRead(SENSPR);
      f_sensFELI= alphaLI*f_sensFELI + betaLI*(float)analogRead(SENSFE);

      Press_H2O = getP(f_sensP);
      PressLI_H2O=getP(f_sensPLI);
      inspFlow_lm = getFI(f_sensFI);
      expFlow_lm = getFE(f_sensFE);
      expFlowLI_lm = getFE(f_sensFELI);
      if (bandInsp) {
        if (inspFlow_lm>0) {
          inspVol_ml += (inspFlow_lm+ant_inspFlow_lm) * 0.008333;
          ant_inspFlow_lm = inspFlow_lm;
        }
        if (expFlow_lm>0)  {
          expVol_ml  += (expFlow_lm+ant_expFlow_lm)  * 0.008333;     
          ant_expFlow_lm = expFlow_lm;
        }
      }
      if (Marcha) {  // *********************** COMIENZA MARCHA *************************************************************
        contPaso++;                                   // Incrementa el contador de pasos
        // ************************************* Inicia INSPIRACION ***********************************************************
        if (contPaso < DurIns) {
          bandInsp = true;
          myservo.write(esplow);                                      // Cierra Válvula Espiratoria
          if (inspVol_ml > Vmax_ml)   BIT_SET(Alarma, 0);  // Si se alcanza el volumen maximo absoluto da alarma
          if (Press_H2O > Pmax_H2O)   BIT_SET(Alarma, 1);   // Si se supera la presión máxima Da alarma
          // SWITCH MODO DE OPERACION *****************************************************************************************         
          switch(operationMode){   
             // CONTROL POR PRESIÓN *********************************************************
             case PRS_CTRL: {  
                //if (Press_H2O >= Pset_H2O ){analogWrite(ANINSP,0);}   // Si alcanza el set de presión cierra la válvula inspiratoria
                //else {                                                // Si no se ha alcanzado Pset calcula PID
                  pressSet = Pset_H2O; pressIn = Press_H2O; pressPID.Compute();               
                  analogWrite(ANINSP,pressOut); 
                 //}                     // Actualiza la salida de la valvula
             break;}   
             // FIN DE CONTROL POR PRESION ****************************************************
 
             // CONTROL POR VOLUMEN ***********************************************************
             case VOL_CTRL: {   
                 if(inspVol_ml >= Vset_ml-10.0){analogWrite(ANINSP,0);}    // Si alcanza el set de volumen Cierra válv. inspiratoria 
                 else {                                               // Si no se ha alcanzado Vset Calcula PID
                    flowSet = peakFlow;  flowIn = inspFlow_lm;  flowPID.Compute();
                    analogWrite(ANINSP,flowOut);}                     // Actualiza la salida de la valvula
             break; }    
             // FIN DE CONTROL POR VOLUMEN  **************************************************
          }
        } else {  // ************************************* Inicia ESPIRACION ***********************************************************
          bandInsp = false;                                             // Señala al PC que entramos en espiración
          analogWrite(ANINSP,inspValue);                                // Cierra Valv Insp al valor dependiente del modo de trigger
          presIn = Press_H2O; peepSet = PEEP_H2O; servoPID.Compute();   // Calcula Posicion Valvula Servo con el PID
          angulo = (int) angOut;
          myservo.write(angulo);                                        // Actualiza el angulo del servo de la valv esp

          // ********************************** DETECCION DEL TRIGGER **********************************
          if(contPaso > DurIns+(float)DurEsp/3.0){                               // Sólo actua a partir de la tercera parte de la espiración
            if(abs(PEEP_H2O - Press_H2O) < 0.5){                 // Si la presión está proxima a la PEEP 
              switch(TriggerMode){                               // Actua dependiendo del modo de trigger
                case 0: {                                        // SIN TRIGGER
                    inspValue = 0;                               // Valvula Inspiratoria cerrada
                break; } 
                case 1: {                                        // TRIGGER POR PRESIÓN
                  inspValue = 0;                                 // Valvula Inspiratoria cerrada
                  if (Press_H2O + 0.5 <  PressLI_H2O)  contPaso = Periodo;
                break; } 
                case 2: {                                         // TRIGGER POR VOLUMEN  HACERLO POR DIF. ENTRE INSP Y ESP
                  inspValue = 150;                                // Valvula Inspiratoria ligeramente abierta ??????
                  if ( expFlow_lm + 2 < expFlowLI_lm) contPaso = Periodo;
                break; } 
              }                 // Fin de TriggerMode
            }                   // Fin de si la presión está próxima a la PEEP
          }                     // Fin de que solo acúa en la tercera fase de la inspiracion
        }                       // Fin de la ESPIRACION
        
        
        // Fin del Periodo **************************************************************
        if (contPaso >= Periodo){ 
          contPaso = 0;
          if (Press_H2O < PEEP_H2O-2.0) BIT_SET(Alarma, 2); //Alarma de peep más bajo de un 2cm del set
          //if ( (operationMode == VOL_CTRL) && (inspVol_ml < Vset_ml-10) ) BIT_SET(Alarma, 3); // Alarma de Vtidal menor de 10 ml del set
          if ( (inspVol_ml < Vset_ml-10) ) BIT_SET(Alarma, 3); // Alarma de Vtidal menor de 10 ml del set
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
      if (not(contcseg % 2)) {
        enviadatos();   
      }
      // ********************** ENVIA DATOS AL PC *********************************************
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
            Alarma = (char) 0;
            digitalWrite(22, LOW);
            break; }
       }
       StrS = "";
       SFin = false;
    }
  }
}
// *************************************** FIN LEE LA LINEA SERIE ***********************************************


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
        sprintf(sinfo, "%04d%01d%02d%03d%03d", (int)Tinsp, bandInsp, (unsigned int) Alarma, (int) ((PressLI_H2O)*10), (int) ( (expFlowLI_lm - 2)*10) ); 
        //Serial.print(f_sensFELI); Serial.print("  "); Serial.println(getFE(f_sensFELI));
        Sm.concat(sinfo);
        if(Sm.length() == 50){ Serial.println (Sm); }
        Sm = "";
}
 
