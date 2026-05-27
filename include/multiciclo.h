#ifndef MULTICICLO_H
#define MULTICICLO_H
#include <stdint.h>
#include "pc.h"
#include "registers.h"
#include "memory.h"
#include "controle.h"
#include "decoder.h"
#include "ULA.h"

typedef struct Stats{
 int r, im, j;
 int arit, desC, mem_d;
 int contInsEx;
 int contCiclos;
 int cpi;
} Stats;

typedef struct Multiciclo {
    ProgramCounter *pc;
    Banco_registradores *regs_bank;
    Memory *memory;
    Controle *controle;
    Decoded decoded_inst;
    ULA *ula;
    uint16_t RI; //registrador de instrução
    int8_t A, B, RDM; //registradores auxiliares, registrador de dados da memoria
    uint8_t saidaULA; //registrador para o resultado da ULA
    Stats *stats;
} Multiciclo;

typedef struct No {
	Multiciclo* multiciclo;
	struct No *proximo;
} No;

Stats *stats_create(void);
Multiciclo *multiciclo_create(void);
void run(Multiciclo *m);
void run_step(Multiciclo *m);
void reset_run(Multiciclo *m);
void reset_all(Multiciclo *m);
void print_temp_regs(Multiciclo *m);
void print_ex_info(Multiciclo *m, int pc, int state, int n_state);
void copiaSimulador (Multiciclo* m_backup, Multiciclo* m);
void empilhar(No **topo, Multiciclo* multiciclo);
void desempilhar(No **topo, Multiciclo* multiciclo); 
void copiaStats(Stats *stats_b, Stats * stats);
void copiaSimulador (Multiciclo* m_backup, Multiciclo* m);
void empilhar(No **topo, Multiciclo* multiciclo);
void desempilhar(No **topo, Multiciclo* multiciclo);
void programStat(Memory *mem, int cond, Stats *stats, Multiciclo *m);
void print_ex_status(Multiciclo *m);

#endif
