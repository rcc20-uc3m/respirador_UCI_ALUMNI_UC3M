//
// Created by diego on 12/2/21.
//

#ifndef T2_ESPIRACION_H
#define T2_ESPIRACION_H

void init_espiracion(void);
void reset_espiracion(void);
void espiracion_ciclo(void);
void espiracion_control(void);
void control_rc_normal(void);
void control_rc_bajo(void);
void control_rc_alto(void);
int16_t get_older_press();
void actualizo_peep_rc_normal();
void actualizo_peep_rc_alto();
void actualizar_peep();

enum class ESTADOESPIRACION {Espera, Estable};
enum class TIPOESPIRACION {RCBAJO, RCNORMAL, RCALTO};
#endif //T2_ESPIRACION_H
