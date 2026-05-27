#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/memory.h"

#define BIN_SIZE 16


Memory *memory_create(void){
    Memory *mem = calloc(1, sizeof(Memory));
    if (!mem) return NULL;

    mem->data = calloc(MEM_SIZE, sizeof(Data));
    if (mem->data == NULL) {
        perror("Erro ao alocar memória");
        free(mem);
        return NULL;
    }
	
    mem->inst_loaded_count = 0;
    mem->data_loaded_count = 0;
    return mem;
}

void get_mem_file(const char *mem_name, Data *data, int* inst_countp, int* data_countp) {
    FILE *arquivo;
    char line[BIN_SIZE+1];
    int processandoDados = 0;
    int i,j;
    i = 0;
    j = 128;
    int inst_count = 0;
    int data_count = 0;
    

    // Abre o arquivo para leitura
    arquivo = fopen(mem_name, "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }



    while (fgets(line, sizeof(line), arquivo)) {
        // Remove a quebra de linha (\n) se existir
        line[strcspn(line, "\n")] = 0;

        // Verifica se encontramos o marcador de transição
        if (strcmp(line, ".data") == 0) {
            processandoDados = 1;
         //   printf("\n[Marcador .data encontrado. Trocando para modo de DADOS]\n\n");
            continue; // Pula para a próxima iteração
        }
		if (line[0] == '\0') continue; // Ignora linhas vazias
        if (!processandoDados) {
             data[i].value = (uint16_t)strtol(line, NULL, 2);
            inst_count++; 
            i++;
          //  printf("Instrucao lida: %s\n", line);
            if(inst_count > 128){
				break;
			printf("acabou o espaço de memória para instruções");
			}
        } else {
            // Lógica para tratar os Dados
            // printf("Dado lido:      %s\n", line);
            data[j].value = (uint16_t)strtol(line, NULL, 2);
            data_count++;
            j++;
            if(data_count > 255){
				break;
			printf("acabou o espaço de memória para dados");
			}
			
        }
       
    }

  
  
    fclose(arquivo);

printf("\nMemória carregada com sucesso!");

*inst_countp = inst_count;
*data_countp = data_count;

if(inst_count == 0 && data_count == 0){
    printf("\nO arquivo de memória está vazio ou não foi lido corretamente. Verifique o conteúdo do arquivo e tente novamente.");

}

	
    return;
}

Out_data_mem ex_data_mem(In_data_mem input, Memory *mem){
	Out_data_mem output = {0};
	if (input.adress < 0 || input.adress >= MEM_SIZE) {
		printf("\nNão é acessado valor na memória, resultado fora do limite de endereçamento");
		return output;
	}
	if (input.read_mem) {
		output.dado_lido = mem->data[input.adress].value;
	}
	if (input.write_mem) {
		mem->data[input.adress].value = input.dado_escrever;
	}
	return output;
}

void exibeEst(const Memory *mem){
    if (!mem) return;
    printf("\nINSTRUCOES CARREGADAS: %d\n", mem->inst_loaded_count);
    printf("\nDADOS CARREGADOS: %d\n", mem->data_loaded_count);
}

void print_binary(uint16_t value){
    for(int i = 15; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i % 4 == 0 && i != 0) printf(" ");
    }
}

void format_binary16(uint16_t value, char *buffer, size_t buffer_size){
    int pos = 0;
    for (int i = 15; i >= 0 && pos < (int)buffer_size - 1; i--) {
        buffer[pos++] = ((value >> i) & 1) ? '1' : '0';
        if (i % 4 == 0 && i != 0 && pos < (int)buffer_size - 1) {
            buffer[pos++] = ' ';
        }
    }
    buffer[pos] = '\0';
}

void binary_opcode(uint8_t value, char *buff, size_t buff_sz){
    if (!buff || buff_sz == 0) return;
    int pos = 0;
    for (int i = 3; i >= 0 && pos < (int)buff_sz - 1; i--) {
        buff[pos++] = ((value >> i) & 1) ? '1' : '0';
    }
    buff[pos] = '\0';
}

void print_memory(const Memory *mem){
    printf("\nMEMÓRIA\n");
    printf("+----------+---------------------+-------------------+\n");
    printf("| ID       | BINÁRIO             | ASSEMBLY/DADO     |\n");
    printf("+----------+---------------------+-------------------+\n");

    for (int i = 0; i < MEM_SIZE; i++){
        char buff[20] = {0};
        format_binary16(mem->data[i].value, buff, sizeof(buff));

        printf("| %8d | %-19s | ", i, buff);
        if (i < 128) {
            if (i < mem->inst_loaded_count) {
                print_asm(decode(mem->data[i].value));
            } else {
                printf("-");
            }
        } else {
            printf("%d", convert(mem->data[i].value));
        }
        printf("\n");
    }

    printf("+----------+---------------------+-------------------+\n");
}

int8_t convert(uint16_t value){
	int8_t converted;
	converted = (int16_t)value & 0xFF;
	return converted;
}

void print_asm(Decoded d){
        if (d.type == TYPE_R) {
            switch (d.funct_op) {
                case FUNCT_ADD: // add
                    printf("add $%u, $%u, $%u\n", d.rd, d.rs, d.rt);
                    break;
                case FUNCT_SUB: // sub
                    printf("sub $%u, $%u, $%u\n", d.rd, d.rs, d.rt);
                    break;
                case FUNCT_AND: // and
                    printf("and $%u, $%u, $%u\n", d.rd, d.rs, d.rt);
                    break;
                case FUNCT_OR: // or
                    printf("or $%u, $%u, $%u\n", d.rd, d.rs, d.rt);
                    break;
            }
        } else if (d.type == TYPE_I) {
            switch (d.opcode) {
                case 0x4: // addi
                    printf("addi $%u, $%u, %d\n", d.rt, d.rs, d.imm);
                    break;
                case 0xB: // lw
                    printf("lw $%u, %d($%u)\n", d.rt, d.imm, d.rs);
                    break;
                case 0xF: // sw
                    printf("sw $%u, %d($%u)\n", d.rt, d.imm, d.rs);
                    break;
                case 0x8: // beq
                    printf("beq $%u, $%u, %d\n", d.rt, d.rs, d.imm);
                    break;
            }
        } else if (d.type == TYPE_J) {
            switch(d.opcode){
                case 0x2: // j
                    printf("j %u\n", (uint8_t)d.imm & 0x3F);
                    break;
            }
        }
    }	

void exibe1_asm(uint16_t value){
	
        Decoded d = decode(value);
		print_asm(d);
			if (d.type == TYPE_R) {
                switch (d.funct_op) {
                    case FUNCT_ADD: // add
						printf("Soma o conteudo do registrador $%u com o conteudo de $%u e armazena em $%u", d.rs, d.rt, d.rd);
						break;
                    case FUNCT_SUB: // sub
						printf("Subtrai o conteudo do registrador $%u com o conteudo de $%u e armazena em $%u", d.rs, d.rt, d.rd);
						break;
                    case FUNCT_AND: // and
						printf("Faz um AND com o conteudo do registrador $%u com o conteudo de $%u e armazena em $%u", d.rs, d.rt, d.rd);
						break;
                    case FUNCT_OR: // or
						printf("Faz um OR com o conteudo do registrador $%u com o conteudo de $%u e armazena em $%u", d.rs, d.rt, d.rd);
						break;
				}
			}else if(d.type == TYPE_I){
				switch (d.opcode){
				case 0x4: // addi
                    printf("Soma o valor imediato %d com o conteudo do registrador $%u e armazena em $%u", d.imm, d.rs, d.rt);
                    break;
                case 0xB: // lw
					printf("Carrega o conteudo armazenado na posicao %d da memoria para o registrador $%u", d.imm, d.rt);
                    break;
                case 0xF: // sw
                    printf("Armazena o conteudo do registrador $%u na posicao %d da memoria", d.rt, d.imm);
                    break;
                case 0x8: // beq
                    printf("Se os valores dos registradores $%u e $%u forem iguais, realiza um salto para a posicao informada.", d.rt, d.rs);
                break;
				}	
            }else if(d.type == TYPE_J){
               switch (d.opcode){
                case 0x2: // j
                     printf("Realiza um salto para posicao %u", (uint8_t)d.imm & 0x3F);
                    break;
               }
            }
		printf("\n");
}

void exibeTodos_asm(Memory *mem){
    if(!mem){
        return;
    }
    int count = mem ? mem->inst_loaded_count : 0;

	if(!count){
		printf("Não há instruções carregadas na memória ainda. Para visualizá-las, carregue um arquivo .mem primeiro.");
		return;
	}
	printf("\nASSEMBLY\n");
	for (int i = 0; i < count; i++) {
        Data raw = mem->data[i];
        Decoded d = decode(raw.value);
        print_asm(d);
      }
}

void mem_to_asm(Memory *mem){
    int count = mem ? mem->inst_loaded_count : 0;
    
    if(!count){
        printf("Não há instruções carregadas na memória ainda. Para visualizá-las, carregue um arquivo .mem primeiro.");
        return;
    }
    
    FILE *arquivo = fopen("output.asm", "w");
    if (arquivo == NULL) {
        perror("Erro ao criar o arquivo");
        return;
    }

    for (int i = 0; i < count; i++) {
        Data raw = mem->data[i];
        Decoded d = decode(raw.value);
        fprintf(arquivo, "%s", d.type == TYPE_R ? funct_to_string(d.funct_op) :
                d.type == TYPE_I ? (d.opcode == 0x4 ? "addi" : d.opcode == 0xB ? "lw" : d.opcode == 0xF ? "sw" : d.opcode == 0x8 ? "beq" : "unknown") :
                d.type == TYPE_J ? (d.opcode == 0x2 ? "j" : "unknown") : "unknown");
        if (d.type == TYPE_R) {
            fprintf(arquivo, " $%u, $%u, $%u\n", d.rd, d.rs, d.rt);
        } else if (d.type == TYPE_I) {
            fprintf(arquivo, " $%u, $%u, %d\n", d.rt, d.rs, d.imm);
        } else if (d.type == TYPE_J) {
            fprintf(arquivo, " %u\n", (uint8_t)d.imm & 0x3F);
        }
    }

    fclose(arquivo);
    printf("\nArquivo output.asm criado com sucesso!\n");
}
void fprintf_binary(uint16_t value, FILE *f){
    for(int i = 15; i >= 0; i--) {
        fprintf(f, "%d", (value >> i) & 1);
    }
}
void output(Memory *mem){
    if (mem->inst_loaded_count == 0) {
        printf("Não há instruções carregadas na memória. Para gerar um arquivo de memória (.mem) carregue a memória primeiro.\n");
        return;
    }

    FILE *f = fopen("output.mem", "w");

    if (!f) {
        printf("Erro ao criar o arquivo");
        return;
    }

    if(mem->inst_loaded_count != 0){
    for (int i = 0; i < mem->inst_loaded_count; i++) {
        fprintf_binary(mem->data[i].value, f);
        fprintf(f, "\n");
        }
    }
    if (mem->data_loaded_count != 0){
        fprintf(f, ".data\n");
        for (int i = 0; i < mem->data_loaded_count; i++) {
            fprintf_binary(mem->data[128 + i].value, f);
            fprintf(f, "\n");
        }
    }

    fclose(f);
    printf("\nArquivo output.mem criado com sucesso!\n");
}

void copiaMemoria(Memory* memoria_backup, Memory* memoria){
    int i;
    for(i = 128; i < MEM_SIZE; i++){
		memoria_backup->data[i].value = memoria->data[i].value;
        }
    }