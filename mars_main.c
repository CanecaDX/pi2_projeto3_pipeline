#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include "decoder.h"
#include "pipeline.h"

    
int main(){
    Pipeline *p = pipeline_create();
    Pipeline  *p_backup = pipeline_create();
    WINDOW *pipeline_data = newwin(20,20,0,0);

    //Iniciando ncurses
    initscr();
    noecho();
    cbreak();

    int yMAX, xMAX;
    getmaxyx(stdscr, yMAX, xMAX); //pega dimensões máximas da tela

    //int start_x = (xMAX - w) / 2; -> como centralizar janela em largura
    //int start_y = (yMAX - h) / 2; -> como centralizar janela em altura

    //caracteristicas menu
    int h_menu = 18, w_menu = 60, op, marc = 0;

    //caracteristicas registradores
    int h_regs = 10, w_regs = (xMAX/2) -3;
    
    //caracteristicas log
    int h_log = 18, w_log = 60;

    //caracteristicas execução
    int h_exec = yMAX-(h_regs+2), w_exec = (xMAX/2) - 3; 


    //JANELAS DO PROGRAMA
    WINDOW *exec = newwin(h_exec, w_exec, h_regs+2, (xMAX/2 + 1));
    WINDOW *regs = newwin(h_regs, w_regs, 1, (xMAX/2 + 1));
    WINDOW *log = newwin((yMAX/2 - 1), w_log, (yMAX - h_log), 1);
    WINDOW *menu = newwin((yMAX/2 - 1), w_menu, 1, 1);


    //matriz com as opções do menu
    char options[][34] = {
        "Fechar simulador",
        "Carregar memória de instruções",
        "Carregar memória de dados",
        "Ver memória de instruções", 
        "Ver instruções em formato assembly",
        "Ver memória de dados",
        "Salvar assembly",
        "Backup memória de dados",
        "Rodar programa",
        "Rodar 1 instrução", 
        "Voltar 1 instrução", 
        "Resetar simulador"
    };

    //quantidade de opções do menu
    int num_options = sizeof(options) / sizeof(options[0]);

    // para mostrar tamanho máximo da tela
    //mvwprintw(menu, 1, 1, "yMAX = %d, xMAX = %d", yMAX, xMAX);
    //wrefresh(menu);

    //caixas das janelas
    box(menu, 0, 0);
    box(exec, 0, 0);
    box(regs, 0, 0);
    box(log, 0, 0);

    //exibição das janelas
    refresh();
    wrefresh(menu);
    wrefresh(exec);
    wrefresh(regs);
    wrefresh(log);
    keypad(menu, true);

    while (1) {
    werase(menu);
    box(menu, 0, 0);

    // Draw menu options
    for (int i = 0; i < num_options; i++) {
        if (i == marc)
            wattron(menu, A_REVERSE);

        mvwprintw(menu, i + 2, 2, "%s", options[i]);

        if (i == marc)
            wattroff(menu, A_REVERSE);
    }

    wrefresh(menu);

    op = wgetch(menu);

    switch (op) {
        case KEY_UP:
            marc--;
            if (marc < 0)
                marc = num_options - 1;
            break;

        case KEY_DOWN:
            marc++;
            if (marc >= num_options)
                marc = 0;
            break;

        case '\n':  // Enter key
            switch (marc) {
                case 0:
                    //mvwprintw(closin, 1, 1, "Fechando simulador... pressione qualquer tecla.");
                    //wrefresh(closin);

                    //wgetch(closin); 

                    //delwin(closin);
                    //delwin(menu);
                    endwin();
                    return 0;

                case 1:
                    // Carregar memória de instruções
                    break;

                case 2:
                    // Carregar memória de dados
                    break;

                // add the remaining cases here
            }
            break;
    }
}

    getch();
    endwin();

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
        printf("\n1_ Carregar arquivo de memória de instruções (.mem)");
        printf("\n2_ Carregar arquivo de memória de dados (.dat)");
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
        printf("\n14_ Resetar simulador");
        printf("\n------------------------------------");
        printf("\nInforme a opção desejada: ");
        scanf("%d", &op);

        switch(op){
            case 0: printf("\nEncerrando simulador..\n"); break;
            case 1: {
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
            case 2: {
                char data_name[128];
                printf("\nInforme o arquivo .dat: ");
                scanf("%127s", data_name);
                data_memory_load(p->mem_data, data_name);
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
            case 7:	print_instruction_memory(p->mem_inst); printf("\n"); data_memory_print(p->mem_data); print_regs(p->regs_bank); printf("\n"); exibeTodos_asm(p->mem_inst); printf("\n"); printf("\n\nPC ESTÁ EM : %d\n", p->pc.pc_index); break;
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
                     if(p->pc.pc_index == p->mem_inst->loaded_count){
                        p->pc.pc_index = 0;
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
				printf("PC ESTA EM : %d \n", p->pc.pc_index);
                break;
            }
            case 13: 
				if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                    printf("Não existem instruções na memória ainda. Para exibir as estatísticas, carregue instruções primeiro.\n");
                    break;
                }
				programStat(13, p, p->mem_inst);
            break;

            case 14: reset_all(p); printf("\nSimulador resetado...\n");
            break;

            default: printf("\nOpção inválida!"); break;
        }
    }while(op != 0);

    return 0;
}
