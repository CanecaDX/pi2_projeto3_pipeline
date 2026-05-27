#ifndef EXTENSOR_H
#define EXTENSOR_H

#include <stdint.h>

int8_t extend(uint8_t imm6);
int16_t extend_data_to_mem(uint8_t data_to_mem);
int8_t reduce_mem_to_data(uint16_t data_mem);


#endif
