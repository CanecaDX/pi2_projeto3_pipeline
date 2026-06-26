#include <stdlib.h>
#include <stdio.h>
#include "registradores_pipeline.h"

BI_DI *bi_di_create(void){
	BI_DI *bi_di = calloc(1,sizeof(BI_DI));
	bi_di->pc = 0;
    bi_di->instrucao = 0;
	return bi_di;
}

DI_EX *di_ex_create(void){
	DI_EX *di_ex = calloc(1,sizeof(DI_EX));
	di_ex->pc = 0;
    di_ex->instrucao =0 ;
   //ula funct
	di_ex->opcode = 0;
	// endereco
	di_ex->rt =0;
    di_ex->rd = 0;
    di_ex->funct = 0;
    //sinais
    di_ex->RegDst = 0;
    di_ex->RegWrite = 0;
    di_ex->Memtoreg = 0;
    di_ex->ULASrc = 0;
    di_ex->MemRead = 0 ;
    di_ex->MemWrite = 0;
    di_ex->Branch = 0;
    di_ex->jump= 0 ;
    di_ex->ULA_op = 0;
    
   //saidas
	di_ex->address = 0;
    di_ex->rs_dado = 0;
    di_ex->rt_dado = 0;
    di_ex->imm_dado= 0; 
	return di_ex;
}

EX_MEM *ex_mem_create(void){
	EX_MEM *ex_mem = calloc(1,sizeof(EX_MEM));
	ex_mem->pc = 0;
    ex_mem->instrucao =0 ;
   //ula funct
	ex_mem->opcode = 0;
	// endereco
    ex_mem->rd = 0;
   //sinais
    ex_mem->RegWrite = 0;
    ex_mem->Memtoreg = 0;
    ex_mem->MemRead = 0 ;
    ex_mem->MemWrite = 0;
    ex_mem->Branch = 0;
    ex_mem->jump= 0 ;
   //saidas
   ex_mem->address = 0;
   ex_mem->branch_resultado =0;
   ex_mem->rt_dado = 0;
     ex_mem->ula_resultado = 0;
     ex_mem->Overflow = 0;
     ex_mem->zero= 0;
	return ex_mem;
}

MEM_WB *mem_wb_create(void){
	MEM_WB *mem_wb = calloc(1,sizeof(MEM_WB));
	mem_wb->instrucao =0 ;
	mem_wb->opcode = 0;
    mem_wb->rd = 0;
    // sinais
    mem_wb->RegWrite = 0;
    mem_wb->Memtoreg = 0;
	//  saidas ula
    mem_wb->ula_resultado = 0;
    //saidas
    mem_wb->saida_mem = 0;
	return mem_wb;
}

void set_bi_di(BI_DI *bi_di,uint8_t pc, uint16_t instrucao){
	bi_di->pc = pc;
    bi_di->instrucao = instrucao;
}

void set_di_ex(DI_EX *di_ex, uint8_t pc, uint16_t instrucao, uint8_t opcode,uint8_t rt, uint8_t rd, uint8_t funct, uint8_t RegDst, uint8_t RegWrite, uint8_t Memtoreg,  uint8_t ULASrc, uint8_t MemRead, uint8_t MemWrite, uint8_t Branch,  uint8_t jump,  uint8_t ULA_op,  int8_t rs_dado, int8_t rt_dado, int8_t imm_dado, uint8_t address, uint8_t rs){
	di_ex->pc = pc;
    di_ex->instrucao = instrucao ;
   //ula funct
	di_ex->opcode = opcode;
	// endereco
	
	di_ex->rt = rt;
    di_ex->rd = rd;
    di_ex->rs = rs;
    di_ex->funct = funct;
    //sinais
    di_ex->RegDst = RegDst;
    di_ex->RegWrite = RegWrite;
    di_ex->Memtoreg = Memtoreg;
    di_ex->ULASrc = ULASrc;
    di_ex->MemRead = MemRead ;
    di_ex->MemWrite = MemWrite;
    di_ex->Branch = Branch;
    di_ex->jump= jump ;
    di_ex->ULA_op = ULA_op;
    
   //saidas
	di_ex->address = address;
    di_ex->rs_dado = rs_dado;
    di_ex->rt_dado = rt_dado;
    di_ex->imm_dado= imm_dado;  
}

void set_ex_mem(EX_MEM *ex_mem, uint8_t pc, uint16_t instrucao, uint8_t opcode, uint8_t rd, uint8_t RegWrite, uint8_t Memtoreg, uint8_t MemRead, uint8_t MemWrite, uint8_t Branch,  uint8_t jump, int8_t rt_dado, int8_t ula_resultado, int8_t Overflow, int8_t zero, uint8_t branch_resultado,uint8_t address){
	ex_mem->pc = pc;
    ex_mem->instrucao =instrucao ;
   //ula funct
	ex_mem->opcode = opcode;
	// endereco
    ex_mem->rd = rd;
   //sinais
    ex_mem->RegWrite = RegWrite;
    ex_mem->Memtoreg = Memtoreg;
    ex_mem->MemRead = MemRead ;
    ex_mem->MemWrite = MemWrite;
    ex_mem->Branch = Branch;
    ex_mem->jump= jump ;
   //saidas
   ex_mem->address = address;
   ex_mem->branch_resultado = branch_resultado;
   ex_mem->rt_dado = rt_dado;
     ex_mem->ula_resultado = ula_resultado;
     ex_mem->Overflow = Overflow;
     ex_mem->zero= zero;
}

void set_mem_wb(MEM_WB *mem_wb, uint16_t instrucao, uint8_t opcode, uint8_t rd, uint8_t RegWrite, uint8_t Memtoreg, int8_t ula_resultado, int8_t saida_mem){
    mem_wb->instrucao =instrucao ;
	mem_wb->opcode = opcode;
    mem_wb->rd = rd;
    // sinais
    mem_wb->RegWrite = RegWrite;
    mem_wb->Memtoreg = Memtoreg;
	//  saidas ula
    mem_wb->ula_resultado = ula_resultado;
    //saidas
    mem_wb->saida_mem = saida_mem;
}

void copiaBI_DI(BI_DI *bi_di_backup, const BI_DI *bi_di){
 if (!bi_di_backup || !bi_di) return;
 *bi_di_backup = *bi_di;
}

void copiaDI_EX(DI_EX *di_ex_backup, const DI_EX *di_ex){
 if (!di_ex_backup || !di_ex) return;
 *di_ex_backup = *di_ex;
}

void copiaEX_MEM(EX_MEM *ex_mem_backup, const EX_MEM *ex_mem){
 if (!ex_mem_backup || !ex_mem) return;
 *ex_mem_backup = *ex_mem;
}

void copiaMEM_WB(MEM_WB *mem_wb_backup, const MEM_WB *mem_wb){
 if (!mem_wb_backup || !mem_wb) return;
 *mem_wb_backup = *mem_wb;
}
void clear_bi_di(BI_DI *bi_di) {
    if (!bi_di) return;
    bi_di->pc = 0;
    bi_di->instrucao = 0;
    bi_di->v = 0;
}

void clear_di_ex(DI_EX *di_ex) {
    if (!di_ex) return;
    di_ex->pc = 0;
    di_ex->instrucao = 0;
    di_ex->opcode = 0;
    di_ex->rs = 0;
    di_ex->rt = 0;
    di_ex->rd = 0;
    di_ex->funct = 0;
    
    di_ex->RegDst = 0;
    di_ex->RegWrite = 0;
    di_ex->Memtoreg = 0;
    di_ex->ULASrc = 0;
    di_ex->MemRead = 0;
    di_ex->MemWrite = 0;
    di_ex->Branch = 0;
    di_ex->jump = 0;
    di_ex->ULA_op = 0;
    
    di_ex->rs_dado = 0;
    di_ex->rt_dado = 0;
    di_ex->imm_dado = 0;
    di_ex->address = 0;
    di_ex->v = 0;
}

void clear_ex_mem(EX_MEM *ex_mem) {
    if (!ex_mem) return;
    ex_mem->pc = 0;
    ex_mem->instrucao = 0;
    ex_mem->opcode = 0;
    ex_mem->rd = 0;
    
    // Sinais de controle zerados
    ex_mem->RegWrite = 0;
    ex_mem->Memtoreg = 0;
    ex_mem->MemRead = 0;
    ex_mem->MemWrite = 0;
    ex_mem->Branch = 0;
    ex_mem->jump = 0;
    
    ex_mem->address = 0;
    ex_mem->branch_resultado = 0;
    ex_mem->ula_resultado = 0;
    ex_mem->Overflow = 0;
    ex_mem->zero = 0;
    ex_mem->rt_dado = 0;
    ex_mem->v = 0;
}

void clear_mem_wb(MEM_WB *mem_wb) {
    if (!mem_wb) return;
    mem_wb->instrucao = 0;
    mem_wb->opcode = 0;
    mem_wb->rd = 0;
    
    // Sinais de controle zerados
    mem_wb->RegWrite = 0;
    mem_wb->Memtoreg = 0;
    
    mem_wb->ula_resultado = 0;
    mem_wb->saida_mem = 0;
    mem_wb->v = 0;
}