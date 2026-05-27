#ifndef CONTROLE_H
#define CONTROLE_H

#include <stdint.h>
#include "decoder.h"

typedef struct Multiciclo Multiciclo;

typedef struct {
    uint8_t opcode;
    Funct funct;
} Entrada_controle;

typedef struct{
    uint8_t RegDst;
    uint8_t RegWrite;
    uint8_t Memtoreg;
    uint8_t ULASrc;
    uint8_t MemRead;
    uint8_t MemWrite;
    uint8_t Branch;
    uint8_t jump;
    uint8_t ULA_op;
    uint8_t IResc;
    uint8_t PCesc;
    uint8_t PCsrc;
    uint8_t ULAFonteA;	
    uint8_t ULAFonteB;
    uint8_t IouD;
} Sinais;

typedef struct {
    int ac_state;
    int ant_state;
    int next_state;
    int ciclos;
} FSM;

typedef struct {
    Entrada_controle entrada;
    Sinais sinais;
    FSM *states;
} Controle;

void controle_sinais(Multiciclo *m);
void copiaEntradaControle(Entrada_controle input_backup, Entrada_controle input);
void copiaSinaisControle(Sinais sinais_backup, Sinais sinais);
void copiaStatesControle(FSM* states_backup , FSM* statess);
void copiaControle(Controle* controle_backup, Controle* controle);
#endif
