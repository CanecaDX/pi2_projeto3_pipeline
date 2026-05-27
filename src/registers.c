#include <stdlib.h>
#include <stdio.h>
#include "registers.h"

static void format_binary8(uint8_t value, char *buffer, size_t buffer_size){
    if (!buffer || buffer_size < 12) return;

    int pos = 0;
    for (int i = 7; i >= 0 && pos < (int)buffer_size - 1; i--) {
        buffer[pos++] = ((value >> i) & 1) ? '1' : '0';
        if (i % 4 == 0 && i != 0 && pos < (int)buffer_size - 1) {
            buffer[pos++] = ' ';
        }
    }
    buffer[pos] = '\0';
}

Banco_registradores *registers_create(){
    Banco_registradores *regs = calloc(1, sizeof(Banco_registradores));
    if (!regs) return NULL;

    regs->registradores = calloc((size_t)8, sizeof(uint8_t));
    if (!regs->registradores) {
        free(regs);
        return NULL;
    }

    return regs;
}

void print_regs(const Banco_registradores *regs){
    if (!regs || !regs->registradores) return;
    printf("\nBANCO DE REGISTRADORES\n");
    printf("+-------------+---------+------------+\n");
    printf("| Registrador | Decimal | Binário    |\n");
    printf("+-------------+---------+------------+\n");
    for (int i = 0; i < 8; i++){
        int8_t sval = (int8_t)regs->registradores[i];
        char bin_buf[12] = {0};

        format_binary8(regs->registradores[i], bin_buf, sizeof(bin_buf));
        printf("| $%-10d | %7d | %-10s |\n", i, sval, bin_buf);
    }
    printf("+-------------+---------+------------+\n");
}

Out_registers ex_registers(In_registers input, Banco_registradores *regs){
    Out_registers output = {0};
    output.val1 = (int8_t)(regs->registradores[input.reg_base1]);
    output.val2 = (int8_t)(regs->registradores[input.reg_base2]);

    //so vai escrever o dado se o sinal for 1
    if(input.write_reg == 1){
        regs->registradores[input.reg_destino] = (uint8_t)(input.dado_escrever);
    }
    return output;
}

void copiaBancoRegistradores(Banco_registradores* banco_backup, Banco_registradores* banco){
    int i;
    for(i = 0; i < 8; i++){
       banco_backup->registradores[i] =  banco->registradores[i]; 
    }
}

