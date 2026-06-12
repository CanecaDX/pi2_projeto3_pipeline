#ifndef PIPELINE_H
#define PIPELINE_H

#include <ncurses.h>

#include "pc.h"
#include "registers.h"
#include "instruction_mem.h"
#include "data_mem.h"
#include "controle.h"
#include "decoder.h"
#include "ULA.h"
#include "registradores_pipeline.h"

typedef struct Stats{
 int r, im, j;
 int arit, desC, mem_d;
 int contInsEx;
 int contCiclos;
 float cpi;
} Stats;

typedef struct {
    ProgramCounter pc;
    Banco_registradores *regs_bank;
    Memoria_dado *mem_data;
    Memoria_instrucao *mem_inst;
    BI_DI bi_di;
    DI_EX di_ex;
    EX_MEM ex_mem;
    MEM_WB mem_wb;
    Controle controle;
    Decoded decoded_inst;
    ULA ula;
    int has_executed;
    int just_rewound;
    Stats *stats;
} Pipeline;

Stats *stats_create(void);
Pipeline *pipeline_create(void);
void buscar(Pipeline *p, WINDOW * exec);
void decodificar(Pipeline *p, WINDOW * exec);
void executar(Pipeline *p, WINDOW * exec);
void acesso_memoria(Pipeline *p, WINDOW * exec);
void write_back(Pipeline *p, WINDOW * exec, WINDOW * regw);
void run(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log);
void run_step(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log);
void copiaSimulador (Pipeline *p_backup, Pipeline* p);
void programHead(int cond, Pipeline *p, Memoria_instrucao *mem, WINDOW * regw);
void reset_all(Pipeline *p);
void reset_run(Pipeline *p);

#endif
