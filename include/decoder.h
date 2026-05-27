#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

typedef enum { TYPE_R = 0, TYPE_I, TYPE_J } Tipo;

typedef enum {
    FUNCT_ADD = 0,
    FUNCT_SUB = 2,
    FUNCT_AND = 4,
    FUNCT_OR = 5,
    FUNCT_UNKNOWN 
} Funct;

typedef struct {
    uint8_t opcode;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint8_t funct;
    Funct funct_op;
    int8_t imm;   
    Tipo type;
} Decoded;

Decoded decode(uint16_t instruction);
char *funct_to_string(Funct funct);

#endif
