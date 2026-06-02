#include <stdlib.h>
#include <stdio.h>
#include "pipeline.h"

Pipeline *pipeline_create(void){
    Pipeline *p = malloc(sizeof(Pipeline));
    if (!p) return NULL;

    p->regs_bank = registers_create();
    p->mem_data = data_memory_create();
    p->mem_inst = instruction_memory_create();
    p->pc = pc_create(0);
    p->bi_di = bi_di_create();
    p->di_ex = di_ex_create();
    p->ex_mem = ex_mem_create();
    p->mem_wb = mem_wb_create();
    
    p->controle = NULL;
    p->decoded_inst = calloc(1, sizeof(Decoded));
    p->ula = calloc(1, sizeof(ULA));
    p->has_executed = 0;
    p->just_rewound = 0;

    if (!p->pc || !p->regs_bank || !p->mem_data || !p->mem_inst || !p->decoded_inst || !p->ula) {
        if (p->pc) {
            free(p->pc);
        }
        if (p->regs_bank) {
            free(p->regs_bank->registradores);
            free(p->regs_bank);
        }
        if (p->mem_data) {
            free(p->mem_data->dado);
            free(p->mem_data);
        }
        if (p->mem_inst) {
            free(p->mem_inst->instrucao);
            free(p->mem_inst);
        }
        if (p->decoded_inst) {
            free(p->decoded_inst);
        }
        if (p->ula) {
            free(p->ula);
        }
        free(p);
        return NULL;
    }

    return p;
}
void buscar(Pipeline *p){
	printf("----------- ESTAGIO DE BUSCA ------------  \n");
	Instrucao *instr_lida = NULL;
	instr_lida = (p->mem_inst->instrucao+p->pc->pc_index);
	printf("INSTRUÇÃO BUSCADA: ");
	print_binary(instr_lida->instr);
	printf("   |   ");
	print_asm(decode(instr_lida->instr));
	printf("\n\n");
	p->pc->pc_index = p->pc->pc_index + 1;
	printf("PC ESTÁ EM: %d \n", p->pc->pc_index);
	// seta os valores do registrador de pipeline bi_di
	set_bi_di(p->bi_di, p->pc->pc_index, instr_lida->instr);
}

void decodificar(Pipeline *p){
	printf("----------- ESTAGIO DE DECODIFICACAO ------------  \n");
	Decoded decoded_inst = {0};
    In_controle in_controle = {0};
    Out_controle sinais = {0};
    Out_registers regs_output = {0};
    decoded_inst = decode(p->bi_di->instrucao);
	printf("INSTRUÇÃO DECODIFICADA:\n");
	print_binary(p->bi_di->instrucao);
	printf("   |   ");
	print_asm(decoded_inst);
	printf("\n\n");
	// passa opcode e function para o controle, gera sinais
    in_controle.opcode = decoded_inst.opcode;
    in_controle.function = decoded_inst.funct;
    
    //sinais
    sinais = controle_sinais(in_controle);
    printf("SINAIS GERADOS: \n RegDst: %d \n RegWrite: %d \n MemToReg: %d \n ULASrc: %d \n MemRead: %d \n MemWrite: %d \n Branch: %d \n Jump: %d \n ULA_op: %d \n", sinais.RegDst, sinais.RegWrite, sinais.Memtoreg, sinais.ULASrc, sinais.MemRead, sinais.MemWrite, sinais.Branch, sinais.jump, sinais.ULA_op);
   
    //sinaliza qual é o rs
    p->regs_bank->in_regs.reg_base1 = decoded_inst.rs;
    p->regs_bank->in_regs.reg_base2 = decoded_inst.rt;
 
    //pega os valores dos registradores base
     regs_output.val1 = (int8_t)(p->regs_bank->registradores[p->regs_bank->in_regs.reg_base1]);
	 regs_output.val2 = (int8_t)(p->regs_bank->registradores[p->regs_bank->in_regs.reg_base2]);
	 
	 // seta os valores do registrador de pipeline di_ex
	 set_di_ex(p->di_ex, p->bi_di->pc, p->bi_di->instrucao, decoded_inst.opcode,  decoded_inst.rt,  decoded_inst.rd, decoded_inst.funct, sinais.RegDst, sinais.RegWrite, sinais.Memtoreg, sinais.ULASrc, sinais.MemRead, sinais.MemWrite, sinais.Branch, sinais.jump, sinais.ULA_op, regs_output.val1, regs_output.val2, decoded_inst.imm, decoded_inst.address);
  
}

void executar(Pipeline *p){
	In_ULA ula_input = {0};
    Out_ULA ula_output = {0};
    uint8_t rbranch = 0;
	printf("----------- ESTAGIO DE EXECUCAO ------------  \n");
	printf("INSTRUÇÃO NO ESTAGIO DE EXECUÇÃO:  ");
	print_binary(p->di_ex->instrucao);
	printf("   |   ");
	print_asm(decode(p->di_ex->instrucao));
	printf("\n\n");
	printf("SINAIS GERADOS: \n RegDst: %d \n RegWrite: %d \n MemToReg: %d \n ULASrc: %d \n MemRead: %d \n MemWrite: %d \n Branch: %d \n Jump: %d \n ULA_op: %d \n", p->di_ex->RegDst, p->di_ex->RegWrite, p->di_ex->Memtoreg, p->di_ex->ULASrc, p->di_ex->MemRead, p->di_ex->MemWrite, p->di_ex->Branch, p->di_ex->jump, p->di_ex->ULA_op);
	printf("Valor de rs: %d \n Valor de rt: %d \n Valor do imediato: %d \n  Endereço jump: %d \n", p->di_ex->rs_dado, p->di_ex->rt_dado, p->di_ex->imm_dado, p->di_ex->address);
	
	 //"MUX" que determina se o registrador de destino é rt ou rd
    if(p->di_ex->RegDst == 0x1){
        p->regs_bank->in_regs.reg_destino = p->di_ex->rd;
    } else {
        p->regs_bank->in_regs.reg_destino = p->di_ex->rt;   
    }

	//prepara os dados para a ula(reg base1, reg base2 ou imm)

    ula_input.op1 = p->di_ex->rs_dado;
    ula_input.ula_op = p->di_ex->ULA_op;
    

	rbranch = p->di_ex->pc + p->di_ex->imm_dado;
    //"MUX" que determina se o segundo op da ula é reg base 2 ou imm
    if(p->di_ex->ULASrc == 0x1){
        ula_input.op2 = p->di_ex->imm_dado;
    } else {
        ula_input.op2 = p->di_ex->rt_dado;
    }

    ula_output = ulaExecuta(&ula_input);
    
		if(ula_output.Overflow == 1){
				printf("\nOperação resultou em overflow. Atribuindo valor máximo de representação: %d", ula_output.resultado);
		}
	// seta os valores do registrador de pipeline ex_mem
	set_ex_mem(p->ex_mem, p->di_ex->pc, p->di_ex->instrucao, p->di_ex->opcode, p->regs_bank->in_regs.reg_destino, p->di_ex->RegWrite,  p->di_ex->Memtoreg,  p->di_ex->MemRead,  p->di_ex->MemWrite,  p->di_ex->Branch,  p->di_ex->jump,  p->di_ex->rt_dado, ula_output.resultado, ula_output.Overflow, ula_output.zero, rbranch, p->di_ex->address);
}

void acesso_memoria(Pipeline *p){
	In_data_mem data_mem_input = {0};
    Out_data_mem data_mem_output = {0};
	printf("----------- ESTAGIO DE ACESSO A MEMORIA ------------  \n");
	printf("INSTRUÇÃO NO ESTAGIO DE ACESSO A MEMORIA:  ");
	print_binary(p->ex_mem->instrucao);
	printf("   |   ");
	print_asm(decode(p->ex_mem->instrucao));
	printf("\n\n");
	printf("SINAIS GERADOS: \n RegWrite: %d \n MemToReg: %d  \n MemRead: %d \n MemWrite: %d \n Branch: %d \n Jump: %d \n Overflow: %d \n ULAZero: %d \n", p->ex_mem->RegWrite, p->ex_mem->Memtoreg, p->ex_mem->MemRead, p->ex_mem->MemWrite, p->ex_mem->Branch, p->ex_mem->jump,p->ex_mem->Overflow, p->ex_mem->zero);
	printf("Valor de ULASaida: %d \n Valor de rt: %d \n Endereço do branch: %d \n Endereço do Jump: %d \n", p->ex_mem->ula_resultado, p->ex_mem->rt_dado, p->ex_mem->branch_resultado, p->ex_mem->address);
	if(p->ex_mem->jump == 1){
		p->pc->pc_index = p->ex_mem->address;
	} else if (p->ex_mem->jump == 0){
		if(p->ex_mem->Branch == 1 && p->ex_mem->zero == 1){
			p->pc->pc_index = p->ex_mem->branch_resultado;
		} 
	}
	
	 //prepara os dados para a memória de dados
    data_mem_input.adress = p->ex_mem->ula_resultado;
    data_mem_input.write_mem = p->ex_mem->MemWrite;
    data_mem_input.read_mem =p->ex_mem->MemRead;
    data_mem_input.dado_escrever = p->ex_mem->rt_dado;

    //chama a memória de dados e envia os dados
    data_mem_output = ex_data_mem(data_mem_input, p->mem_data);
	
	// seta os valores do registrador de pipeline mem_wb
	set_mem_wb(p->mem_wb, p->ex_mem->instrucao, p->ex_mem->opcode, p->ex_mem->rd, p->ex_mem->RegWrite, p->ex_mem->Memtoreg, p->ex_mem->ula_resultado, data_mem_output.dado_lido);
}

void write_back(Pipeline *p){
	printf("----------- ESTAGIO DE WRITE BACK ------------  \n");
	printf("INSTRUÇÃO NO ESTAGIO DE WRITE BACK:  ");
	print_binary(p->mem_wb->instrucao);
	printf("   |   ");
	print_asm(decode(p->mem_wb->instrucao));
	printf("\n\n");
	   //"MUX" que determina o que vai pro banco
    if(p->mem_wb->Memtoreg == 0x1){
        p->regs_bank->in_regs.dado_escrever = p->mem_wb->saida_mem;
    } else {
        p->regs_bank->in_regs.dado_escrever = p->mem_wb->ula_resultado;
    }

    //chama a função dos registradores, só escreve se a flag de escrita for 1
    p->regs_bank->in_regs.reg_destino = p->mem_wb->rd;
    printf("RD: %d \n",  p->mem_wb->rd);
    p->regs_bank->in_regs.write_reg = p->mem_wb->RegWrite;
    ex_registers(p->regs_bank->in_regs, p->regs_bank);
}

int run_step(Pipeline *p){
    // size_t size = 0;
    //verifica se os componentes existem e se o pc aponta para uma instruçao
    if (!p || !p->pc || !p->mem_inst || !p->mem_inst->instrucao) return -1;

    // Base e tamanho efetivo da memoria de instrucoes carregada.
  /*
    size = (size_t)p->mem_inst->loaded_count;
    if (size == 0) {
        printf("Não há instruções carregadas na memória. Para rodar um programa, carregue instruções primeiro.\n");
        return 1;
    }
*/
    // Se o PC sair do range do vetor o programa termina
    // if ((size_t)p->pc->pc_index >= size) return 1;
   write_back(p);
   acesso_memoria(p);
   executar(p);
   decodificar(p); 
   buscar(p);
   
   
/*

    pc_atual = p->pc->pc_index;
    pc_inst = &instrucao[p->pc->pc_index];
    decoded_inst = decode(pc_inst->instr);

    // passa opcode e function para o controle, gera sinais
    in_controle.opcode = decoded_inst.opcode;
    in_controle.function = decoded_inst.funct;
    
    //sinais
    sinais = controle_sinais(in_controle);

    if(sinais.jump == 0 && sinais.Branch == 0){
        pc_step(p->pc);
    } else if(sinais.jump == 1) {
        pc_set(p->pc, (uint8_t)decoded_inst.address);
        sinais.jump = 0;
    }    

    //sinaliza qual é o rs
    p->regs_bank->in_regs.write_reg = sinais.RegWrite;
    p->regs_bank->in_regs.reg_base1 = decoded_inst.rs;
    p->regs_bank->in_regs.reg_base2 = decoded_inst.rt;

    //"MUX" que determina se o registrador de destino é rt ou rd
    if(sinais.RegDst == 0x1){
        p->regs_bank->in_regs.reg_destino = decoded_inst.rd;
    } else {
        p->regs_bank->in_regs.reg_destino = decoded_inst.rt;   
    }

    //pega os valores dos registradores base
    regs_output = ex_registers(p->regs_bank->in_regs, p->regs_bank);

    //prepara os dados para a ula(reg base1, reg base2 ou imm)

    ula_input.op1 = regs_output.val1;
    ula_input.ula_op = sinais.ULA_op;

    //"MUX" que determina se o segundo op da ula é reg base 2 ou imm
    if(sinais.ULASrc == 0x1){
        ula_input.op2 = decoded_inst.imm;
    } else {
        ula_input.op2 = regs_output.val2;
    }

    ula_output = ulaExecuta(&ula_input);
    
		if(ula_output.Overflow == 1){
				printf("\nOperação resultou em overflow. Atribuindo valor máximo de representação: %d", ula_output.resultado);
		}

    if(sinais.Branch == 0x1){
        if(ula_output.zero == 0x1){
            ula_input.op1 = p->pc->pc_index;
			ula_input.op2 = 1;
			ula_input.ula_op = 0x0;
			ula_output = ulaExecuta(&ula_input);
			ula_input.op1 = ula_output.resultado;
			ula_input.op2 =  decoded_inst.imm;
			ula_output = ulaExecuta(&ula_input);
			pc_set(p->pc, (uint8_t)ula_output.resultado);
        }else{
            pc_step(p->pc);
        }
        sinais.Branch = 0;
    }

    //prepara os dados para a memória de dados
    data_mem_input.adress = ula_output.resultado;
    data_mem_input.write_mem = sinais.MemWrite;
    data_mem_input.read_mem = sinais.MemRead;
    data_mem_input.dado_escrever = regs_output.val2;

    //chama a memória de dados e envia os dados
    data_mem_output = ex_data_mem(data_mem_input, p->mem_data);

    //"MUX" que determina o que vai pro banco
    if(sinais.Memtoreg == 0x1){
        p->regs_bank->in_regs.dado_escrever = data_mem_output.dado_lido;
    } else {
        p->regs_bank->in_regs.dado_escrever = ula_output.resultado;
    }
    //chama a função dos registradores, só escreve se a flag de escrita for 1
    ex_registers(p->regs_bank->in_regs, p->regs_bank);


    p->has_executed = 1;
    p->just_rewound = 0;

	// Saida
    //printf("INSTRUÇÃO 0x%02X NO ÍNDICE %d EXECUTADA!\n", pc_inst->instr, pc_atual);
    printf("\n");
    printf("Instrução executada: ");
    exibe1_asm(p->mem_inst, pc_atual);
    printf("\n");
    if(p->pc->pc_index == p->mem_inst->loaded_count){
		printf("Todas as instruções executadas.");
	}else{
		printf("Proxíma instrução: ");
		exibe1_asm(p->mem_inst, p->pc->pc_index);
	}
		printf("\n");
		printf("PC AGORA ESTÁ EM : %d \n", p->pc->pc_index);
	
*/
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
    p->pc->pc_index = 0;
    return status;
}

int run_back(Pipeline *p){
    (void)p;
    return -1;
}

void copiaSimulador (Pipeline* p_backup, Pipeline* p){
	 copiaBancoRegistradores(p_backup->regs_bank, p->regs_bank);
	 copiaMemoria(p_backup->mem_data, p->mem_data);
	 copiaPC(p_backup->pc,p->pc);
	 if(p->controle != NULL){
		 copiaControle(p_backup->controle, p->controle);
	 }
	 
	 
}
