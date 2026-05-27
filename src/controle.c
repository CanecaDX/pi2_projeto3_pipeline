#include "../include/multiciclo.h"

void controle_sinais(Multiciclo *m){
	m->controle->entrada.opcode = m->decoded_inst.opcode;
	m->controle->entrada.funct = m->decoded_inst.funct;

	switch(m->controle->states->ciclos){
		case 0: //busca
			m->controle->states->ac_state = 0;
			m->controle->states->next_state = 1;
			m->controle->sinais = (Sinais){0};
			m->controle->sinais.IResc = 0x1;
			m->controle->sinais.PCesc = 0x1;
			m->controle->sinais.ULAFonteB = 0x1;
			m->controle->sinais.ULA_op = 0x0; // soma para incremento do PC
			m->controle->states->ciclos++;
			break;
		case 1: //decodiicacao
			m->controle->states->ac_state = 1;
			switch(m->controle->entrada.opcode){
				case 0x0: // tipo r
					m->controle->states->next_state = 7;
					break;
				case 0xB: // lw
				case 0xF: // sw
				case 0x04: // addi
					m->controle->states->next_state = 2;
					break;
				case 0x8: // beq
					m->controle->states->next_state = 9;
					break;
				case 0x2: // j
					m->controle->states->next_state = 10;
					break;
			}
			m->controle->sinais = (Sinais){0};
			m->controle->sinais.ULAFonteB = 0x2;
			m->controle->sinais.RegDst = 0x1;
			m->controle->sinais.ULA_op = 0x0; // soma para cálculo de endereço/branch
			m->controle->states->ciclos++;
			break;
		case 2: //execuçao
			switch(m->controle->states->next_state){
				case 7: //tipo r
					m->controle->states->ac_state = 7;
					m->controle->states->next_state = 8;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.ULAFonteA = 1;
					m->controle->sinais.ULAFonteB = 00;
					m->controle->sinais.ULA_op = m->controle->entrada.funct; 
					m->controle->sinais.RegDst = 1;
					break;
					
				case 2: //tipo I
					m->controle->states->ac_state = 2;
					
					if(m->controle->entrada.opcode == 0xB){
						m->controle->states->next_state = 3;
					}
					else if(m->controle->entrada.opcode == 0xF){
						m->controle->states->next_state = 5;
					}
					else if(m->controle->entrada.opcode == 0x4){
						m->controle->states->next_state = 6;
					}
					
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.ULAFonteA = 1;
					m->controle->sinais.ULAFonteB = 10;
					m->controle->sinais.ULA_op = 0x0; 
					break;
				case 9: //beq
					m->controle->states->ac_state = 9;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.ULAFonteA = 1;
					m->controle->sinais.ULA_op = 0x2; 
					m->controle->sinais.Branch = 1;
					m->controle->sinais.PCsrc = 1;
					// m->controle->states->ciclos=0;
					break;
				case 10: //jump
					m->controle->states->ac_state = 10;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.PCesc = 1;
					m->controle->sinais.PCsrc = 0x2;
					m->controle->states->ciclos=0;
					break;
			}
			m->controle->states->ciclos++;
			break;
		case 3: //escrita no banco de registradores ou acesso a memoria
		//a partir desse ciclo, precisa fazer um switch ou ifelse com o estado anterior e atual, alem do opcode, pra determinar os sinais e próximo estado
			switch(m->controle->states->next_state){
				case 8: // tipo r
					m->controle->states->ac_state = 8;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.RegDst = 1;
					m->controle->sinais.RegWrite = 1;
					m->controle->sinais.Memtoreg = 0;	
					break; 
				case 3: //lw
					m->controle->states->ac_state = 3;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.MemRead = 1;
					m->controle->sinais.MemWrite = 0;
					m->controle->sinais.IouD = 1;	
					m->controle->states->next_state = 4;
					break;
				case 5: //sw
					m->controle->states->ac_state = 5;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.MemRead = 1;
					m->controle->sinais.MemWrite = 1;
					m->controle->sinais.IouD = 0;	
					break;
				case 6: //addi
					m->controle->states->ac_state = 6;
					m->controle->sinais = (Sinais){0};
					m->controle->sinais.MemWrite = 0;
					m->controle->sinais.RegWrite = 1;
					m->controle->sinais.RegDst = 0;
					m->controle->sinais.Memtoreg = 0;	
					break;
			}
			if (m->controle->states->next_state == 4) {
				m->controle->states->ciclos++;
			} else {
				m->controle->states->ciclos = 0;
			}
			break;
		case 4: //escrita do dado da memoria no banco de registradores (lw)
			m->controle->states->ac_state = 4;
			m->controle->sinais = (Sinais){0};
			m->controle->sinais.MemWrite = 0;
			m->controle->sinais.RegWrite = 1;
			m->controle->sinais.Memtoreg = 1;	
		
			m->controle->states->ciclos ++;
			break;
	}
/*
deixei aqui só pra gente não se perder com os opcodes
0x0 tipo r
0xB lw
0xF sw
0x4 addi
0x8 beq
0x2 j
*/

	return;
}

void copiaEntradaControle(Entrada_controle input_backup, Entrada_controle input){
 input_backup.opcode = input.opcode; 
 input_backup.funct = input.funct;
}
void copiaSinaisControle(Sinais sinais_backup, Sinais sinais){
	sinais_backup.RegDst = sinais.RegDst;
    sinais_backup.RegWrite = sinais.RegWrite;;
   	sinais_backup.Memtoreg = sinais.Memtoreg;
    sinais_backup.ULASrc = sinais.ULASrc;
    sinais_backup.MemRead = sinais.MemRead;
    sinais_backup.MemWrite = sinais.MemWrite;
    sinais_backup.Branch = sinais.Branch; 
    sinais_backup.jump = sinais.jump;
    sinais_backup.ULA_op = sinais.ULA_op;
    sinais_backup.IResc = sinais.IResc;
    sinais_backup.PCesc = sinais.PCesc;
    sinais_backup.PCsrc = sinais.PCsrc;
    sinais_backup.ULAFonteA = 	sinais.ULAFonteA;
    sinais_backup.ULAFonteB = sinais.ULAFonteB;
    sinais_backup.IouD = sinais.IouD;
 }

 void copiaStatesControle(FSM* states_backup , FSM* statess){
 	states_backup->ac_state = statess->ac_state;
    states_backup->ant_state = statess->ant_state ;
    states_backup->next_state = statess->next_state;
    states_backup->ciclos = statess->ciclos;
 }

void copiaControle(Controle* controle_backup, Controle* controle){

 copiaEntradaControle(controle_backup->entrada, controle->entrada);

 copiaSinaisControle(controle_backup->sinais, controle->sinais);

 copiaStatesControle(controle_backup->states, controle->states);
}  
