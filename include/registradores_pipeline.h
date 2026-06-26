#ifndef REGISTRADORESP_H
#define REGISTRADORESP_H

#include <stdint.h>


typedef struct{
	uint8_t  pc;
    uint16_t instrucao;
    int v;
} BI_DI;

typedef struct{
    int v;
	uint8_t  pc;
    uint16_t instrucao;
   //ula funct
	uint8_t opcode;
	// endereco
	
    uint8_t rt;
    uint8_t rd;
    uint8_t rs;
    uint8_t funct;
    //sinais
    uint8_t RegDst;
    uint8_t RegWrite;
    uint8_t Memtoreg;
    uint8_t ULASrc;
    uint8_t MemRead;
    uint8_t MemWrite;
    uint8_t Branch;
    uint8_t jump;
    uint8_t ULA_op;
    
   //saidas
    int8_t rs_dado;
    int8_t rt_dado;
    int8_t imm_dado;  
    uint8_t address;
} DI_EX;

typedef struct{
    int v;
	uint8_t  pc;
	//instrucao
    uint16_t instrucao;
	uint8_t opcode;
    uint8_t rd;
    // sinais
    uint8_t RegWrite;
    uint8_t Memtoreg;
    uint8_t MemRead;
    uint8_t MemWrite;
    uint8_t Branch;
    uint8_t jump;
    
    uint8_t address;
    uint8_t branch_resultado;
    //  saidas ula
    int8_t ula_resultado;
    int8_t Overflow;
    int8_t zero;
    
    //saidas
    int8_t rt_dado;
    

} EX_MEM;

typedef struct{
    int v;
	//instrucao
    uint16_t instrucao;
	uint8_t opcode;
    uint8_t rd;
    // uint8_t funct; pode ser ula op 
    // sinais
    uint8_t RegWrite;
    uint8_t Memtoreg;  
	//  saidas ula
    int8_t ula_resultado;
    //saidas
    int8_t saida_mem;
    
} MEM_WB;

BI_DI *bi_di_create(void);
DI_EX *di_ex_create(void);
EX_MEM *ex_mem_create(void);
MEM_WB *mem_wb_create(void);

void set_bi_di(BI_DI *bi_di,uint8_t pc, uint16_t instrucao);
void set_di_ex(DI_EX *di_ex, uint8_t pc, uint16_t instrucao, uint8_t opcode, uint8_t rt, uint8_t rd, uint8_t funct, uint8_t RegDst, uint8_t RegWrite, uint8_t Memtoreg,  uint8_t ULASrc, uint8_t MemRead, uint8_t MemWrite, uint8_t Branch,  uint8_t jump,  uint8_t ULA_op,  int8_t rs_dado, int8_t rt_dado, int8_t imm_dado, uint8_t address, uint8_t rs);
void set_ex_mem(EX_MEM *ex_mem, uint8_t pc, uint16_t instrucao, uint8_t opcode, uint8_t rd, uint8_t RegWrite, uint8_t Memtoreg, uint8_t MemRead, uint8_t MemWrite, uint8_t Branch,  uint8_t jump, int8_t rt_dado, int8_t ula_resultado, int8_t Overflow, int8_t zero, uint8_t branch_resultado, uint8_t address);
void set_mem_wb(MEM_WB *mem_wb, uint16_t instrucao, uint8_t opcode, uint8_t rd, uint8_t RegWrite, uint8_t Memtoreg, int8_t ula_resultado, int8_t saida_mem);
void copiaBI_DI(BI_DI *bi_di_backup, const BI_DI *bi_di);
void copiaDI_EX(DI_EX *di_ex_backup, const DI_EX *di_ex);
void copiaEX_MEM(EX_MEM *ex_mem_backup, const EX_MEM *ex_mem);
void copiaMEM_WB(MEM_WB *mem_wb_backup, const MEM_WB *mem_wb);
void clear_mem_wb(MEM_WB *mem_wb);
void clear_ex_mem(EX_MEM *ex_mem);
void clear_di_ex(DI_EX *di_ex);
void clear_bi_di(BI_DI *bi_di);

 #endif
