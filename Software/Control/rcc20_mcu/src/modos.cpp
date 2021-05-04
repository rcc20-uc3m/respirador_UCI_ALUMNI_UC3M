//
// Created by diego on 13/2/21.
//

#include "modos.h"

struct ModoVentilador misModos[(int)MODOS::Count];

void init_modos(){
    misModos[(int) MODOS::PC_CMV] = {
            {CICLOS::Tiempo,
             ESPERAS::TiempoEstable,
             DISPAROS::Tiempo,
             CONTROLES::Presion},
            {CICLOS::Tiempo,
             ESPERAS::TiempoEstable,
             DISPAROS::Tiempo,
             CONTROLES::Presion}};
    misModos[(int) MODOS::PC_AC] = {
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::UnaVentana,
                    CONTROLES::Presion},
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::UnaVentana,
                    CONTROLES::Presion}};
    misModos[(int) MODOS::PC_SIMV] = {
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::DosVentanas,
                    CONTROLES::Presion},
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::DosVentanas,
                    CONTROLES::PresionSoporte}};
    misModos[(int) MODOS::VC_CMV] = {
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::Tiempo,
                    CONTROLES::Volumen},
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::Tiempo,
                    CONTROLES::Volumen}};
    misModos[(int) MODOS::VC_AC] = {
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::UnaVentana,
                    CONTROLES::Volumen},
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::UnaVentana,
                    CONTROLES::Presion}};
    misModos[(int) MODOS::VC_SIMV] = {
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::DosVentanas,
                    CONTROLES::Volumen},
            {CICLOS::Tiempo,
                    ESPERAS::TiempoEstable,
                    DISPAROS::DosVentanas,
                    CONTROLES::PresionSoporte}};
}