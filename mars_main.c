#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "multiciclo.h"

    
int main(){
    int op;
    Multiciclo *m = multiciclo_create();
    No *pilha = NULL;
    
    if (!m) {
        printf("\nFalha ao alocar multiciclo.");
        return 1;
    }
    //teste de representação dos registradores
    // m->regs_bank->registradores[7] = 128;
    //m->regs_bank->registradores[2] = 10;

    do{
        printf("\n------------------------------------");
        printf("\n0_ Fechar programa");
        printf("\n1_ Carregar arquivo de memória (.mem)");
        printf("\n2_ Mostrar memória");
        printf("\n3_ Mostrar banco de registradores");
        printf("\n4_ Mostrar instruções em formato Assembly");
        printf("\n5_ Mostrar estado de execução");
        printf("\n6_ Mostrar todo o simulador");
        printf("\n7_ Salvar arquivo Assembly (.asm)");
        printf("\n8_ Salvar arquivo de memória (.mem)");
        printf("\n9_ Rodar programa");
        printf("\n10_ Rodar 1 ciclo");
        printf("\n11_ Voltar 1 instrução");
        printf("\n12_ Exibir estatísticas");
        printf("\n13_ Resetar simulador");
        printf("\n------------------------------------");
        printf("\nInforme a opção desejada: ");
        scanf("%d", &op);

        switch(op){
            case 0: printf("\nEncerrando simulador.."); break;
            case 1: 
                char data_name[128];
                printf("\n--------------------------------------\n");
                printf("\nInforme o nome do arquivo: ");
                scanf("%127s", data_name);
                get_mem_file(data_name, m->memory->data, &m->memory->inst_loaded_count, &m->memory->data_loaded_count);
                break;
            
            case 2: 
                printf("\n--------------------------------------\n");
                print_memory(m->memory); break;
            case 3: 
                printf("\n--------------------------------------\n");
                print_regs(m->regs_bank); break;
            case 4: 
                printf("\n--------------------------------------\n");
                if (m->memory->inst_loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para exibir o Assembly, carregue a memória primeiro.\n");
                    break;
                }
                exibeTodos_asm(m->memory);
                break;
            
            case 5: 
                printf("\n--------------------------------------\n");
                print_ex_status(m);
                print_temp_regs(m);
                break;

            case 6:	
                printf("\n--------------------------------------\n");
                print_memory(m->memory); 
                print_regs(m->regs_bank); 
                print_temp_regs(m);
                printf("\nENDEREÇO DO PC: %d\n", m->pc->pc_index); 
                break;
            case 7: 
            
                if (m->memory->inst_loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para gerar o arquivo Assembly, carregue a memória primeiro.\n");
                    break;
                }
                mem_to_asm(m->memory);
                break;
            
            case 8: 
            output(m->memory);
            break;
            case 9: 
                run(m);
                break;
            
            case 10:
                empilhar(&pilha, m);
				run_step(m);
            break;
            
            case 11:
            desempilhar(&pilha,m);
            printf("CICLO %d", m->controle->states->ciclos);
            print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
            break;
            
            case 12: 
                printf("\n--------------------------------------\n");
				if (!m->memory || m->memory->inst_loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para exibir as estatísticas, carregue instruções primeiro e rode o programa primeiro.\n");
                    break;
                }
				programStat(m->memory, op, m->stats, m);
            break;
            case 13:
                printf("\n--------------------------------------\n");
                printf("\nResetando componentes...");
                printf("\nResetando PC, banco de registradores, memória e controle...");
                reset_all(m);
                printf("\nSimulador resetado com sucesso!");
            break;
            case 14:
            int cont = 0;
            for(cont = 0; cont <= 128; cont++){
                empilhar(&pilha, m);
                run_step(m);
            }
                
            break;
            case 15:
            int contd = 0;
            for(contd = 0; contd <=128 ; contd++){
                desempilhar(&pilha, m);
            }
                
            break;
            default: 
                printf("\n--------------------------------------\n");
                printf("\nOpção inválida! Selecione alguma entre 0 e 13."); break;
        }
    }while(op != 0);

    return 0;
    }
