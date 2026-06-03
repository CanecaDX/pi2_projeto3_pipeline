#ifndef PIPELINE_H
#define PIPELINE_H

#include "pc.h"
#include "registers.h"
#include "instruction_mem.h"
#include "data_mem.h"
#include "controle.h"
#include "decoder.h"
#include "ULA.h"
#include "registradores_pipeline.h"

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
} Pipeline;

Pipeline *pipeline_create(void);
void buscar(Pipeline *p);
void decodificar(Pipeline *p);
void executar(Pipeline *p);
void acesso_memoria(Pipeline *p);
int run(Pipeline *p);
int run_step(Pipeline *p);
int run_back(Pipeline *p); 
void copiaSimulador (Pipeline *p_backup, Pipeline* p);

#endif
