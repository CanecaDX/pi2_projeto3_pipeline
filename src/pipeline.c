#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "pipeline.h"
#include "registers.h"

Stats *stats_create(void){
	Stats *stats = malloc(sizeof(Stats));
	stats->arit = 0;
	stats->im = 0;
	stats->j = 0;
	stats->desC = 0;
	stats->r = 0;
	stats->mem_d = 0; 
	stats->contCiclos = 0;
	stats->contInsEx = 0;
	stats->cpi = 0;
	return stats;
}


int mem_wb_empty(Pipeline *p){

    if (p->mem_wb.instrucao == 0 &&
        p->mem_wb.opcode == 0 &&
        p->mem_wb.rd == 0 &&
        p->mem_wb.RegWrite == 0 &&
        p->mem_wb.Memtoreg == 0 &&
        p->mem_wb.ula_resultado == 0 &&
        p->mem_wb.saida_mem == 0) {
        return 1;
    }

    return 0;
}

int ex_mem_empty(Pipeline *p){

    if (p->ex_mem.pc == 0 &&
        p->ex_mem.instrucao == 0 &&
        p->ex_mem.opcode == 0 &&
        p->ex_mem.rt_dado == 0 &&
        p->ex_mem.rd == 0 &&
        p->ex_mem.RegWrite == 0 &&
        p->ex_mem.Memtoreg == 0 &&
        p->ex_mem.MemRead == 0 &&
        p->ex_mem.MemWrite == 0 &&
        p->ex_mem.Branch == 0 &&
        p->ex_mem.jump == 0 &&
        p->ex_mem.ula_resultado == 0 &&
        p->ex_mem.Overflow == 0 &&
        p->ex_mem.zero == 0 &&
        p->ex_mem.rt_dado == 0) {
        return 1;
    }

    return 0;
}

int di_ex_empty(Pipeline *p){

    if (p->di_ex.pc == 0 &&
        p->di_ex.instrucao == 0 &&
        p->di_ex.opcode == 0 &&
        p->di_ex.rs_dado == 0 &&
        p->di_ex.rt == 0 &&
        p->di_ex.rd == 0 &&
        p->di_ex.funct == 0 &&
        p->di_ex.imm_dado == 0 &&
        p->di_ex.RegDst == 0 &&
        p->di_ex.RegWrite == 0 &&
        p->di_ex.Memtoreg == 0 &&
        p->di_ex.ULASrc == 0 &&
        p->di_ex.MemRead == 0 &&
        p->di_ex.MemWrite == 0 &&
        p->di_ex.Branch == 0 &&
        p->di_ex.jump == 0 &&
        p->di_ex.ULA_op == 0 &&
        p->di_ex.rt_dado == 0 &&
        p->di_ex.imm_dado == 0) {
        return 1;
    }

    return 0;
}

int bi_di_empty(Pipeline *p){
    if (p->bi_di.pc == 0 &&
        p->bi_di.instrucao == 0) {
        return 1;
    }

    return 0;
}

Pipeline *pipeline_create(){
    Pipeline *p = calloc(1, sizeof(Pipeline));
    if (!p) return NULL;

	p->stats = stats_create();
    p->regs_bank = registers_create();
    p->mem_data = data_memory_create();
    p->mem_inst = instruction_memory_create();
    p->has_executed = 0;
    p->just_rewound = 0;

    return p;
}
void buscar(Pipeline *p, WINDOW * exec){

    // COLUNA 1: Inicia em X = 2
    int col_x = 2;
    mvwprintw(exec, 1, col_x, "=== FETCH ===");
    
    Instrucao *instr_lida = p->mem_inst->instrucao + p->pc.pc_index;
    
    mvwprintw(exec, 3, col_x, "PC: %d", p->pc.pc_index + 1); // Mostra o próximo PC amigavelmente
    
    p->pc.pc_index = p->pc.pc_index + 1;
    set_bi_di(&p->bi_di, p->pc.pc_index, instr_lida->instr);
    
    wrefresh(exec);
}

void decodificar(Pipeline *p, WINDOW * exec){
    // COLUNA 2: Inicia em X = 22
    int col_x = 22;
    mvwprintw(exec, 1, col_x, "=== DECODE ===");
    
    p->decoded_inst = decode(p->bi_di.instrucao);
    
    p->controle.input.opcode = p->decoded_inst.opcode;
    p->controle.input.function = p->decoded_inst.funct;
    p->controle.output = controle_sinais(p->controle.input);
    
    // Sinais condensados em linha para caber na tela
    mvwprintw(exec, 3, col_x, "SINAIS DE CTR:");
    mvwprintw(exec, 4, col_x, "RegDst:   %d", p->controle.output.RegDst);
    mvwprintw(exec, 5, col_x, "RegWrite: %d", p->controle.output.RegWrite);
    mvwprintw(exec, 6, col_x, "MemToReg: %d", p->controle.output.Memtoreg);
    mvwprintw(exec, 7, col_x, "ULASrc:   %d", p->controle.output.ULASrc);
    mvwprintw(exec, 8, col_x, "MemRead:  %d", p->controle.output.MemRead);
    mvwprintw(exec, 9, col_x, "MemWrite: %d", p->controle.output.MemWrite);
    mvwprintw(exec, 10, col_x, "Branch:   %d", p->controle.output.Branch);
    mvwprintw(exec, 11, col_x, "Jump:     %d", p->controle.output.jump);
    mvwprintw(exec, 12, col_x, "ULA_op:   %d", p->controle.output.ULA_op);

    p->regs_bank->in_regs.reg_base1 = p->decoded_inst.rs;
    p->regs_bank->in_regs.reg_base2 = p->decoded_inst.rt;
    p->regs_bank->in_regs.write_reg = 0;
    p->regs_bank->out_regs = ex_registers(p->regs_bank->in_regs, p->regs_bank);

    set_di_ex(&p->di_ex, p->bi_di.pc, p->bi_di.instrucao, p->decoded_inst.opcode, p->decoded_inst.rt,
              p->decoded_inst.rd, p->decoded_inst.funct, p->controle.output.RegDst,
              p->controle.output.RegWrite, p->controle.output.Memtoreg, p->controle.output.ULASrc,
              p->controle.output.MemRead, p->controle.output.MemWrite, p->controle.output.Branch,
              p->controle.output.jump, p->controle.output.ULA_op, p->regs_bank->out_regs.val1,
              p->regs_bank->out_regs.val2, p->decoded_inst.imm, p->decoded_inst.address);
              
    wrefresh(exec);
}

void executar(Pipeline *p, WINDOW * exec){
    // COLUNA 3: Inicia em X = 42
    int col_x = 42;
    mvwprintw(exec, 1, col_x, "=== EXECUTE ===");
    
    mvwprintw(exec, 3, col_x, "Val rs:  %d", p->di_ex.rs_dado);
    mvwprintw(exec, 4, col_x, "Val rt:  %d", p->di_ex.rt_dado);
    mvwprintw(exec, 5, col_x, "Imediato:%d", p->di_ex.imm_dado);
    mvwprintw(exec, 6, col_x, "End Jump:%d", p->di_ex.address);

    if (p->di_ex.RegDst == 0x1) {
        p->regs_bank->in_regs.reg_destino = p->di_ex.rd;
    } else {
        p->regs_bank->in_regs.reg_destino = p->di_ex.rt;
    }

    p->ula.input.op1 = p->di_ex.rs_dado;
    p->ula.input.ula_op = p->di_ex.ULA_op;
    if (p->di_ex.ULASrc == 0x1) {
        p->ula.input.op2 = p->di_ex.imm_dado;
    } else {
        p->ula.input.op2 = p->di_ex.rt_dado;
    }

    p->ula.output = ulaExecuta(&p->ula.input);
    
    mvwprintw(exec, 8, col_x, "ULA Out: %d", p->ula.output.resultado);
    
    if (p->ula.output.Overflow == 1) {
        mvwprintw(exec, 10, col_x, "OVERFLOW!");
    }

    set_ex_mem(&p->ex_mem, p->di_ex.pc, p->di_ex.instrucao, p->di_ex.opcode,
               p->regs_bank->in_regs.reg_destino, p->di_ex.RegWrite, p->di_ex.Memtoreg,
               p->di_ex.MemRead, p->di_ex.MemWrite, p->di_ex.Branch, p->di_ex.jump,
               p->di_ex.rt_dado, p->ula.output.resultado, p->ula.output.Overflow,
               p->ula.output.zero, (uint8_t)(p->di_ex.pc + p->di_ex.imm_dado), p->di_ex.address);
               
    wrefresh(exec);
}

void acesso_memoria(Pipeline *p, WINDOW * exec){
    // COLUNA 4: Inicia em X = 62
    int col_x = 62;
    mvwprintw(exec, 1, col_x, "=== MEMORY ===");
    
    mvwprintw(exec, 3, col_x, "ULA Res:  %d", p->ex_mem.ula_resultado);
    mvwprintw(exec, 4, col_x, "Dado Es:  %d", p->ex_mem.rt_dado);
    mvwprintw(exec, 5, col_x, "End Br:   %d", p->ex_mem.branch_resultado);
    mvwprintw(exec, 6, col_x, "End Jmp:  %d", p->ex_mem.address);

    if (p->ex_mem.jump == 1) {
        p->pc.pc_index = p->ex_mem.address;
    } else if (p->ex_mem.Branch == 1 && p->ex_mem.zero == 1) {
        p->pc.pc_index = p->ex_mem.branch_resultado;
    }

    p->mem_data->in_data.adress = p->ex_mem.ula_resultado;
    p->mem_data->in_data.write_mem = p->ex_mem.MemWrite;
    p->mem_data->in_data.read_mem = p->ex_mem.MemRead;
    p->mem_data->in_data.dado_escrever = p->ex_mem.rt_dado;
    p->mem_data->out_data = ex_data_mem(p->mem_data->in_data, p->mem_data);

    set_mem_wb(&p->mem_wb, p->ex_mem.instrucao, p->ex_mem.opcode, p->ex_mem.rd,
               p->ex_mem.RegWrite, p->ex_mem.Memtoreg, p->ex_mem.ula_resultado,
               p->mem_data->out_data.dado_lido);
               
    wrefresh(exec);
}

void write_back(Pipeline *p, WINDOW * exec, WINDOW * regw){
    // COLUNA 5: Inicia em X = 82
    int col_x = 82;
    mvwprintw(exec, 1, col_x, "=== W-BACK ===");

    if (p->mem_wb.Memtoreg == 0x1) {
        p->regs_bank->in_regs.dado_escrever = p->mem_wb.saida_mem;
    } else {
        p->regs_bank->in_regs.dado_escrever = p->mem_wb.ula_resultado;
    }

    p->regs_bank->in_regs.reg_destino = p->mem_wb.rd;
    mvwprintw(exec, 3, col_x, "Reg Dest: r%d", p->mem_wb.rd);
    
    p->regs_bank->in_regs.write_reg = p->mem_wb.RegWrite;
    p->regs_bank->out_regs = ex_registers(p->regs_bank->in_regs, p->regs_bank);
    
    mvwprintw(exec, 4, col_x, "Dado Wr:  %d", p->regs_bank->in_regs.dado_escrever);
    mvwprintw(exec, 5, col_x, "RegWr:    %d", p->mem_wb.RegWrite);
    
    p->stats->contInsEx++;
    
    // Atualiza o topo com o banco de registradores e estatísticas sem apagar o log
    programHead(0, p, p->mem_inst, regw);
}

void run_step(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log){

	if(p->mem_inst->loaded_count == 0){
        mvwprintw(log, 5, 1, "Carregue instruções na memória primeiro");
        wrefresh(log);
		return;
	}
	
	p->stats->contCiclos++;

    werase(log);
    box(log, 0, 0);
	
	werase(exec);
    box(exec, 0, 0);

    wrefresh(log);
	
    if (!mem_wb_empty(p)) {
        write_back(p, exec, regw);
    }
    if (!ex_mem_empty(p)) {
        acesso_memoria(p, exec);
    }
    if (!di_ex_empty(p)) {
        executar(p, exec);
    }
    if (!bi_di_empty(p)) {
        decodificar(p, exec);
    }

    buscar(p, exec);
    return;
}

void run(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log){
    int status = 0;
    if (p->mem_inst->loaded_count == 0) {
    mvwprintw(log, 5, 1, "Carregue instruções na memória primeiro!");
    wrefresh(log);
        return;
    }
    while (p->pc.pc_index < p->mem_inst->loaded_count) {
		run_step(p, exec, regw, log);
    }
    mvwprintw(log, 5, 1, "Programa finalizado!");
	reset_run(p);
    return;
}

void reset_run(Pipeline *p){
	p->pc.pc_index = 0;

	for(int i = 0; i < 8; i++){
		p->regs_bank->registradores[i] = 0;
	}

	return;
}

void reset_all(Pipeline *p){

	reset_run(p);

	for(int i = 0; i < 265; i++){
		p->mem_inst->instrucao[i].instr = 0;
		p->mem_data->dado[i] = 0;
	}

	p->mem_inst->loaded_count = 0;
}

void copiaSimulador (Pipeline* p_backup, Pipeline* p){
	 copiaBancoRegistradores(p_backup->regs_bank, p->regs_bank);
	 copiaMemoria(p_backup->mem_data, p->mem_data);
	 copiaPC(&p_backup->pc, &p->pc);
	 copiaControle(&p_backup->controle, &p->controle);
	 p_backup->bi_di = p->bi_di;
	 p_backup->di_ex = p->di_ex;
	 p_backup->ex_mem = p->ex_mem;
	 p_backup->mem_wb = p->mem_wb;
	 p_backup->decoded_inst = p->decoded_inst;
	 p_backup->ula = p->ula;
	 p_backup->has_executed = p->has_executed;
	 p_backup->just_rewound = p->just_rewound;
}

void programHead(int cond, Pipeline *p, Memoria_instrucao *mem, WINDOW * regw){
    
    int count = mem ? mem->loaded_count : 0;
    
    if(cond != 13){
        Decoded d = decode(p->mem_wb.instrucao);
        if(d.type == TYPE_R){
            p->stats->r++;
        }else if(d.type == TYPE_I){
            p->stats->im++;
        }else{
            p->stats->j++;  
        }   
            
        if(d.type == TYPE_I && (d.opcode == 0xB || d.opcode == 0xF))
            p->stats->mem_d++;
                
        if(d.type == TYPE_I && d.opcode == 0x8)
            p->stats->desC++;
                
        if((d.type == TYPE_R || d.type == TYPE_I) && (d.opcode != 0x8 && d.opcode != 0xB && d.opcode != 0xF))
            p->stats->arit++;
        
        p->stats->cpi = (float)p->stats->contCiclos / (float)p->stats->contInsEx;       
    }   
    
    werase(regw);
    box(regw, 0, 0);

    mvwprintw(regw, 1, 2, "[REGISTRADORES]");    
	for (int i = 0; i < 8; i++) {
			mvwprintw(regw, i + 2, 2, "$r%d=%08X", i, p->regs_bank->registradores[i]);
	}

    // COLUNAS DE ESTATÍSTICAS (Só aparecem se cond == 13)
    if(cond == 13){ 
        // COLUNA gerais (Inicia em X = 32)
        mvwprintw(regw, 1, 20, "[GERAL]");
        mvwprintw(regw, 2, 20, "Inst. Carregadas: %d", count);  
        mvwprintw(regw, 3, 20, "Inst. Executadas: %d", p->stats->contInsEx);
        mvwprintw(regw, 4, 20, "Ciclos Executados: %d", p->stats->contCiclos);
        mvwprintw(regw, 5, 20, "CPI MEDIO: %.2f", p->stats->cpi);

        // COLUNA tipos (Inicia em X = 58)
        mvwprintw(regw, 1, 44, "[TIPOS]");
        mvwprintw(regw, 2, 44, "Tipo R: %d", p->stats->r);
        mvwprintw(regw, 3, 44, "Tipo I: %d", p->stats->im);
        mvwprintw(regw, 4, 44, "Tipo J: %d", p->stats->j);

        // COLUNA classes (Inicia em X = 74)
        mvwprintw(regw, 1, 56, "[CLASSES]");
        mvwprintw(regw, 2, 56, "Logica/Arit.: %d", p->stats->arit);
        mvwprintw(regw, 3, 56, "Desvio Incond.: %d", p->stats->j);
        mvwprintw(regw, 4, 56, "Desvio Cond.: %d", p->stats->desC);
        mvwprintw(regw, 5, 56, "Transf. Dados: %d", p->stats->mem_d);
    }
    
    // Renderiza as alterações na tela
    wrefresh(regw);
}



// void programHead(int cond, Pipeline *p, Memoria_instrucao *mem, WINDOW * regw){
	
// 	int count = mem ? mem->loaded_count : 0;
	
// 	if(cond != 13){
// 			Decoded d = decode(p->mem_wb.instrucao);
// 			if(d.type == TYPE_R){
// 				p->stats->r++;
// 			}else if(d.type == TYPE_I){
// 				p->stats->im++;
// 			}else{
// 				p->stats->j++;	
// 			}	
				
// 			if(d.type == TYPE_I && (d.opcode == 0xB || d.opcode == 0xF))
// 				p->stats->mem_d++;
					
// 			if(d.type == TYPE_I && d.opcode == 0x8)
// 				p->stats->desC++;
					
// 			if((d.type == TYPE_R || d.type == TYPE_I) && (d.opcode != 0x8 && d.opcode != 0xB && d.opcode != 0xF))
// 				p->stats->arit++;
			
// 			p->stats->cpi = (float)p->stats->contCiclos / (float)p->stats->contInsEx;		
// 	}	
	
// 	for (int i = 0; i < 8; i++) {
//         mvwprintw(regw, i + 1, 2, "$r%-2d = %08X",
//                   i, p->regs_bank->registradores[i]);
//     }
//     wrefresh(regw);

// 	if(cond == 13){ //exibe stats
// 			printf("\n%d instruções carregadas\n", count);	
// 			printf("\nInstruções Executadas: %d\n", p->stats->contInsEx);
// 			printf("\nCiclos Executados: %d\n", p->stats->contCiclos);
// 			printf("\nCPI MÉDIO: %.2f\n", p->stats->cpi);
// 			printf("\nTIPOS");
// 			printf("\n%d instruções do tipo R", p->stats->r);
// 			printf("\n%d instruções do tipo I", p->stats->im);
// 			printf("\n%d instruções do tipo J", p->stats->j);
// 			printf("\n");
// 			printf("\nCLASSES");
// 			printf("\n%d operações de lógica e aritimética;", p->stats->arit);
// 			printf("\n%d operações de desvio incodicional;", p->stats->j);
// 			printf("\n%d operações de desvio condicional;", p->stats->desC);
// 			printf("\n%d operações de transferência de dados.", p->stats->mem_d);
// 	}
// }
