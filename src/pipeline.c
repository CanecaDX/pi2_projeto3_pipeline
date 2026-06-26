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


//verifica se o reg está vazio pelo bit de validade
int mem_wb_empty(Pipeline *p){return (p->mem_wb.v == 0) ? 1 : 0;}

int ex_mem_empty(Pipeline *p){return (p->ex_mem.v == 0) ? 1 : 0;}

int di_ex_empty(Pipeline *p){return (p->di_ex.v == 0) ? 1 : 0;}

int bi_di_empty(Pipeline *p){return (p->bi_di.v == 0) ? 1 : 0;}

Pipeline *pipeline_create(){
    Pipeline *p = calloc(1, sizeof(Pipeline));
    if (!p) return NULL;

	p->stats = stats_create();
    p->regs_bank = registers_create();
    p->mem_data = data_memory_create();
    p->mem_inst = instruction_memory_create();
    p->f = (Forward*)malloc(sizeof(Forward));
    p->has_executed = 0;
    p->just_rewound = 0;

    //busca
  
    p->instrucao_buscada.instr=0;
    //decodificao
  
    p->instrucao_decodificao.instr=0;
    //execucao

    p->instrucao_executada.instr=0;
    p->imm_dado_exec=0;  
    p->address_exec=0;
    //memoria
  
    p->instrucao_memoria.instr=0;
    p->Branch_mem=0;
    p->zero_mem=0;
    p->address_mem=0;
    p->branch_resultado_mem=0;
    p->ula_resultado_mem=0;
    p->rt_dado_mem=0;
    //wb
    
    p->RegWrite_wb=0;
    p->rd_wb = 0;
    p->instrucao_wb.instr=0;
    
    return p;
}
void buscar(Pipeline *p, WINDOW * exec, WINDOW *log){
    int yMax, xMax;
    getmaxyx(exec, yMax, xMax);
    int tam_coluna = (xMax - 4) / 5;
    int col_x = 2 + (0 * tam_coluna);
    char bufF[17];
    Decoded d;
    // COLUNA 1: Inicia em X = 2

    if(p->pc.pc_index >= p->mem_inst->loaded_count){
        //seta a instrução como 0 - bolha e 'bit' de validade como 0 - instrução inválida
        //para bolhas que entram no pipeline quando o programa termina
        p->bi_di.instrucao = 0;
        p->bi_di.v = 0;
        d = decode(p->bi_di.instrucao);
        get_asm_string(d, bufF, sizeof(bufF));
        p->bi_di.pc = p->pc.pc_index;
        return;
    }
    mvwprintw(exec, 1, col_x + 4, "    BUSCA    ");
    mvwprintw(exec, 2, col_x,"_____________________" );
    
   
    Instrucao *instr_lida = p->mem_inst->instrucao + p->pc.pc_index;
    p->instrucao_buscada.instr = instr_lida->instr;
    d = decode(instr_lida->instr);
    get_asm_string(d, bufF, sizeof(bufF));
    mvwprintw(exec, 4, col_x, bufF);
    

    p->pc.pc_index = p->pc.pc_index + 1;
    mvwprintw(exec, 6, col_x, "PC: %d", p->pc.pc_index-1); 
    set_bi_di(&p->bi_di, p->pc.pc_index, instr_lida->instr);

    //diz que a instrução é valida
    p->bi_di.v = 1;
    mvwprintw(log, 1, 1, "A INSTRUCAO BUSCADA REALIZA:");
    exibe1_asm_log(instr_lida->instr,log);
    wrefresh(exec);
}

void decodificar(Pipeline *p, WINDOW * exec){
    int yMax, xMax;
    getmaxyx(exec, yMax, xMax);
    int tam_coluna = (xMax - 4) / 5;
    int col_x = 2 + (1 * tam_coluna);
    char bufD[17];
    // COLUNA 2: Inicia em X = 22
    p->instrucao_decodificao.instr = p->bi_di.instrucao;
    p->decoded_inst = decode(p->bi_di.instrucao);
    get_asm_string(p->decoded_inst, bufD, sizeof(bufD));
    mvwprintw(exec, 1, col_x+4, "    DECODIFICACAO    ");
    mvwprintw(exec, 2, col_x,"___________________________" );
    mvwprintw(exec, 4, col_x, bufD);
    
    
    p->controle.input.opcode = p->decoded_inst.opcode;
    p->controle.input.function = p->decoded_inst.funct;

    p->controle.output = controle_sinais(p->controle.input, p->bi_di.v);
    // Sinais condensados em linha para caber na tela
    mvwprintw(exec, 6, col_x, "SINAIS DE CONTROLE:");
    if(p->controle.output.RegDst == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 7, col_x, "RegDst:   %d", p->controle.output.RegDst);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 7, col_x, "RegDst:   %d", p->controle.output.RegDst);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
     if(p->controle.output.RegWrite == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 8, col_x, "EscReg: %d", p->controle.output.RegWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 8, col_x, "EscReg: %d", p->controle.output.RegWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
      if(p->controle.output.Memtoreg == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 9, col_x, "MemParaReg: %d", p->controle.output.Memtoreg);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 9, col_x, "MemParaReg: %d", p->controle.output.Memtoreg);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    
     if(p->controle.output.MemRead == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 10, col_x, "LerMem:  %d", p->controle.output.MemRead);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 10, col_x, "LerMem:  %d", p->controle.output.MemRead);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    if(p->controle.output.MemWrite == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 11, col_x, "EscMem: %d", p->controle.output.MemWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 11, col_x, "EscMem: %d", p->controle.output.MemWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    mvwprintw(exec, 12, col_x, "ULAFonte: %d", p->controle.output.ULASrc);
    if(p->controle.output.Branch == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 13, col_x, "Branch: %d", p->controle.output.Branch);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 13, col_x, "Branch: %d", p->controle.output.Branch);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    if(p->controle.output.jump == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 14, col_x, "Jump:  %d", p->controle.output.jump);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 14, col_x, "Jump:  %d", p->controle.output.jump);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    mvwprintw(exec, 15, col_x, "ULA_op: %d", p->controle.output.ULA_op);

    p->regs_bank->in_regs.reg_base1 = p->decoded_inst.rs;
    p->regs_bank->in_regs.reg_base2 = p->decoded_inst.rt;
    p->regs_bank->in_regs.write_reg = 0;
    p->regs_bank->out_regs = ex_registers(p->regs_bank->in_regs, p->regs_bank);

    set_di_ex(&p->di_ex, p->bi_di.pc, p->bi_di.instrucao, p->decoded_inst.opcode, p->decoded_inst.rt,
              p->decoded_inst.rd, p->decoded_inst.funct, p->controle.output.RegDst,
              p->controle.output.RegWrite, p->controle.output.Memtoreg, p->controle.output.ULASrc,
              p->controle.output.MemRead, p->controle.output.MemWrite, p->controle.output.Branch,
              p->controle.output.jump, p->controle.output.ULA_op, p->regs_bank->out_regs.val1,
              p->regs_bank->out_regs.val2, p->decoded_inst.imm, p->decoded_inst.address, p->decoded_inst.rs);
              p->di_ex.v = p->bi_di.v;
              
    wrefresh(exec);
    clear_bi_di(&p->bi_di);
}

void executar(Pipeline *p, WINDOW * exec){
    int yMax, xMax;
    getmaxyx(exec, yMax, xMax);
    int tam_coluna = (xMax - 4) / 5;
    int col_x = 2 + (2 * tam_coluna);
    // COLUNA 3: Inicia em X = 42
    char bufEx[17];
    int dado_final_rs = p->di_ex.rs_dado; //pega os valores padrões
    int dado_final_rt = p->di_ex.rt_dado;
    p->instrucao_executada.instr = p->di_ex.instrucao;
    Decoded d = decode(p->di_ex.instrucao);
    get_asm_string(d, bufEx, sizeof(bufEx));

    int linha_fw = 12;

    mvwprintw(exec, 1, col_x+4, "    EXECUCAO    ");
    mvwprintw(exec, 2, col_x,"_______________________" );
    mvwprintw(exec, 4, col_x, bufEx);

    mvwprintw(exec, 6, col_x, "SINAIS DE FORWARDING");
    mvwprintw(exec, 7, col_x, "$rs: %d estagio (s)", p->f->A);
    mvwprintw(exec, 8, col_x, "$rt: %d estagio (s)", p->f->B);

    //sinais 1 e 2 pro foward representam a distancia dos dados
    // 2: dado que veio do estágio de wb
    //1: dado que veio de estágio de acesso a memoria
    //A = rs; B = rt

    if (p->f->A != 0) {
        dado_final_rs = p->f->valA;
        mvwprintw(exec, 10, col_x, "Valor antecipado para $rs: %d", p->f->valA);
    }else if(p->f->A == 0){
        mvwprintw(exec, 10, col_x, "Valor $rs: %d ", dado_final_rs);
    }

    if (p->f->B != 0) {
        dado_final_rt = p->f->valB;
        mvwprintw(exec, 11, col_x, "Valor antecipado para $rt: %d", p->f->valB);
    }else if(p->f->B == 0){
        mvwprintw(exec, 11, col_x, "Valor $rt: %d", dado_final_rt);
    }
    
    p->address_exec = p->di_ex.address;
    p->imm_dado_exec = p->di_ex.imm_dado;
    mvwprintw(exec, 12, col_x, "Imediato:%d", p->di_ex.imm_dado);
    mvwprintw(exec, 13, col_x, "Endereco Jump:%d", p->di_ex.address);


    if (p->di_ex.RegDst == 0x1) {
        p->regs_bank->in_regs.reg_destino = p->di_ex.rd;
    } else {
        p->regs_bank->in_regs.reg_destino = p->di_ex.rt;
    }

    p->ula.input.op1 = dado_final_rs;
    p->ula.input.ula_op = p->di_ex.ULA_op;

    if (p->di_ex.ULASrc == 0x1) {
        p->ula.input.op2 = p->di_ex.imm_dado;
    } else {
        p->ula.input.op2 = dado_final_rt;
    }

    p->ula.output = ulaExecuta(&p->ula.input);
    
    mvwprintw(exec, 14, col_x, "Resultado ULA: %d", p->ula.output.resultado);
    
    if (p->ula.output.Overflow == 1) {
        mvwprintw(exec, 15, col_x, "OVERFLOW!");
    }
    
    if (p->ex_mem.Branch == 1 && p->ex_mem.zero == 1) {
        p->stats->beqT++;
    } else if (p->ex_mem.Branch == 1 && p->ex_mem.zero == 0) {
        p->stats->beqNT++;
    }

    set_ex_mem(&p->ex_mem, p->di_ex.pc, p->di_ex.instrucao, p->di_ex.opcode,
               p->regs_bank->in_regs.reg_destino, p->di_ex.RegWrite, p->di_ex.Memtoreg,
               p->di_ex.MemRead, p->di_ex.MemWrite, p->di_ex.Branch, p->di_ex.jump,
               dado_final_rt, p->ula.output.resultado, p->ula.output.Overflow, p->ula.output.zero, 
               (uint8_t)(p->di_ex.pc + p->di_ex.imm_dado), p->di_ex.address);
               p->ex_mem.v = p->di_ex.v;
    wrefresh(exec);
    clear_di_ex(&p->di_ex);
}

void acesso_memoria(Pipeline *p, WINDOW * exec){
    int yMax, xMax;
    getmaxyx(exec, yMax, xMax);
    int tam_coluna = (xMax - 4) / 5;
    int col_x = 2 + (3 * tam_coluna);    
    char buf[17];
    p->instrucao_memoria.instr = p->ex_mem.instrucao;
    Decoded d = decode(p->ex_mem.instrucao);
    get_asm_string(d, buf, sizeof(buf));
    mvwprintw(exec, 1, col_x+4, "    ACESSO A MEMORIA    ");
    mvwprintw(exec, 2, col_x,"______________________________" );
    mvwprintw(exec, 4, col_x, buf);

    p->Branch_mem=p->ex_mem.Branch;
    p->zero_mem=p->ex_mem.zero;
    p->address_mem=p->ex_mem.address;
    p->branch_resultado_mem=p->ex_mem.branch_resultado;
    p->ula_resultado_mem=p->ex_mem.ula_resultado;
    p->rt_dado_mem=p->ex_mem.rt_dado;

    mvwprintw(exec, 6, col_x, "Resultado ULA:  %d", p->ex_mem.ula_resultado);
    mvwprintw(exec, 7, col_x, "Dado a escrever:  %d", p->ex_mem.rt_dado);

    mvwprintw(exec, 8, col_x, "Endereco Branch:   %d", p->ex_mem.branch_resultado);
    mvwprintw(exec, 9, col_x, "Endereco Jump:  %d", p->ex_mem.address);


    if (p->ex_mem.jump == 1) {
        // colocar log de execucao que desviou para tal endereco
        p->pc.pc_index = p->ex_mem.address;
    } else if (p->ex_mem.Branch == 1 && p->ex_mem.zero == 1) {
        mvwprintw(exec, 10, col_x, "Flag 0 ULA:  %d", p->ex_mem.zero);
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 11, col_x, "Branch tomado!");
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));
        p->stats->beqT++;
        // colocar log de execucao que branch deu certo para tal endereco
        p->pc.pc_index = p->ex_mem.branch_resultado;
    } else if (p->ex_mem.Branch == 1 && p->ex_mem.zero == 0) {
        mvwprintw(exec, 10, col_x, "Flag 0 ULA:  %d", p->ex_mem.zero);
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 11, col_x, "Branch nao tomado!");
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
        p->stats->beqNT++;
    // colocar log de execucao que branch nao deu certo para tal endereco
    }


    p->mem_data->in_data.adress = p->ex_mem.ula_resultado;
    p->mem_data->in_data.write_mem = p->ex_mem.MemWrite;
    p->mem_data->in_data.read_mem = p->ex_mem.MemRead;
    p->mem_data->in_data.dado_escrever = p->ex_mem.rt_dado;
    p->mem_data->out_data = ex_data_mem(p->mem_data->in_data, p->mem_data);

    set_mem_wb(&p->mem_wb, p->ex_mem.instrucao, p->ex_mem.opcode, p->ex_mem.rd,
               p->ex_mem.RegWrite, p->ex_mem.Memtoreg, p->ex_mem.ula_resultado,
               p->mem_data->out_data.dado_lido);
               p->mem_wb.v = p->ex_mem.v;
    wrefresh(exec);
    p->ex_mem.address = 0;
    clear_ex_mem(&p->ex_mem);

}

void write_back(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log){
    int yMax, xMax;
    getmaxyx(exec, yMax, xMax);
    int tam_coluna = (xMax - 4) / 5;
    int col_x = 2 + (4 * tam_coluna);    
    char buf[17];
    p->instrucao_wb.instr = p->mem_wb.instrucao;
    Decoded d = decode(p->mem_wb.instrucao);
    get_asm_string(d, buf, sizeof(buf));
    mvwprintw(exec, 1, col_x+6, "    ESCRITA DE RETORNO    ");
    mvwprintw(exec, 2, col_x+6,"____________________________" );
    mvwprintw(exec, 4, col_x+6, buf);

    if (p->mem_wb.Memtoreg == 0x1) {
        p->regs_bank->in_regs.dado_escrever = p->mem_wb.saida_mem;
    } else {
        p->regs_bank->in_regs.dado_escrever = p->mem_wb.ula_resultado;
    }

    p->regs_bank->in_regs.reg_destino = p->mem_wb.rd;
    mvwprintw(exec, 6, col_x+6, "Registrador de destino: r%d", p->mem_wb.rd);
    
    p->regs_bank->in_regs.write_reg = p->mem_wb.RegWrite;
    if (p->regs_bank->in_regs.reg_destino != 0) {
        p->regs_bank->out_regs = ex_registers(p->regs_bank->in_regs, p->regs_bank);
    }else if(p->regs_bank->in_regs.reg_destino == 0){
		p->stats->nops++;
        mvwprintw(log, 3, 2, "Registrador r$0 permite apenas leitura!");
    }
    
    mvwprintw(exec, 7, col_x+6, "Dado a escrever:  %d", p->regs_bank->in_regs.dado_escrever);
    mvwprintw(exec, 8, col_x+6, "EscReg:    %d", p->mem_wb.RegWrite);

    p->RegWrite_wb=p->mem_wb.RegWrite;
    p->rd_wb = p->mem_wb.rd;
    
    p->stats->contInsEx++;
    
    programHead(0, p, p->mem_inst, regw);
    clear_mem_wb(&p->mem_wb);
    wrefresh(exec);
    wrefresh(log);
}

void forward_unit(Pipeline *p){
    p->f->A = 0;
    p->f->B = 0;
    p->f->valA = 0;
    p->f->valB = 0;

    if(p->ex_mem.RegWrite && (p->ex_mem.rd != 0) && (p->ex_mem.rd == p->di_ex.rs)){
        p->f->A = 1;
        p->f->valA = p->ex_mem.ula_resultado; 
    }
    if(p->ex_mem.RegWrite && (p->ex_mem.rd != 0) && (p->ex_mem.rd == p->di_ex.rt)){
        p->f->B = 1;
        p->f->valB = p->ex_mem.ula_resultado; // Salva o valor real
    }

    if (p->mem_wb.RegWrite && (p->mem_wb.rd != 0) && (p->mem_wb.rd == p->di_ex.rs) && !(p->ex_mem.RegWrite && (p->ex_mem.rd == p->di_ex.rs))) {
        p->f->A = 2; 
        p->f->valA = (p->mem_wb.Memtoreg == 1) ? p->mem_wb.saida_mem : p->mem_wb.ula_resultado;
    }
    if (p->mem_wb.RegWrite && (p->mem_wb.rd != 0) && (p->mem_wb.rd == p->di_ex.rt) && !(p->ex_mem.RegWrite && (p->ex_mem.rd == p->di_ex.rt))) {
        p->f->B = 2; 
        p->f->valB = (p->mem_wb.Memtoreg == 1) ? p->mem_wb.saida_mem : p->mem_wb.ula_resultado;
    }
}

void mostra_estagios(Pipeline *p, WINDOW * exec){
    char bufF[17], bufD[17], bufEx[17], bufMem[17], bufWb[17];
    Decoded d;

    int yMax, xMax;
    getmaxyx(exec, yMax, xMax);
    int tam_coluna = (xMax - 4) / 5;

    // ==================== COLUNA 0: BUSCA ====================
    int col_x = 2 + (0 * tam_coluna);
    mvwprintw(exec, 1, col_x + 4, "    BUSCA    ");
    mvwprintw(exec, 2, col_x, "_____________________");
    
    d = decode(p->instrucao_buscada.instr);
    get_asm_string(d, bufF, sizeof(bufF));
    mvwprintw(exec, 4, col_x, bufF);
    mvwprintw(exec, 6, col_x, "PC: %d", p->pc.pc_index); 
        
    wrefresh(exec);

    // ==================== COLUNA 1: DECODIFICACAO ====================
    col_x = 2 + (1 * tam_coluna);
    p->decoded_inst = decode(p->instrucao_decodificao.instr);
    get_asm_string(p->decoded_inst, bufD, sizeof(bufD));
    mvwprintw(exec, 1, col_x + 4, "    DECODIFICACAO    ");
    mvwprintw(exec, 2, col_x, "___________________________");
    mvwprintw(exec, 4, col_x, bufD);
    
    p->controle.output = controle_sinais(p->controle.input, p->bi_di.v);
    
    mvwprintw(exec, 6, col_x, "SINAIS DE CONTROLE:");
    if(p->controle.output.RegDst == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 7, col_x, "RegDst:   %d", p->controle.output.RegDst);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 7, col_x, "RegDst:   %d", p->controle.output.RegDst);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    if(p->controle.output.RegWrite == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 8, col_x, "EscReg: %d", p->controle.output.RegWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 8, col_x, "EscReg: %d", p->controle.output.RegWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    if(p->controle.output.Memtoreg == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 9, col_x, "MemParaReg: %d", p->controle.output.Memtoreg);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 9, col_x, "MemParaReg: %d", p->controle.output.Memtoreg);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    
    if(p->controle.output.MemRead == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 10, col_x, "LerMem:  %d", p->controle.output.MemRead);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 10, col_x, "LerMem:  %d", p->controle.output.MemRead);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    if(p->controle.output.MemWrite == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 11, col_x, "EscMem: %d", p->controle.output.MemWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 11, col_x, "EscMem: %d", p->controle.output.MemWrite);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    mvwprintw(exec, 12, col_x, "ULAFonte: %d", p->controle.output.ULASrc);
    if(p->controle.output.Branch == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 13, col_x, "Branch: %d", p->controle.output.Branch);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 13, col_x, "Branch: %d", p->controle.output.Branch);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    if(p->controle.output.jump == 1){
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 14, col_x, "Jump:  %d", p->controle.output.jump);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));    
    } else {
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 14, col_x, "Jump:  %d", p->controle.output.jump);
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }
    mvwprintw(exec, 15, col_x, "ULA_op: %d", p->controle.output.ULA_op);

    // ==================== COLUNA 2: EXECUCAO ====================
    col_x = 2 + (2 * tam_coluna);
    int dado_final_rs = p->di_ex.rs_dado; 
    int dado_final_rt = p->di_ex.rt_dado;
    d = decode(p->instrucao_executada.instr);
    get_asm_string(d, bufEx, sizeof(bufEx));

    mvwprintw(exec, 1, col_x + 4, "    EXECUCAO    ");
    mvwprintw(exec, 2, col_x, "_______________________");
    mvwprintw(exec, 4, col_x, bufEx);
    
    mvwprintw(exec, 5, col_x, "Val rs:  %d (Fw:%d)", dado_final_rs, p->f->A);
    mvwprintw(exec, 6, col_x, "Val rt:  %d (Fw:%d)", dado_final_rt, p->f->B);
    mvwprintw(exec, 7, col_x, "Imediato:%d", p->imm_dado_exec);
    mvwprintw(exec, 8, col_x, "End Jump:%d", p->address_exec);    
    mvwprintw(exec, 9, col_x, "ULA Saida: %d", p->ula.output.resultado);
    
    if (p->ula.output.Overflow == 1) {
        mvwprintw(exec, 10, col_x, "OVERFLOW!");
    }

    // ==================== COLUNA 3: ACESSO A MEMORIA ====================
    col_x = 2 + (3 * tam_coluna);
    d = decode(p->instrucao_memoria.instr);
    get_asm_string(d, bufMem, sizeof(bufMem));
    mvwprintw(exec, 1, col_x + 4, "    ACESSO A MEMORIA    ");
    mvwprintw(exec, 2, col_x, "______________________________");
    mvwprintw(exec, 4, col_x, bufMem);

    mvwprintw(exec, 5, col_x, "ULA Res:  %d",  p->ula_resultado_mem);
    mvwprintw(exec, 6, col_x, "Dado Esc:  %d",  p->rt_dado_mem);
    mvwprintw(exec, 7, col_x, "End Br:   %d", p->branch_resultado_mem);
    mvwprintw(exec, 8, col_x, "End Jmp:  %d", p->address_mem);

    if (p->Branch_mem == 1 && p->zero_mem == 1) {
        mvwprintw(exec, 9, col_x, "ULA ZERO:  %d", p->zero_mem);
        wattron(exec, COLOR_PAIR(1));
        mvwprintw(exec, 10, col_x, "BRANCH FOI TOMADO");
        clrtoeol();
        wattroff(exec, COLOR_PAIR(1));
    } else if (p->Branch_mem == 1 && p->zero_mem == 0) {
        mvwprintw(exec, 9, col_x, "ULA ZERO:  %d", p->zero_mem);
        wattron(exec, COLOR_PAIR(2));
        mvwprintw(exec, 10, col_x, "BRANCH NAO FOI TOMADO");
        clrtoeol();
        wattroff(exec, COLOR_PAIR(2));
    }

    // ==================== COLUNA 4: RETORNO DA ESCRITA ====================
    col_x = 2 + (4 * tam_coluna);
    d = decode(p->instrucao_wb.instr);
    get_asm_string(d, bufWb, sizeof(bufWb));
    mvwprintw(exec, 1, col_x + 6, "    ESCRITA DE RETORNO    ");
    mvwprintw(exec, 2, col_x + 6, "____________________________");
    mvwprintw(exec, 4, col_x + 6, bufWb);

    mvwprintw(exec, 5, col_x + 6, "Reg Dest: r%d", p->rd_wb);
    mvwprintw(exec, 6, col_x + 6, "Dado Esc:  %d", p->regs_bank->in_regs.dado_escrever);
    mvwprintw(exec, 7, col_x + 6, "EscReg:    %d", p->RegWrite_wb);
}

void run_step(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log){

	if(p->mem_inst->loaded_count == 0){
        mvwprintw(log, 5, 1, "Carregue instruções na memória primeiro");
        wrefresh(log);
		return;
	}
	wclear(regw);
    programHead(13, p, p->mem_inst, regw);
    wrefresh(regw);
	p->stats->contCiclos++;
	
    werase(log);
    box(log, 0, 0);
	
	werase(exec);
    box(exec, 0, 0);

    wrefresh(log);

    //precisou chamar antes de rodar pra pegar os valores verdadeiros para rs e rt

    forward_unit(p);
	
    if (!mem_wb_empty(p)) {
        write_back(p, exec, regw, log);
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
    
    buscar(p, exec, log);

    return;
}

void run(Pipeline *p, WINDOW * exec, WINDOW * regw, WINDOW * log){
    int status = 0;
    if (p->mem_inst->loaded_count == 0) {
    mvwprintw(log, 5, 1, "Carregue instruções na memória primeiro!");
    wrefresh(log);
        return;
    }
    while (!pipeline_terminou(p)) {
		run_step(p, exec, regw, log);
    }
    wclear(log);
    wrefresh(log);
    mvwprintw(log, 5, 1, "Programa finalizado!");
    wrefresh(log);
    wrefresh(regw);
    wrefresh(exec);
    return;
}

void reset_run(Pipeline *p){
	p->pc.pc_index = 0;

	for(int i = 0; i < 8; i++){
		p->regs_bank->registradores[i] = 0;
	}

	return;
}

void reset_all(Pipeline *p, Pilha * pilha){

	reset_run(p);
    reset_stats(p);

    clear_bi_di(&p->bi_di);
    clear_di_ex(&p->di_ex);
    clear_ex_mem(&p->ex_mem);
    clear_mem_wb(&p->mem_wb);

    p->instrucao_buscada.instr = 0;
    
    p->instrucao_decodificao.instr = 0;
    
    p->instrucao_executada.instr = 0;
    p->imm_dado_exec = 0;  
    p->address_exec = 0;
    
    p->instrucao_memoria.instr = 0;
    p->Branch_mem = 0;
    p->zero_mem = 0;
    p->address_mem = 0;
    p->branch_resultado_mem = 0;
    p->ula_resultado_mem = 0;
    p->rt_dado_mem = 0;
    
    p->RegWrite_wb = 0;
    p->rd_wb = 0;
    p->instrucao_wb.instr = 0;

	for(int i = 0; i < 265; i++){
		p->mem_inst->instrucao[i].instr = 0;
		p->mem_data->dado[i] = 0;
	} 
}

void reset_stats(Pipeline *p){
	p->mem_inst->loaded_count = 0;
    p->stats->arit = 0;
    p->stats->contCiclos = 0;
    p->stats->contInsEx = 0;
    p->stats->cpi = 0;
    p->stats->desC = 0;
    p->stats->im = 0;
    p->stats->j = 0;
    p->stats->mem_d = 0;
    p->stats->r = 0;
    p->stats->beqNT = 0;
    p->stats->beqT = 0;
    p->stats->nops = 0;
    return;
}

int pipeline_terminou(Pipeline *p) {
    // O programa terminou se o PC passou do limite de instruções carregadas
    // E todos os registradores de estágio estão vazios 
    return (p->pc.pc_index >= p->mem_inst->loaded_count) &&
        (p->bi_di.v == 0) &&
        bi_di_empty(p) && 
        di_ex_empty(p) && 
        ex_mem_empty(p) && 
        mem_wb_empty(p);
}

void copiaStats(Stats *stats_backup,  Stats *stats){
 if (!stats_backup || !stats) return;
 *stats_backup = *stats;
}

void copiaSimulador (Pipeline* p_backup, Pipeline* p){
	 copiaBancoRegistradores(p_backup->regs_bank, p->regs_bank);
	 copiaMemoria(p_backup->mem_data, p->mem_data);
	 copiaPC(&p_backup->pc, &p->pc);
	 copiaControle(&p_backup->controle, &p->controle);
     copiaBI_DI(&p_backup->bi_di, &p->bi_di);
	 copiaDI_EX(&p_backup->di_ex, &p->di_ex);
	 copiaEX_MEM(&p_backup->ex_mem, &p->ex_mem);
	 copiaMEM_WB(&p_backup->mem_wb, &p->mem_wb);
	 copiaStats(p_backup->stats, p->stats);
	 p_backup->decoded_inst = p->decoded_inst;
	 p_backup->ula = p->ula;
	 p_backup->has_executed = p->has_executed;
	 p_backup->just_rewound = p->just_rewound;

     //busca
    p_backup->instrucao_buscada=p->instrucao_buscada;
    p_backup->instrucao_buscada.instr= p->instrucao_buscada.instr;
    //decodificao
    p_backup->instrucao_decodificao=p->instrucao_decodificao;
    p_backup->instrucao_decodificao.instr= p->instrucao_decodificao.instr;
    //execucao
    p_backup->instrucao_executada=p->instrucao_executada;
    p_backup->instrucao_executada.instr=p->instrucao_executada.instr;
    p_backup->imm_dado_exec=p->imm_dado_exec;  
    p_backup->address_exec=p->address_exec;
    //memoria
    p_backup->instrucao_memoria=p->instrucao_memoria;
    p_backup->instrucao_memoria.instr=p->instrucao_memoria.instr;
    p_backup->Branch_mem=p->Branch_mem;
    p_backup->zero_mem=p->zero_mem;
    p_backup->address_mem=p->address_mem;
    p_backup->branch_resultado_mem=p->branch_resultado_mem;
    p_backup->ula_resultado_mem=p->ula_resultado_mem;
    p_backup->rt_dado_mem=p->rt_dado_mem;
    //wb
    p_backup->instrucao_wb=p->instrucao_wb;
    p_backup->instrucao_wb.instr=p->instrucao_wb.instr;
    p_backup->RegWrite_wb=p->RegWrite_wb;
    p_backup->rd_wb = p->rd_wb;
     
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
			mvwprintw(regw, i + 2, 2, "$r%d=%d", i, p->regs_bank->registradores[i]);
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
        mvwprintw(regw, 2, 56, "Logica/Arit: %d", p->stats->arit);
        mvwprintw(regw, 3, 56, "Desvio Incond: %d", p->stats->j);
        mvwprintw(regw, 4, 56, "Desvio Cond: %d", p->stats->desC);
        mvwprintw(regw, 5, 56, "Transf. Dados: %d", p->stats->mem_d);
        
        mvwprintw(regw, 1, 74, "[Branchs e NOPs]");
        wattron(regw, COLOR_PAIR(2));
        mvwprintw(regw, 2, 74, "Branches 0: %d", p->stats->beqNT); 
        clrtoeol();
        wattroff(regw, COLOR_PAIR(2));
        wattron(regw, COLOR_PAIR(1));
        mvwprintw(regw, 3, 74, "Branches 1: %d", p->stats->beqT);
        clrtoeol();
        wattroff(regw, COLOR_PAIR(1)); 
        mvwprintw(regw, 4, 74, "NOPs: %d", p->stats->nops);  
        
    }
    
    // Renderiza as alterações na tela
    wrefresh(regw);
}

Pilha* criarPilha() {
    Pilha* p = (Pilha*) malloc(sizeof(Pilha));
    if (p != NULL) {
        p->topo = NULL;
        p->tamanho = 0;
    }
    return p;
}

void empilhar(Pilha * p, Pipeline* pipeline) {
    No *novoNo = (No *)malloc(sizeof(No));
    novoNo->pipeline = pipeline_create();
    if (novoNo == NULL) {
        printf("Erro de alocação de memória.\n");
        exit(1);
    }
    copiaSimulador(novoNo->pipeline, pipeline);
    novoNo->proximo = p->topo;
    p->topo = novoNo;
    p->tamanho++;
}

void desempilhar(Pilha * p, Pipeline* pipeline) {
    if (p->topo == NULL) {
        printf("A pilha está vazia.\n");
         // Valor indicativo de erro
    }
    No *temp = p->topo;
   	copiaSimulador(pipeline, temp->pipeline);
    
    p->topo = temp->proximo;
	p->tamanho--;
    free(temp);
}

void limpar_pilha(Pilha *p) {
    if (p == NULL) return;

    No *atual = p->topo;
    No *proximo = NULL;

    // Vira a pilha liberando nó por nó
    while (atual != NULL) {
        proximo = atual->proximo; // Salva o ponteiro para o próximo nó antes de apagar o atual

        // 1. Se o nó possui uma cópia própria do pipeline, limpa as sub-alocações dele
        if (atual->pipeline != NULL) {
            if (atual->pipeline->stats != NULL)     free(atual->pipeline->stats);
            if (atual->pipeline->regs_bank != NULL) free(atual->pipeline->regs_bank);
            if (atual->pipeline->mem_data != NULL)  free(atual->pipeline->mem_data);
            if (atual->pipeline->mem_inst != NULL)  free(atual->pipeline->mem_inst);
            if (atual->pipeline->f != NULL)         free(atual->pipeline->f);
            
            // Libera a estrutura principal do pipeline clonado neste nó
            free(atual->pipeline);
        }

        // 2. Libera a caixinha do Nó em si
        free(atual);

        // Avança para o próximo da fila
        atual = proximo;
    }

    // 3. Deixa o descritor da pilha totalmente zerado e pronto para reuso
    p->topo = NULL;
    p->tamanho = 0;
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
