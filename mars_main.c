#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include "decoder.h"
#include "pipeline.h"

int main() {
    Pipeline *p = pipeline_create();
    Pipeline *p_backup = pipeline_create();
    Memoria_instrucao *new_mem = NULL;
    
    int yMAX, xMAX;
    int h_menu = 12, w_menu = 40, op, marc = 0;
    int h_regs = 11, w_regs;
    int h_log = 18, w_log = 60;
    int h_exec, w_exec;
    char mem_name[128];
    char data_name[128];

    if (!p || !p_backup) {
        printf("\nFalha ao alocar pipeline.\n");
        return 1;
    }

    // Iniciando ncurses
    initscr();
    noecho();
    cbreak();

    getmaxyx(stdscr, yMAX, xMAX); // pega dimensões máximas da tela

    //   dimensões baseadas na tela
    w_regs = (xMAX - w_menu) - 2;
    h_exec = yMAX - (h_regs + 2) - 8;
    w_exec = (xMAX / 2) + 12;

    // JANELAS DO PROGRAMA
    WINDOW *menu = newwin((yMAX - (yMAX - h_menu - 1) - 2), (xMAX - w_exec - 3), 1, 1);
    WINDOW *mem = newwin((yMAX - h_menu), (xMAX - w_exec - 3), h_menu, 1);
    WINDOW *log = newwin((yMAX - h_regs - h_exec - 1), w_exec, (h_regs + h_exec + 1), (xMAX - w_exec - 1));
    WINDOW *regw = newwin(h_regs, w_exec, 1, (xMAX / 2 - 13));
    WINDOW *exec = newwin(h_exec, w_exec, h_regs + 1, (xMAX / 2 - 13));

    // Matriz com  opções do menu
    char options[][40] = {
        "Fechar simulador",
        "Carregar memória de instruções",
        "Carregar memória de dados",
        "Ver memória de instruções", 
        "Ver instruções em formato assembly",
        "Ver memória de dados",
        "Salvar assembly",
        "Salvar memória de dados",
        "Rodar programa",
        "Rodar 1 instrução", 
        "Voltar 1 instrução",
        "Resetar simulador"
    };

    int num_options = sizeof(options) / sizeof(options[0]);

    // Caixas das janelas
    box(menu, 0, 0);
    box(exec, 0, 0);
    box(mem, 0, 0);
    box(regw, 0, 0);
    box(log, 0, 0);

    // Exibe as janelas
    refresh();
    wrefresh(menu);
    wrefresh(exec);
    //ja mostra no painel assim que inicia 
    programHead(13, p, p->mem_inst, regw);
    mvwprintw(mem, 1, 1, "[MEMORIAS]");
    wrefresh(mem);
    wrefresh(regw);
    mvwprintw(log, 1, 1, "[LOG DE EXECUCAO]");
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
        int pos_x = 2 + (coluna * 40);
        
        if (i == marc) wattron(menu, A_REVERSE);
        
        mvwprintw(menu, linha + 2, pos_x, "%s", options[i]);
        
        if (i == marc) wattroff(menu, A_REVERSE);
    }
        wrefresh(menu);

        op = wgetch(menu);

        switch (op) {
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

                    case 1: // Carregar memória de instruções
                        echo();
                        mvwprintw(exec, 1, 2, "Informe o nome do arquivo .mem: ");
                        wrefresh(exec);
                        wgetnstr(exec, mem_name, 127); 
                        noecho();
                        
                        werase(exec); 
                        box(exec, 0, 0);
                        programHead(13, p, p->mem_inst, regw); // Atualiza cabecalho
                        wrefresh(regw);
                        
                        new_mem = instruction_memory_load_file(mem_name, &exec);
                        if (new_mem) {
                            if (p->mem_inst) {
                                free(p->mem_inst->instrucao);
                                free(p->mem_inst);
                            }
                            p->mem_inst = new_mem;
                            p->has_executed = 0;
                            p->just_rewound = 0;
                            mvwprintw(exec, 1, 2, "Memória de instruções carregada com sucesso!");
                        } else {
                            mvwprintw(exec, 1, 2, "Erro ao carregar a memória de instruções.");
                        }
                        wrefresh(exec);
                        break;

                    case 2: // Carregar memória de dados
                        echo();
                        mvwprintw(exec, 1, 2, "Informe o nome do arquivo .dat: ");
                        wrefresh(exec);
                        wgetnstr(exec, data_name, 127); 
                        noecho();
                        
                        werase(exec); 
                        box(exec, 0, 0);
                        
                        data_memory_load(p->mem_data, data_name);
                        mvwprintw(exec, 1, 2, "Memória de dados carregada com sucesso!");
                        wrefresh(exec);
                        break;
                    
                    case 3: // Ver memória de instruções
                        if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                            mvwprintw(exec, 1, 2, "Carregue instruções na memória primeiro.");
                        } else {
                            //print_instruction_memory(p->mem_inst, exec);
                        }
                        wrefresh(exec);
                        break;

                    case 4: // Ver instruções em formato assembly
                        if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                            mvwprintw(exec, 1, 2, "Carregue instruções na memória primeiro.");
                        } else {
                            //exibeTodos_asm(p->mem_inst, exec);
                        }
                        wrefresh(exec);
                        break;

                    case 5: // Ver memória de dados
                        //data_memory_print(p->mem_data, exec);
                        wrefresh(exec);
                        break;

                    case 6: // Salvar assembly
                        if (!p->mem_inst || p->mem_inst->loaded_count == 0) {
                            mvwprintw(exec, 1, 2, "Sem instruções para salvar. Carregue-as primeiro.");
                        } else {
                            mem_to_asm(p->mem_inst);
                            mvwprintw(exec, 1, 2, "Salvo no arquivo 'program.asm' com sucesso!");
                        }
                        wrefresh(exec);
                        break;

                    case 7: // Backup memória de dados
                        data_memory_save(p->mem_data, "output_dados.dat");
                        mvwprintw(exec, 1, 2, "Salvo no arquivo 'output_dados.dat' com sucesso!");
                        wrefresh(exec);
                        break;

                    case 8: // Rodar programa
                        run(p, exec, regw);
                        p->just_rewound = 1;
                        mvwprintw(exec, 1, 2, "Programa executado até o fim.");
                        print_regs(p->regs_bank, regw);

                        programHead(13, p, p->mem_inst, regw); // Atualiza stats
                        wrefresh(exec);
                        wrefresh(regw);
                        wrefresh(log);
                        break;

                    case 9: // Rodar 1 instrução
                        run_step(p, exec, regw);
                        print_regs(p->regs_bank, regw);
                        programHead(13, p, p->mem_inst, regw); // Atualiza stats
                        wrefresh(exec);
                        wrefresh(regw);
                        wrefresh(log);
                        break;

                    case 10: // Voltar 1 instrução
                        if (!p->has_executed) {
                            mvwprintw(exec, 1, 2, "Ao menos uma instrução deve ter sido executada.");
                        } else if (p->just_rewound) {
                            mvwprintw(exec, 1, 2, "Não é possível voltar duas instruções seguidas.");
                        } else {
                            copiaSimulador(p, p_backup);
                            p->just_rewound = 1;
                            mvwprintw(exec, 1, 2, "VOLTOU 1 INSTRUÇÃO! PC ESTÁ EM: %d", p->pc.pc_index);
                            print_regs(p->regs_bank, regw);
                            programHead(13, p, p->mem_inst, regw);
                        }
                        wrefresh(exec);
                        wrefresh(regw);
                        wrefresh(log);
                        break;

                    case 11: // Resetar simulador
                        reset_all(p);
                        mvwprintw(exec, 1, 2, "Simulador resetado com sucesso.");
                        werase(log); box(log, 0, 0); // Limpa as estatísticas visuais
                        print_regs(p->regs_bank, regw);
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