#include <stdlib.h>
#include <stdio.h>
#include "pipeline.h"

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
void buscar(Pipeline *p){
	printf("\n");
	printf("----------- ESTAGIO DE BUSCA ------------  \n");
	Instrucao *instr_lida = p->mem_inst->instrucao + p->pc.pc_index;
	printf("INSTRUÇÃO BUSCADA: ");
	print_binary(instr_lida->instr);
	printf("   |   ");
	print_asm(decode(instr_lida->instr));
	printf("\n\n");
	p->pc.pc_index = p->pc.pc_index + 1;
	printf("PC ESTÁ EM: %d \n", p->pc.pc_index);
	set_bi_di(&p->bi_di, p->pc.pc_index, instr_lida->instr);
}

void decodificar(Pipeline *p){
	printf("\n");
	printf("----------- ESTAGIO DE DECODIFICACAO ------------  \n");
	p->decoded_inst = decode(p->bi_di.instrucao);
	printf("INSTRUÇÃO DECODIFICADA:\n");
	print_binary(p->bi_di.instrucao);
	printf("   |   ");
	print_asm(p->decoded_inst);
	printf("\n\n");

	p->controle.input.opcode = p->decoded_inst.opcode;
	p->controle.input.function = p->decoded_inst.funct;
	p->controle.output = controle_sinais(p->controle.input);
	printf("SINAIS GERADOS: \n RegDst: %d \n RegWrite: %d \n MemToReg: %d \n ULASrc: %d \n MemRead: %d \n MemWrite: %d \n Branch: %d \n Jump: %d \n ULA_op: %d \n",
	       p->controle.output.RegDst, p->controle.output.RegWrite, p->controle.output.Memtoreg,
	       p->controle.output.ULASrc, p->controle.output.MemRead, p->controle.output.MemWrite,
	       p->controle.output.Branch, p->controle.output.jump, p->controle.output.ULA_op);

	p->regs_bank->in_regs.reg_base1 = p->decoded_inst.rs;
	p->regs_bank->in_regs.reg_base2 = p->decoded_inst.rt;
	p->regs_bank->out_regs = ex_registers(p->regs_bank->in_regs, p->regs_bank);

	set_di_ex(&p->di_ex, p->bi_di.pc, p->bi_di.instrucao, p->decoded_inst.opcode, p->decoded_inst.rt,
	          p->decoded_inst.rd, p->decoded_inst.funct, p->controle.output.RegDst,
	          p->controle.output.RegWrite, p->controle.output.Memtoreg, p->controle.output.ULASrc,
	          p->controle.output.MemRead, p->controle.output.MemWrite, p->controle.output.Branch,
	          p->controle.output.jump, p->controle.output.ULA_op, p->regs_bank->out_regs.val1,
	          p->regs_bank->out_regs.val2, p->decoded_inst.imm, p->decoded_inst.address);
}

void executar(Pipeline *p){
	printf("\n");
	printf("----------- ESTAGIO DE EXECUCAO ------------  \n");
	printf("INSTRUÇÃO NO ESTAGIO DE EXECUÇÃO:  ");
	print_binary(p->di_ex.instrucao);
	printf("   |   ");
	print_asm(decode(p->di_ex.instrucao));
	printf("\n\n");
	printf("SINAIS GERADOS: \n RegDst: %d \n RegWrite: %d \n MemToReg: %d \n ULASrc: %d \n MemRead: %d \n MemWrite: %d \n Branch: %d \n Jump: %d \n ULA_op: %d \n",
	       p->di_ex.RegDst, p->di_ex.RegWrite, p->di_ex.Memtoreg, p->di_ex.ULASrc,
	       p->di_ex.MemRead, p->di_ex.MemWrite, p->di_ex.Branch, p->di_ex.jump, p->di_ex.ULA_op);
	printf("Valor de rs: %d \n Valor de rt: %d \n Valor do imediato: %d \n  Endereço jump: %d \n",
	       p->di_ex.rs_dado, p->di_ex.rt_dado, p->di_ex.imm_dado, p->di_ex.address);

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
	if (p->ula.output.Overflow == 1) {
		printf("\nOperação resultou em overflow. Atribuindo valor máximo de representação: %d", p->ula.output.resultado);
	}

	set_ex_mem(&p->ex_mem, p->di_ex.pc, p->di_ex.instrucao, p->di_ex.opcode,
	           p->regs_bank->in_regs.reg_destino, p->di_ex.RegWrite, p->di_ex.Memtoreg,
	           p->di_ex.MemRead, p->di_ex.MemWrite, p->di_ex.Branch, p->di_ex.jump,
	           p->di_ex.rt_dado, p->ula.output.resultado, p->ula.output.Overflow,
	           p->ula.output.zero, (uint8_t)(p->di_ex.pc + p->di_ex.imm_dado), p->di_ex.address);
}

void acesso_memoria(Pipeline *p){
	printf("\n");
	printf("----------- ESTAGIO DE ACESSO A MEMORIA ------------  \n");
	printf("INSTRUÇÃO NO ESTAGIO DE ACESSO A MEMORIA:  ");
	print_binary(p->ex_mem.instrucao);
	printf("   |   ");
	print_asm(decode(p->ex_mem.instrucao));
	printf("\n\n");
	printf("SINAIS GERADOS: \n RegWrite: %d \n MemToReg: %d  \n MemRead: %d \n MemWrite: %d \n Branch: %d \n Jump: %d \n Overflow: %d \n ULAZero: %d \n",
	       p->ex_mem.RegWrite, p->ex_mem.Memtoreg, p->ex_mem.MemRead, p->ex_mem.MemWrite,
	       p->ex_mem.Branch, p->ex_mem.jump, p->ex_mem.Overflow, p->ex_mem.zero);
	printf("Valor de ULASaida: %d \n Valor de rt: %d \n Endereço do branch: %d \n Endereço do Jump: %d \n",
	       p->ex_mem.ula_resultado, p->ex_mem.rt_dado, p->ex_mem.branch_resultado, p->ex_mem.address);

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
}

void write_back(Pipeline *p){
	printf("\n");
	printf("----------- ESTAGIO DE WRITE BACK ------------  \n");
	printf("INSTRUÇÃO NO ESTAGIO DE WRITE BACK:  ");
	print_binary(p->mem_wb.instrucao);
	printf("   |   ");
	print_asm(decode(p->mem_wb.instrucao));
	printf("\n\n");

	if (p->mem_wb.Memtoreg == 0x1) {
		p->regs_bank->in_regs.dado_escrever = p->mem_wb.saida_mem;
	} else {
		p->regs_bank->in_regs.dado_escrever = p->mem_wb.ula_resultado;
	}

	p->regs_bank->in_regs.reg_destino = p->mem_wb.rd;
	printf("RD: %d \n", p->mem_wb.rd);
	p->regs_bank->in_regs.write_reg = p->mem_wb.RegWrite;
	p->regs_bank->out_regs = ex_registers(p->regs_bank->in_regs, p->regs_bank);
	p->stats->contInsEx++;
	programStat(0, p, p->mem_inst);
}

int run_step(Pipeline *p){
	
	p->stats->contCiclos++;
	
    if (!mem_wb_empty(p)) {
        write_back(p);
    }
    if (!ex_mem_empty(p)) {
        acesso_memoria(p);
    }
    if (!di_ex_empty(p)) {
        executar(p);
    }
    if (!bi_di_empty(p)) {
        decodificar(p);
    }

    buscar(p);
    return 0;
}

int run(Pipeline *p){
    int status = 0;
    if (!p || !p->mem_inst) return -1;
    if (p->mem_inst->loaded_count == 0) {
        printf("Não há instruções carregadas na memória. Para rodar um programa, carregue instruções primeiro.\n");
        return 1;
    }
    while ((status = run_step(p)) == 0) {
    }
    printf("Programa finalizado!");
    p->pc.pc_index = 0;
    return status;
}

int run_back(Pipeline *p){
    (void)p;
    return -1;
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

void programStat(int cond, Pipeline *p, Memoria_instrucao *mem ){
	
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

	if(cond == 13){ //exibe stats
			printf("\n%d instruções carregadas\n", count);	
			printf("\nInstruções Executadas: %d\n", p->stats->contInsEx);
			printf("\nCiclos Executados: %d\n", p->stats->contCiclos);
			printf("\nCPI MÉDIO: %.2f\n", p->stats->cpi);
			printf("\nTIPOS");
			printf("\n%d instruções do tipo R", p->stats->r);
			printf("\n%d instruções do tipo I", p->stats->im);
			printf("\n%d instruções do tipo J", p->stats->j);
			printf("\n");
			printf("\nCLASSES");
			printf("\n%d operações de lógica e aritimética;", p->stats->arit);
			printf("\n%d operações de desvio incodicional;", p->stats->j);
			printf("\n%d operações de desvio condicional;", p->stats->desC);
			printf("\n%d operações de transferência de dados.", p->stats->mem_d);
	}
}
