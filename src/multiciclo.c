#include <stdlib.h>
#include <stdio.h>
#include "../include/multiciclo.h"

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

Multiciclo *multiciclo_create(void){
    Multiciclo *m = malloc(sizeof(Multiciclo));
    if (!m) return NULL;
	m->stats = stats_create();
    m->regs_bank = registers_create();
    m->memory = memory_create();
    m->pc = pc_create(0);
    m->controle = calloc(1, sizeof(Controle));
    if (!m->controle) return NULL;
    m->controle->states = calloc(1, sizeof(FSM));
    if (!m->controle->states) return NULL;
    m->decoded_inst = (Decoded){0};
    m->ula = calloc(1, sizeof(ULA));
    if (!m->ula) return NULL;

    return m;
}

void print_temp_regs(Multiciclo *m){
	char ir_buff[20] = {0};
	format_binary16(m->RI, ir_buff, sizeof(ir_buff));

	printf("\nREGISTRADORES INTERMEDIÁRIOS\n");
	printf("+-------------+--------------------+\n");
	printf("| Registrador | Valor              |\n");
	printf("+-------------+--------------------+\n");
	printf("| IR          | %-18s|\n", ir_buff);
	printf("| A           | %-18d |\n", m->A);
	printf("| B           | %-18d |\n", m->B);
	printf("| ULASaida    | M: %-3u  | C2: %-4d |\n", m->saidaULA, (int8_t)(m->saidaULA));
	printf("| RDM         | %-18d |\n", m->RDM);
	printf("+-------------+--------------------+\n");
}

void print_ex_info(Multiciclo *m, int pc, int state, int n_state){
	char ir_buff[20] = {0};
	format_binary16(m->RI, ir_buff, sizeof(ir_buff));
	
	printf("\nFSM E COMPONENTES SEQUENCIAIS AO FINAL DO CICLO\n");
	printf("+---------------+--------------------+\n");
	printf("| REG/FSM       | Valor              |\n");
	printf("+---------------+--------------------+\n");
	printf("| IR            | %-18s|\n", ir_buff);
	printf("| A             | %-18d |\n", m->A);
	printf("| B             | %-18d |\n", m->B);
	printf("| ULASaida      | M: %-3u  | C2: %-4d |\n", m->saidaULA, (int8_t)(m->saidaULA));
	printf("| RDM           | %-18d |\n", m->RDM);
	printf("| PC            | %-18d |\n", pc);
	printf("| ESTADO ATUAL  | %-18d |\n", state);
	printf("| PRÓXIMO ESTADO| %-18d |\n", n_state);
	printf("+---------------+--------------------+\n");
}

void print_ex_status(Multiciclo *m){
	printf("\nSTATUS DE EXECUÇÃO");
	printf("\nInstrução sendo executada: ");
	print_asm(decode(m->RI));
	printf("Ciclo executado: %d", m->controle->states->ciclos-1);
	printf("\nEstado executado: %d", m->controle->states->ac_state);
	printf("\nPróximo estado: %d", m->controle->states->next_state);
	printf("\nPróxima instrução a ser executada: ");
	print_asm(decode(m->memory->data[m->pc->pc_index].value));
}

void run_step(Multiciclo *m){
	switch(m->controle->states->ciclos){
		case 0 :
			//ciclos de busca e decodificaçao sempre serao os mesmos sinais
			m->controle->states->ciclos = 0;
			printf("\n===============================================\n");
			printf("\nCICLO %d - BUSCA", m->controle->states->ciclos);
			controle_sinais(m);
			m->RI = m->memory->data[m->pc->pc_index].value;
			printf("\nPC: %d\n", m->pc->pc_index);
			print_asm(decode(m->RI));

			m->ula->input.op1 = m->pc->pc_index;
			m->ula->input.op2 = 1;
			m->ula->input.ula_op = m->controle->sinais.ULA_op;

			m->ula->output = ulaExecuta(&m->ula->input);
			m->saidaULA = m->ula->output.resultado;

			m->pc->pc_index = m->ula->output.resultado; 
			m->stats->contCiclos++;
			print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
		break;
		
		case 1:
			//ciclo 1 - decodificação
			//ciclos de busca e decodificaçao sempre serao os mesmos sinais
			printf("\n===============================================\n");
			printf("\nCICLO %d - DECODIFICAÇÃO\n", m->controle->states->ciclos);
			m->decoded_inst = decode(m->RI);
			controle_sinais(m);
			char opcode_buff[8] = {0};
			binary_opcode(m->decoded_inst.opcode, opcode_buff, sizeof(opcode_buff));
			printf("\nINSTRUÇÃO DECODIFICADA: \nOPCODE = %s \nRS = $r%d \nRT = $r%d \nRD = $r%d \nOPERAÇÃO NA ULA = %s \nIMEDIATO = %d\n",
			opcode_buff, m->decoded_inst.rs, m->decoded_inst.rt, m->decoded_inst.rd, funct_to_string(m->decoded_inst.funct_op), m->decoded_inst.imm);			
			m->A = m->regs_bank->registradores[m->decoded_inst.rs];
			m->B = m->regs_bank->registradores[m->decoded_inst.rt];
			m->ula->input.op1 = m->pc->pc_index;
			m->ula->input.op2 = m->decoded_inst.imm;
			m->ula->input.ula_op = m->controle->sinais.ULA_op;
			m->ula->output = ulaExecuta(&m->ula->input);
			m->saidaULA = m->ula->output.resultado;
			m->stats->contCiclos++;
			print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
		break;
		
		case 2:
			//ciclo 2 - execução
			//MODIFICAR O CICLO DE EXEC PARA CHAMAR A ULA UMA UNICA VEZ
			//UNIFICAR CICLO DE EXECUÇAO DE TODOS OS TIPOS COMPARANDO OS SINAIS
					printf("\n===============================================\n");
					printf("\nCICLO %d - PROCESSAMENTO\n", m->controle->states->ciclos);
					controle_sinais(m);						
					//jump
					if (m->controle->sinais.PCesc == 1){
						m->stats->contInsEx++;
						m->stats->contCiclos++;
						programStat(m->memory, 0,m->stats, m);
						m->pc->pc_index = m->decoded_inst.imm & 0x3F;
						printf("\nPC: %d\n", m->pc->pc_index);
						m->controle->states->ciclos=0;
						print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
						break;
					}
					
					//MUXes da ULA
					if((m->controle->sinais.ULAFonteA == 1 && m->controle->sinais.ULAFonteB == 00) || (m->controle->sinais.Branch == 1)){ //tipo r e beq
						m->ula->input.op1 = m->A;
						m->ula->input.op2 = m->B;
					}
					else{
						if(m->controle->sinais.ULAFonteA == 1 && m->controle->sinais.ULAFonteB == 10){ //tipo i
							m->ula->input.op1 = m->A;
							m->ula->input.op2 = m->decoded_inst.imm;
						}	
					}
			
					m->ula->input.ula_op = m->controle->sinais.ULA_op;
					m->ula->output = ulaExecuta(&m->ula->input);
					
					printf("\nFLAG ULA SAÍDA ZERO: %d\n", m->ula->output.zero);
					if(m->ula->output.zero && m->controle->sinais.Branch){
						printf("\nCONDIÇÃO DE DESVIO SATISFEITA\n");
						m->stats->contInsEx++;
						m->stats->contCiclos++;
						programStat(m->memory, 0,m->stats, m);
						m->pc->pc_index = m->saidaULA;
						m->controle->states->ciclos=0;
						printf("\nPC APOS BEQ: %d\n", m->pc->pc_index);
						print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
						break;
					}			
					
					m->saidaULA = (uint8_t)(m->ula->output.resultado);
					//para o caso de ser um endereço saidaULA suporta uma magnitude
					m->stats->contCiclos++;
					print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
		break;
		
		case 3:
			//ciclo 3 - escrita no banco
			printf("\n===============================================\n");
			printf("\nCICLO %d - ESCRITA NO BANCO/ACESSO A MEMÓRIA\n", m->controle->states->ciclos);
			controle_sinais(m);
			//prepara os dados para a memória de dados
			if (m->controle->sinais.MemRead || m->controle->sinais.MemWrite) {
				m->memory->in_data.adress = m->saidaULA; //usa a magnitude para não ter a limitação do complemento de dois
			} 
			
			m->memory->in_data.write_mem = m->controle->sinais.MemWrite;
			m->memory->in_data.read_mem = m->controle->sinais.MemRead;
		  	m->memory->in_data.dado_escrever = m->regs_bank->registradores[m->decoded_inst.rt];

			//chama a memória de dados e envia os dados
			m->memory->out_data = ex_data_mem(m->memory->in_data, m->memory);

			
			//ajusta os sinais para a escrita no banco
			m->regs_bank->in_regs.write_reg = m->controle->sinais.RegWrite;
			
			if(m->controle->sinais.RegDst == 1){
				m->regs_bank->in_regs.reg_destino = m->decoded_inst.rd;
			}
			else if(m->controle->sinais.RegDst == 0){
				m->regs_bank->in_regs.reg_destino = m->decoded_inst.rt;
			}			
			
			if(m->controle->sinais.RegWrite == 1)
				   m->regs_bank->in_regs.dado_escrever = (int8_t)(m->saidaULA); //faz o cast para representar considerando complemento de dois
				   
			ex_registers(m->regs_bank->in_regs, m->regs_bank);
			

			if(!m->controle->sinais.RegDst){
				printf("\nVALOR %d ESCRITO NO REGISTRADOR $r%d", m->regs_bank->in_regs.dado_escrever, m->decoded_inst.rt);
			}
			else{
				printf("\nVALOR %d ESCRITO NO REGISTRADOR $r%d", m->regs_bank->in_regs.dado_escrever, m->decoded_inst.rd);
			}
			
			if(m->controle->states->ac_state == 5){
				printf("\nVALOR %d ESCRITO NO ENDEREÇO DE MEMÓRIA %d", m->memory->in_data.dado_escrever, m->memory->in_data.adress);
			}
			
			if(m->controle->states->ac_state == 3){
				printf("\nENDEREÇO %d ACESSADO NA MEMÓRIA", m->memory->in_data.adress);
			}
			
			m->RDM = m->memory->out_data.dado_lido;
			
				m->stats->contInsEx++;
				m->stats->contCiclos++;
				programStat(m->memory, 0,m->stats, m);
			if(m->controle->states->ciclos == 0)
				print_ex_info(m, m->pc->pc_index, 4, 0);
			else
				print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
			break;
		
		case 4: //escreve o valor da memória no banco de registradores
			printf("\n===============================================\n");
			printf("\nCICLO %d - ESCRITA DE RETORNO\n", m->controle->states->ciclos);
			controle_sinais(m);
			
			if(m->controle->sinais.Memtoreg == 1){
				m->regs_bank->in_regs.dado_escrever = m->RDM;			
			}
			m->regs_bank->in_regs.write_reg = m->controle->sinais.RegWrite;
			m->regs_bank->in_regs.reg_destino = m->decoded_inst.rt;
			ex_registers(m->regs_bank->in_regs, m->regs_bank);
						
			printf("\nVALOR DO ENDEREÇO %d ESCRITO NO REGISTRADOR $r%d.\n", m->memory->in_data.adress, m->decoded_inst.rt);
			m->stats->contInsEx++;
			m->stats->contCiclos++;
			programStat(m->memory, 0,m->stats, m);
			print_ex_info(m, m->pc->pc_index, m->controle->states->ac_state, m->controle->states->next_state);
			m->controle->states->ciclos = 0;


		break;	
	}
}

void run(Multiciclo *m){
    if (m->memory->inst_loaded_count == 0) {
        printf("Não há instruções carregadas na memória. Para rodar um programa, carregue instruções primeiro.\n");
        return;
    }
    while (m->pc->pc_index <= m->memory->inst_loaded_count) {
        run_step(m);
    }
    printf("Programa finalizado!");
	reset_run(m);
	return;
}

void reset_run(Multiciclo *m){
	//reseta o valor do pc
	m->pc->pc_index = 0;
	//reseta o banco de registradores
	for (int i =  0; i < 8; i++){
		m->regs_bank->registradores[i] = 0;
	}
	//reseta ciclos, estados e sinais do controle
	m->controle->states->ciclos = 0;
	m->controle->states->ac_state = 0;
	m->controle->states->next_state = 0;
	m->controle->states->ant_state = 0;
	m->controle->sinais = (Sinais){0};
}

void reset_all(Multiciclo *m){
	reset_run(m);
	// para a opção do usuário ele reseta todas as estruturas
	//memória

	for(int i = 0; i <256; i++){
		m->memory->data[i].value = 0;
	}
	m->memory->inst_loaded_count = 0;
	m->memory->data_loaded_count = 0;
}

void copiaStats(Stats *stats_b, Stats * stats){
	stats_b->arit = stats->arit;
	stats_b->im = stats->im;
	stats_b->r = stats->r;
	stats_b->j = stats->j;
	stats_b->desC = stats->desC;
	stats_b->mem_d = stats->mem_d;
	stats_b->contInsEx = stats->contInsEx;
	stats_b->contCiclos = stats->contCiclos;
	stats_b->cpi = stats->cpi;
}

void copiaSimulador(Multiciclo* m_backup, Multiciclo* m){
	m_backup->A = m->A;
	m_backup->B = m->B;
	m_backup->RI = m->RI;
	m_backup->RDM = m->RDM;
	m_backup->saidaULA = m->saidaULA;
	copiaStats(m_backup->stats, m->stats);
	copiaBancoRegistradores(m_backup->regs_bank, m->regs_bank);
	copiaMemoria(m_backup->memory, m->memory);
	copiaPC(m_backup->pc,m->pc);
	if(m->controle != NULL){
		copiaControle(m_backup->controle, m->controle);
	}
	 
	 
}

void empilhar(No **topo, Multiciclo* multiciclo) {
    No *novoNo = (No *)malloc(sizeof(No));
    novoNo->multiciclo = multiciclo_create();
    if (novoNo == NULL) {
        printf("Erro de alocação de memória.\n");
        exit(1);
    }
    copiaSimulador(novoNo->multiciclo, multiciclo);
    novoNo->proximo = *topo;
    *topo = novoNo;
    
}

void desempilhar(No **topo, Multiciclo* multiciclo) {
    if (*topo == NULL) {
        printf("A pilha está vazia.\n");
         // Valor indicativo de erro
    }
    No *temp = *topo;
   	copiaSimulador(multiciclo, temp->multiciclo);
    
    *topo = temp->proximo;
    free(temp);
}

//função que atualiaza valores das stats
void programStat(Memory *mem, int cond, Stats *stats, Multiciclo *m){
	
	int count = mem ? mem->inst_loaded_count : 0;
	
	if(cond != 12){
			Data raw = mem->data[m->pc->pc_index];
			Decoded d = decode(raw.value);
			if(d.type == TYPE_R){
				stats->r++;
			}else if(d.type == TYPE_I){
				stats->im++;
			}else{
				stats->j++;	
			}	
				
			if(d.type == TYPE_I && (d.opcode == 0xB || d.opcode == 0xF))
				stats->mem_d++;
					
			if(d.type == TYPE_I && d.opcode == 0x8)
				stats->desC++;
					
			if((d.type == TYPE_R || d.type == TYPE_I) && (d.opcode != 0x8 && d.opcode != 0xB && d.opcode != 0xF))
				stats->arit++;
			
			m->stats->cpi = m->stats->contCiclos / m->stats->contInsEx;		
	}				
	if(cond == 12){ //exibe stats
			printf("\n%d instruções carregadas\n", count);	
			printf("\nInstruções Executadas: %d\n", m->stats->contInsEx);
			printf("\nCPI MÉDIO: %d\n", m->stats->cpi);
			printf("\nTIPOS");
			printf("\n%d instruções do tipo R", stats->r);
			printf("\n%d instruções do tipo I", stats->im);
			printf("\n%d instruções do tipo J", stats->j);
			printf("\n");
			printf("\nCLASSES");
			printf("\n%d operações de lógica e aritimética;", stats->arit);
			printf("\n%d operações de desvio incodicional;", stats->j);
			printf("\n%d operações de desvio condicional;", stats->desC);
			printf("\n%d operações de transferência de dados.", stats->mem_d);
	}
}
