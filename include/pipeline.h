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
 int beqT, beqNT;
 int nops;
} Stats;

typedef struct Forward{
    int A;
    int B;
    int valA;
    int valB; //valores reais para os registradores
} Forward;

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
    Forward *f;

    //busca
    Instrucao instrucao_buscada;

    //decodificao
    Instrucao instrucao_decodificao;

    //execucao
    Instrucao instrucao_executada;
    int8_t imm_dado_exec;  
    uint8_t address_exec;

    //memoria
    Instrucao instrucao_memoria;
    uint8_t Branch_mem;
    int8_t zero_mem;
    uint8_t address_mem;
    uint8_t branch_resultado_mem;
    int8_t ula_resultado_mem;
    int8_t rt_dado_mem;

    //wb
    uint8_t RegWrite_wb;
    uint8_t rd_wb;
    Instrucao instrucao_wb;

} Pipeline;

typedef struct No {
	Pipeline* pipeline;
	struct No *proximo;
} No;

typedef struct {
    No* topo;
    int tamanho;
} Pilha;

Stats *stats_create(void);
Pipeline *pipeline_create(void);
void buscar(Pipeline *p, WINDOW * exec, WINDOW *log);
void decodificar(Pipeline *p, WINDOW * exec);
void executar(Pipeline *p, WINDOW * exec);
void acesso_memoria(Pipeline *p, WINDOW * exec);
void write_back(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log);
void run(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log);
void run_step(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log);
void copiaSimulador (Pipeline *p_backup, Pipeline* p);
void programHead(int cond, Pipeline *p, Memoria_instrucao *mem, WINDOW * regw);
void reset_all(Pipeline *p);
void reset_run(Pipeline *p);
void copiaStats(Stats *stats_backup, Stats *stats);
Pilha* criarPilha();
void empilhar(Pilha * p, Pipeline* pipeline);
void desempilhar(Pilha * p, Pipeline* pipeline);
void reset_stats(Pipeline *p);
int pipeline_terminou(Pipeline *p);
void limparPilha(Pilha *p);
void forward_unit(Pipeline *p);
void mostra_estagios(Pipeline *p, WINDOW * exec);
#endif
