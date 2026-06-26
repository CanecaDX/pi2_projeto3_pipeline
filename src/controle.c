#include "controle.h"

Out_controle controle_sinais(In_controle in, int v){
	Out_controle out = {0};

	if(v == 0){
		return out;
	}

	switch (in.opcode) {
		case 0x0: // tipo r
			out.RegDst = 1;
			out.RegWrite = 1;
			out.ULASrc = 0;
			out.Memtoreg = 0;
			out.MemRead = 0;
			out.MemWrite = 0;
			out.Branch = 0;
			out.jump = 0;
			out.ULA_op = in.function;
			break;

		case 0xB: // lw
			out.RegDst = 0;
			out.RegWrite = 1;
			out.Memtoreg = 1;
			out.ULASrc = 1;
			out.MemRead = 1;
			out.MemWrite = 0;
			out.Branch = 0;
			out.jump = 0;
			out.ULA_op = 0; 
			break;

		case 0xF: // sw
			out.RegDst = 0;
			out.RegWrite = 0;
			out.Memtoreg = 0;
			out.ULASrc = 1;
			out.MemRead = 0;
			out.MemWrite = 1;
			out.Branch = 0;
			out.jump = 0;
			out.ULA_op = 0;
			break;

        case 0x4: //addi
			out.RegDst = 0;
			out.RegWrite = 1;
			out.Memtoreg = 0;
			out.ULASrc = 1;
			out.MemRead = 0;
			out.MemWrite = 0;
			out.Branch = 0;
			out.jump = 0;
			out.ULA_op = 0;
			break;

        case 0x8: //beq 
			out.RegDst = 0;
			out.RegWrite = 0;
			out.Memtoreg = 0;
			out.ULASrc = 0;
			out.MemRead = 0;
			out.MemWrite = 0;
			out.Branch = 1;
			out.jump = 0;
			out.ULA_op = 2; 
            break;

        case 0x2: //j
			out.RegDst = 0;
			out.RegWrite = 0;
			out.Memtoreg = 0;
			out.ULASrc = 0;
			out.MemRead = 0;
			out.MemWrite = 0;
			out.Branch = 0;
			out.jump = 1;
			out.ULA_op = 0;
            break;

		default:
			break;
	}

	return out;
}

void copiaEntradaControle(In_controle *input_backup, const In_controle *input){
 if (!input_backup || !input) return;
 *input_backup = *input;
}

void copiaSaidaControle(Out_controle *output_backup, const Out_controle *output){
 if (!output_backup || !output) return;
 *output_backup = *output;
}

void copiaControle(Controle* controle_backup, const Controle* controle){
 if (!controle_backup || !controle) return;
 copiaEntradaControle(&controle_backup->input, &controle->input);
 copiaSaidaControle(&controle_backup->output, &controle->output);

} 