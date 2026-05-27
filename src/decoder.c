#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "decoder.h"
#include "extensor.h"

char *funct_to_string(Funct funct){
    switch (funct) {
        case FUNCT_ADD: return "add";
        case FUNCT_SUB: return "sub";
        case FUNCT_AND: return "and";
        case FUNCT_OR: return "or";
        case FUNCT_UNKNOWN: return "Valor de function não reconhecido";
    }
}

Decoded decode(uint16_t instruction){
    Decoded decode_instruction;
    //retorna todos os campos preenchidos
    decode_instruction.opcode = (instruction >> 12) & 0xF;    
    decode_instruction.rs     = (instruction >> 9) & 0x7;
    decode_instruction.rt     = (instruction >> 6) & 0x7;
    decode_instruction.rd     = (instruction >> 3) & 0x7;
    decode_instruction.funct  = instruction & 0x7;

    switch (decode_instruction.funct) {
        case 0x0: decode_instruction.funct_op = FUNCT_ADD; break;
        case 0x2: decode_instruction.funct_op = FUNCT_SUB; break;
        case 0x4: decode_instruction.funct_op = FUNCT_AND; break;
        case 0x5: decode_instruction.funct_op = FUNCT_OR; break;
        default: decode_instruction.funct_op = FUNCT_UNKNOWN; break;
    }

    uint8_t imm6 = instruction & 0x3F;
    decode_instruction.imm = extend(imm6);

    if(decode_instruction.opcode == 0){
        decode_instruction.type = TYPE_R;
    } else if(decode_instruction.opcode == 0x2){
        decode_instruction.type = TYPE_J;
    } else if(decode_instruction.opcode == 0x4 || decode_instruction.opcode == 0xB || decode_instruction.opcode == 0xF || decode_instruction.opcode == 0x8){ 
        decode_instruction.type = TYPE_I;
    }

    return decode_instruction;
}
