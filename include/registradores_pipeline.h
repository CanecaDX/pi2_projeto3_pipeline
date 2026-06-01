#ifndef REGISTRADORESP_H
#define REGISTRADORESP_H

#include <stdint.h>


typedef struct{
	uint8_t  pc;
    uint16_t instrucao;
} BI_DI;

typedef struct{
	uint8_t  pc;
	
    uint16_t instrucao;
   
   //ula funct
	uint8_t opcode;
	
	// enderecos
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint8_t funct
    int8_t imm;   
    
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
} DI_EX;

typedef struct{
	uint8_t  pc;
	//instrucao
    uint16_t instrucao;
   
	uint8_t opcode;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    int8_t imm; 
    // sinais
    uint8_t RegWrite;
    uint8_t Memtoreg;
    uint8_t MemRead;
    uint8_t MemWrite;
    uint8_t Branch;
    uint8_t jump;
    
    //  saidas ula
    int8_t ula_resultado;
    int8_t Overflow;
    int8_t zero;
    
    //saidas
    int8_t rt_dado;
    
} EX_MEM;

typedef struct{
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


 
