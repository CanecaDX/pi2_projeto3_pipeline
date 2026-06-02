#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "decoder.h"
#include "pipeline.h"

    
int main(){
    int op;
    Pipeline *p = pipeline_create();
    Pipeline  *p_backup = pipeline_create();

    if (!p) {
        printf("\nFalha ao alocar monociclo.");
        return 1;
    }
    //teste de representação dos registradores
    //m->regs_bank->registradores[1] = -5;
    //m->regs_bank->registradores[2] = 10;

    do{
        printf("\n------------------------------------");
        printf("\n0_ Fechar programa");
        printf("\n1_ Carregar arquivo de memória de dados (.dat)");
        printf("\n2_ Carregar arquivo de memória de instruções (.mem)");
        printf("\n3_ Mostrar dados na memória de dados");
        printf("\n4_ Mostrar dados na memória de instruções");
        printf("\n5_ Mostrar dados no banco de registradores");
        printf("\n6_ Mostrar instruções no formato Assembly");
		printf("\n7_ Mostrar todo o simulador");
        printf("\n8_ Salvar arquivo Assembly (.asm)");
        printf("\n9_ Salvar dados da memória da dados (.dat)");
        printf("\n10_ Rodar programa");
        printf("\n11_ Rodar 1 instrução");
        printf("\n12_ Voltar 1 instrução");
        printf("\n13_ Exibir estatísticas");
        printf("\n------------------------------------");
        printf("\nInforme a opção desejada: ");
        scanf("%d", &op);

        switch(op){
            case 0: printf("\nEncerrando simulador.."); break;
            case 1: {
                char data_name[128];
                printf("\nInforme o arquivo .dat: ");
                scanf("%127s", data_name);
                data_memory_load(p->mem_data, data_name);
                break;
            }
            case 2: {
                char mem_name[128];
                printf("\nInforme o arquivo .mem: ");
                scanf("%127s", mem_name);
                Memoria_instrucao *new_mem = instruction_memory_load_file(mem_name);
                if (new_mem) {
                    if (p->mem_inst) {
                        free(p->mem_inst->instrucao);
                        free(p->mem_inst);
                    }
                    p->mem_inst = new_mem;
                    p->has_executed = 0;
                    p->just_rewound = 0;
                }
                break;
            }
            case 3: data_memory_print(p->mem_data); break;
            case 4: print_instruction_memory(p->mem_inst); break;
            case 5: print_regs(p->regs_bank); break;
            case 6: {
                if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para exibir o Assembly, carregue instruções primeiro.\n");
                    break;
                }
                exibeTodos_asm(p->mem_inst);
                break;
            }
            case 7:	print_instruction_memory(p->mem_inst); printf("\n"); data_memory_print(p->mem_data); print_regs(p->regs_bank); printf("\n"); exibeTodos_asm(p->mem_inst); printf("\n"); printf("\n\nPC ESTÁ EM : %d\n", p->pc->pc_index); break;
            case 8: {
                if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para gerar o arquivo Assembly, carregue instruções primeiro.\n");
                    break;
                }
                mem_to_asm(p->mem_inst);
                break;
            }
            case 9: data_memory_save(p->mem_data, "output_dados.dat"); break;
            case 10: {
                run(p);
                p->just_rewound = 1;
                break;
            }
            case 11: {
                 /*
                 copiaSimulador(p_backup, p);
                 if(p->pc->pc_index == p->mem_inst->loaded_count){
						p->pc->pc_index = 0;
				 }
				 
				 */
				run_step(p);
            break;
            }
            case 12: {
                if (!p->has_executed) {
                    printf("Para voltar uma instrução, ao menos uma deve ter sido executada.\n");
                    break;
                }
                if (p->just_rewound) {
                    printf("Não é possível voltar duas instruções seguidas.\n");
                    break;
                }
                copiaSimulador(p,p_backup);
                p->just_rewound = 1;
                printf("VOLTOU 1 INSTRUCAO ! \n");
                printf("PC ESTA EM : %d \n", p->pc->pc_index);
                break;
            }
            case 13: 
				if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para exibir as estatísticas, carregue instruções primeiro.\n");
                    break;
                }
				exibeEst(p->mem_inst);
            break;
            default: printf("\nOpção inválida!"); break;
        }
    }while(op != 0);

    return 0;
}
