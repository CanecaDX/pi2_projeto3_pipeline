#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#define MEM_SIZE 256

typedef struct {
    uint16_t value;
} Data;

typedef struct {
	uint8_t write_mem;
	uint8_t read_mem;
	int adress;
	int16_t dado_escrever;
}In_data_mem;

typedef struct{
	int16_t dado_lido;
}Out_data_mem;

typedef struct {
    Data *data;
	int inst_loaded_count;
	int data_loaded_count;
	In_data_mem in_data;
	Out_data_mem out_data;
} Memory;

void get_mem_file(const char *mem_name, Data *data, int* inst_countp, int* data_countp);
Memory *memory_create(void);
Out_data_mem ex_data_mem(In_data_mem input, Memory *mem);
void exibeEst(const Memory *mem);
void print_memory(const Memory *mem);
void print_binary(uint16_t value);
void mem_to_asm(Memory *mem);
int8_t convert(uint16_t value);
void print_asm(Decoded d);
void exibe1_asm(uint16_t value);
void exibeTodos_asm(Memory *mem);
void output(Memory *mem);
void format_binary16(uint16_t value, char *buffer, size_t buffer_size);
void binary_opcode(uint8_t value, char *buff, size_t buff_sz);
void fprintf_binary(uint16_t value, FILE *f);
void copiaMemoria(Memory* memoria_backup, Memory* memoria);

#endif
