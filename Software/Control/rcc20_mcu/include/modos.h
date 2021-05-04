//
// Created by diego on 12/2/21.
//

#ifndef T2_MODOS_H
#define T2_MODOS_H

enum class CICLOS  {Tiempo, Flujo, Count};
enum class ESPERAS {Tiempo, TiempoEstable, Count};
enum class DISPAROS {Tiempo, UnaVentana, DosVentanas, Count};
enum class CONTROLES {Presion, Volumen, PresionSoporte, Count};

enum class MODOS {PC_CMV, PC_AC, VC_CMV, VC_AC, PC_SIMV, VC_SIMV, Count};
enum class TRIGGER {None, Presion, Flujo, Count};



struct RESPIRACION {
    enum CICLOS ciclo;
    enum ESPERAS espera;
    enum DISPAROS disparo;
    enum CONTROLES control;
};

struct ModoVentilador {
    struct RESPIRACION Mandatoria;
    struct RESPIRACION Soporte;
};

void init_modos();

#endif //T2_MODOS_H
