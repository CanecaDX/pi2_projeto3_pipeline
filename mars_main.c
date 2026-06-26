#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include "decoder.h"
#include "pipeline.h"

int main() {
    Pipeline *p = pipeline_create();
    Pilha* pilha = criarPilha();
    Memoria_instrucao *new_mem = NULL;
    
    int yMAX, xMAX;
    int h_menu = 12, w_menu = 40, op, marc = 0;
    int h_regs = 11, w_regs;
    int h_log = 18, w_log = 60;
    int h_exec, w_exec;
    char mem_name[128];
    char data_name[128];

    if (!p) {
        printf("\nFalha ao alocar pipeline.\n");
        return 1;
    }

    // Iniciando ncurses
    initscr();
    start_color();
    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal não suporta cores.\n");
        return 1;
    }
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    noecho();
    cbreak();

  

    

    getmaxyx(stdscr, yMAX, xMAX); // pega dimensões máximas da tela

    //   dimensões baseadas na tela
    w_regs = (xMAX - w_menu) - 2;
    h_exec = yMAX - (h_regs + 2) - 8;
    w_exec = xMAX - 2;

    // JANELAS DO PROGRAMA
    WINDOW *menu = newwin((yMAX - (yMAX - h_menu - 1) - 2), (xMAX - (xMAX/2 + 13) - 3), 1, 1);
    WINDOW *mem2 = newwin(yMAX, xMAX, 0, 0);
    WINDOW *log = newwin((yMAX - h_regs - h_exec - 1), w_exec, (h_regs + h_exec + 1), 1);
    WINDOW *regw = newwin(h_regs, (xMAX / 2 + 13), 1, (xMAX / 2 - 13));
    WINDOW *exec = newwin(h_exec, w_exec, h_regs + 1, 1);

    // Matriz com  opções do menu
    char options[][40] = {
        "Sair",
        "Carregar dados",
        "Carregar Instruções",
        "Memória de instruções",
        "Memória de dados", 
        "Ver instruções em assembly",
        "Salvar assembly",
        "Backup dados",
        "Rodar programa",
        "Rodar 1 ciclo", 
        "Voltar 1 ciclo",
        "Resetar simulador"
    };

    int num_options = sizeof(options) / sizeof(options[0]);

    // Caixas das janelas
    box(menu, 0, 0);
    box(exec, 0, 0);
    box(mem2, 0, 0);
    box(regw, 0, 0);
    box(log, 0, 0);

    // Exibe as janelas
    refresh();
    wrefresh(menu);
    wrefresh(exec);
    //ja mostra no painel assim que inicia 
    programHead(13, p, p->mem_inst, regw);
    //mvwprintw(regp, 1, 1, "[MEMORIAS]");
    wrefresh(regw);
    mvwprintw(log, 1, 1, "[TERMINAL]");
    //mvwprintw(log, 3, 1, "altura: %d || largura: %d", yMAX, xMAX); 37 e 162
    wrefresh(log);
    keypad(menu, true);

    while (1) {
        werase(menu);
        box(menu, 0, 0);

        mvwprintw(menu, 1, 1, "[MENU]");
        int itens_por_coluna = 6; // Metade de 12

    for (int i = 0; i < num_options; i++) {
        // Calcula a linha 
        int linha = i % itens_por_coluna;
        // Calcula a coluna 
        int coluna = i / itens_por_coluna;
        int pos_x =  2 + (coluna * 35);
        
        if (i == marc) wattron(menu, A_REVERSE);
        
        mvwprintw(menu, linha + 3, pos_x, "%s", options[i]);
        
        if (i == marc) wattroff(menu, A_REVERSE);
    }
        wrefresh(menu);

        op = wgetch(menu);

        switch (op) {        
			wclear(log);
			wrefresh(log);
            case KEY_UP:
                marc--;
                if (marc < 0) marc = num_options - 1;
                break;

            case KEY_DOWN:
                marc++;
                if (marc >= num_options) marc = 0;
                break;

            case '\n':  
                werase(exec);
                box(exec, 0, 0);

                switch (marc) {
                    case 0: // Fechar simulador
                        endwin();
                        return 0;

                    case 1: // Dados
						wrefresh(log);
                        echo();
                        mvwprintw(log, 3, 1, "Informe o nome do arquivo .dat: ");
                        wrefresh(log);
                        wgetnstr(log, data_name, 127); 
                        noecho();
                        
                        werase(log); 
                        box(log, 0, 0);
                        
                        data_memory_load(p->mem_data, data_name, log);
                        werase(log);
                        box(log, 0, 0);
                        break;
                    
                    case 2: // Instruções
						wrefresh(log);
                        echo();
                        mvwprintw(log, 3, 1, "Informe o nome do arquivo .mem: ");
                        wrefresh(log);
                        wgetnstr(log, mem_name, 127);
                        werase(log); 
                        box(log, 0, 0); 
                        noecho();
                        
                        
                        new_mem = instruction_memory_load_file(mem_name, &log);
                        if (new_mem) {
                            if (p->mem_inst) {
                                free(p->mem_inst->instrucao);
                                free(p->mem_inst);
                            }
                            p->mem_inst = new_mem;
                            p->has_executed = 0;
                            p->just_rewound = 0;
                            mvwprintw(log, 5, 1, "Memória de instruções carregada com sucesso!");
                            programHead(13, p, p->mem_inst, regw); // Atualiza cabecalho
                            wrefresh(regw);
                        } else {
                            mvwprintw(log, 5, 1, "Erro ao carregar a memória de instruções!");

                        }
                        wrefresh(log);
                        break;
                        
                    case 3: // Ver memória instrução
                        if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                            mvwprintw(log, 3, 1, "Carregue instruções na memória primeiro.");
                        } else {
							erase();
							clear();
							refresh();
							print_instruction_memory(p->mem_inst, mem2);
							wrefresh(mem2);						
							wgetch(mem2);
							
							werase(mem2);

							touchwin(menu);
							touchwin(log);
							touchwin(regw);
							touchwin(exec);

							wrefresh(menu);
							wrefresh(log);
							wrefresh(regw);
							wrefresh(exec);      
                        }
						break;
                    case 4:
						erase();
						clear();
						refresh();
						data_memory_print(p->mem_data, mem2);  
						wrefresh(mem2);
						wgetch(mem2);
		

                        werase(mem2);
                        wrefresh(mem2); 

                        touchwin(menu);  wrefresh(menu);
                        touchwin(log);   wrefresh(log);
                        touchwin(regw);  wrefresh(regw);
                        touchwin(exec);  wrefresh(exec);


					
						break;
                    case 5: // Ver instruções em formato assembly
                        if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                            mvwprintw(log, 3, 1, "Carregue instruções na memória primeiro.");
                        } else {
							erase();
							clear();
							refresh();
                            exibeTodos_asm(p->mem_inst, mem2);
                            
                            wrefresh(mem2);						
							wgetch(mem2);
							
							werase(mem2);

							touchwin(menu);
							touchwin(log);
							touchwin(regw);
							touchwin(exec);

							wrefresh(menu);
							wrefresh(log);
							wrefresh(regw);
							wrefresh(exec);      
                        }
                        wrefresh(log);
                        break;

                    case 6: // Salvar assembly
                        if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                            mvwprintw(log, 3, 1, "Sem instruções para salvar. Carregue-as primeiro.");
                        } else {
                            mem_to_asm(p->mem_inst);
                            mvwprintw(log, 3, 1, "Salvo no arquivo 'program.asm' com sucesso!");
                        }
                        wrefresh(log);
                        break;

                    case 7: // Backup memória de dados
                        data_memory_save(p->mem_data, "output_dados.dat");
                        mvwprintw(log, 3, 1, "Salvo no arquivo 'output_dados.dat' com sucesso!");
                        wrefresh(log);
                        break;

                    case 8: // Rodar programa completo
                        if (p->mem_inst->loaded_count == 0) {
                            mvwprintw(log, 3, 2, "Erro: Carregue as instrucoes primeiro!");
                            wrefresh(log);
                            break;
                        }

                        run(p, exec, regw, log);
                        p->just_rewound = 1; 
                        
                        programHead(13, p, p->mem_inst, regw); // Exibe estatísticas finais acumuladas
                        
                        wrefresh(exec);
                        wrefresh(regw);
                        wrefresh(log);

                        break;

                    case 9: // Rodar 1 ciclo
                        if (p->mem_inst->loaded_count == 0) {
                            mvwprintw(log, 3, 2, "Carregue as instrucoes primeiro!");
                            wrefresh(log);
                            break;
                        }
                        if (pipeline_terminou(p)) {
                            reset_run(p);
                            reset_stats(p);
                        }

                        empilhar(pilha, p);
                        
                        werase(log);
                        box(log, 0, 0);
                        mvwprintw(log, 1, 1, "[LOG DE EXECUCAO]");
                        mvwprintw(log, 2, 1, "Ciclo guardado na pilha (Tamanho: %d)", pilha->tamanho);
                        
                        run_step(p, exec, regw, log);
                        
                        print_regs(p->regs_bank, regw);
                        programHead(13, p, p->mem_inst, regw); 
                        wrefresh(exec);
                        wrefresh(regw);
                        wrefresh(log);
                        break;

                    case 10: // Voltar 1 ciclo
                        if (pilha->tamanho > 0) {
                            desempilhar(pilha, p);
                            mostra_estagios(p, exec);
                            werase(log);
                            box(log, 0, 0);
                            mvwprintw(log, 1, 1, "[LOG DE EXECUCAO]");
                            mvwprintw(log, 3, 1, "VOLTOU 1 CICLO! PC ATUAL: %d", p->pc.pc_index);
                            mvwprintw(log, 4, 1, "Estados na pilha: %d", pilha->tamanho);
                            
                            print_regs(p->regs_bank, regw);
                            programHead(13, p, p->mem_inst, regw);
                        } else {
                            mvwprintw(log, 3, 1, "Impossivel voltar: Inicio do programa atingido.");
                        }
                        wrefresh(exec);
                        wrefresh(regw);
                        wrefresh(log);
                        break;

                    case 11: // Resetar simulador explicitamente
                        reset_all(p);
                        
                        werase(log); 
                        box(log, 0, 0); 
                        mvwprintw(log, 3, 1, "Simulador resetado com sucesso!");
                        
                        programHead(13, p, p->mem_inst, regw);
                        wrefresh(exec);
                        wrefresh(log);
                        wrefresh(regw);
                        break;
                    }
                break;
        }
    }

    return 0;
}
