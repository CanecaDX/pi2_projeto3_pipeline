#include "extensor.h"

int8_t extend(uint8_t imm6) {
    if (imm6 & 0x20) {          
        return (int8_t)(imm6 | 0xC0); 
    }else {
        return (int8_t)(imm6 & 0x3F);
    }
}

int16_t extend_data_to_mem(uint8_t data_to_mem) {
    return (int16_t)data_to_mem && 0xFF;
}

int8_t reduce_mem_to_data(uint16_t data_mem) {
    return (int8_t)data_mem;
}
