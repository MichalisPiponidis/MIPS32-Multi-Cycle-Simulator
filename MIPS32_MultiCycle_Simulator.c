#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#define MAX_WORD_SIZE 500
#define MAX_INSTRUCTIONS 1000
#define MAX_INSTRUCTION_LENGTH 500
#define MAX_LABELS 1000
#define MAX_MEMORY_SIZE 2000
#define MAX_REG_LENGTH 150

typedef struct {
	int address;
	int contents;
}memory_slot;

typedef struct {
	char name[MAX_WORD_SIZE];
	int instruction;
}label;

//Control Signals
static int memread, alusrca, iord, irwrite, alusrcb, aluop, pcwrite, pcsource, pcwritecond, memwrite, memtoreg, regwrite, regdst;
//Monitors
static int pc_monitor, write_data_reg, read_data1, read_data2, address_monitor, write_data_memory, jump, branch, immediate_monitor, memory_data_reg, a, b, alu_in1, alu_in2;
static char alu_out[MAX_INSTRUCTION_LENGTH], alu_out_reg[MAX_INSTRUCTION_LENGTH], memory_out[MAX_INSTRUCTION_LENGTH], opcode_monitor[10], rs_monitor[MAX_REG_LENGTH], rt_monitor[MAX_REG_LENGTH], read_reg1[MAX_REG_LENGTH], read_reg2[MAX_REG_LENGTH], write_reg[MAX_REG_LENGTH];
//Registers
static int zero, pc, r0, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra;
//Variables
static int i, k, current_state = 0, instruction_counter = -1, label_counter = -1, current_instruction = 0, end = 0, cycle1, cycle2, branch_instruction, immediate, memory_slots_used = -1, current_cycle;
static char ch, word[MAX_WORD_SIZE], instructions[MAX_INSTRUCTIONS][MAX_INSTRUCTION_LENGTH], opcode[10], rs[MAX_REG_LENGTH], rt[MAX_REG_LENGTH], rd[MAX_REG_LENGTH], branch_label[MAX_WORD_SIZE];
static label labels[MAX_LABELS];
static memory_slot memory[MAX_MEMORY_SIZE];
FILE* output, *filepointer;
time_t start, endt;

int add(int a, int b) {
	return (a + b);
}

int addi(int a, int immediate) {
	return (a + immediate);
}

int addiu(int a, int immediate) {
	return (a + abs(immediate));
}

int addu(int a, int b) {
	return (a + abs(b));
}

int and (int a, int b) {
	return (a & b);
}

int andi (int a, int immediate) {
	return (a & immediate);
}

bool beq(int a, int b) {
	if (a == b)
		return true;
	else
		return false;
}

bool bne(int a, int b) {
	if (a != b)
		return true;
	else
		return false;
}

int j(int address) {
	return (address);
}

long lw(int address, int offset) {
	address = address + offset;
	int o;
	for (o = 0; o <= memory_slots_used; o++) {
		if (memory[o].address == address) {
			return memory[o].contents;
		}
	}
}

int nor(int a, int b) {
	return (~(a | b));
}

int or(int a, int b) {
	return (a | b);
}

int ori (int a, int immediate) {
	return (a | immediate);
}

int slt(int a, int b) {
	if (a < b)
		return 1;
	else
		return 0;
}

int slti(int a, int immediate) {
	if (a < immediate)
		return 1;
	else
		return 0;
}

int sltiu(int a, int immediate) {
	if (a < abs(immediate))
		return 1;
	else
		return 0;
}

int sltu(int a, int b) {
	if (a < abs(b))
		return 1;
	else
		return 0;
}

int sll(int a, int amount) {
	return (a << amount);
}

int srl(int a, int amount) {
	return (a >> amount);
}

void sw(int value, int address, int offset) {
	address = address + offset;
	int p, found=0;
	for (p = 0; p <= memory_slots_used; p++) {
		if (memory[p].address == address) {
			found = 1;
			memory[p].contents = value;
			break;
		}
	}
	if (found == 0) {
		memory_slots_used++;
		memory[memory_slots_used].address = address;
		memory[memory_slots_used].contents = value;
	}
}

int sub(int a, int b) {
	return (a - b);
}

int subu(int a, int b) {
	return (a - abs(b));
}

void swap(int a, int b) { //xrisimopiite sto sortmemory
	memory_slot temp;
	temp.address = memory[a].address;
	temp.contents = memory[a].contents;
	memory[a].address = memory[b].address;
	memory[a].contents = memory[b].contents;
	memory[b].address = temp.address;
	memory[b].contents = temp.contents;
}

void sortmemory() {
	bool swapped=true;
	int u;
	while (swapped == true) {
		swapped = false;
		for (u = 0; i <= memory_slots_used; u++) {
			if (memory[u].address > memory[u + 1].address) {
				swap(u, u + 1);
				swapped = true;
			}
		}
	}
}

int register_decode(char reg[MAX_REG_LENGTH]) {
	if (strcmp(reg, "$zero") == 0) return 0;
	else if (strcmp(reg, "$pc") == 0) return 1;
	else if (strcmp(reg, "$r0") == 0) return 2;
	else if (strcmp(reg, "$at") == 0) return 3;
	else if (strcmp(reg, "$v0") == 0) return 4;
	else if (strcmp(reg, "$v1") == 0) return 5;
	else if (strcmp(reg, "$a0") == 0) return 6;
	else if (strcmp(reg, "$a1") == 0) return 7;
	else if (strcmp(reg, "$a2") == 0) return 8;
	else if (strcmp(reg, "$a3") == 0) return 9;
	else if (strcmp(reg, "$t0") == 0) return 10;
	else if (strcmp(reg, "$t1") == 0) return 11;
	else if (strcmp(reg, "$t2") == 0) return 12;
	else if (strcmp(reg, "$t3") == 0) return 13;
	else if (strcmp(reg, "$t4") == 0) return 14;
	else if (strcmp(reg, "$t5") == 0) return 15;
	else if (strcmp(reg, "$t6") == 0) return 16;
	else if (strcmp(reg, "$t7") == 0) return 17;
	else if (strcmp(reg, "$s0") == 0) return 18;
	else if (strcmp(reg, "$s1") == 0) return 19;
	else if (strcmp(reg, "$s2") == 0) return 20;
	else if (strcmp(reg, "$s3") == 0) return 21;
	else if (strcmp(reg, "$s4") == 0) return 22;
	else if (strcmp(reg, "$s5") == 0) return 23;
	else if (strcmp(reg, "$s6") == 0) return 24;
	else if (strcmp(reg, "$s7") == 0) return 25;
	else if (strcmp(reg, "$t8") == 0) return 26;
	else if (strcmp(reg, "$t9") == 0) return 27;
	else if (strcmp(reg, "$k0") == 0) return 28;
	else if (strcmp(reg, "$k1") == 0) return 29;
	else if (strcmp(reg, "$gp") == 0) return 30;
	else if (strcmp(reg, "$sp") == 0) return 31;
	else if (strcmp(reg, "$fp") == 0) return 32;
	else if (strcmp(reg, "$ra") == 0) return 33;
}

int register_read(char reg[MAX_REG_LENGTH]) {
	if (strcmp(reg, "$zero") == 0) return zero;
	else if (strcmp(reg, "$pc") == 0) return pc;
	else if (strcmp(reg, "$r0") == 0) return r0;
	else if (strcmp(reg, "$at") == 0) return at;
	else if (strcmp(reg, "$v0") == 0) return v0;
	else if (strcmp(reg, "$v1") == 0) return v1;
	else if (strcmp(reg, "$a0") == 0) return a0;
	else if (strcmp(reg, "$a1") == 0) return a1;
	else if (strcmp(reg, "$a2") == 0) return a2;
	else if (strcmp(reg, "$a3") == 0) return a3;
	else if (strcmp(reg, "$t0") == 0) return t0;
	else if (strcmp(reg, "$t1") == 0) return t1;
	else if (strcmp(reg, "$t2") == 0) return t2;
	else if (strcmp(reg, "$t3") == 0) return t3;
	else if (strcmp(reg, "$t4") == 0) return t4;
	else if (strcmp(reg, "$t5") == 0) return t5;
	else if (strcmp(reg, "$t6") == 0) return t6;
	else if (strcmp(reg, "$t7") == 0) return t7;
	else if (strcmp(reg, "$s0") == 0) return s0;
	else if (strcmp(reg, "$s1") == 0) return s1;
	else if (strcmp(reg, "$s2") == 0) return s2;
	else if (strcmp(reg, "$s3") == 0) return s3;
	else if (strcmp(reg, "$s4") == 0) return s4;
	else if (strcmp(reg, "$s5") == 0) return s5;
	else if (strcmp(reg, "$s6") == 0) return s6;
	else if (strcmp(reg, "$s7") == 0) return s7;
	else if (strcmp(reg, "$t8") == 0) return t8;
	else if (strcmp(reg, "$t9") == 0) return t9;
	else if (strcmp(reg, "$k0") == 0) return k0;
	else if (strcmp(reg, "$k1") == 0) return k1;
	else if (strcmp(reg, "$gp") == 0) return gp;
	else if (strcmp(reg, "$sp") == 0) return sp;
	else if (strcmp(reg, "$fp") == 0) return fp;
	else if (strcmp(reg, "$ra") == 0) return ra;
}

void register_write(char reg[MAX_REG_LENGTH], int value) {
	if (strcmp(reg, "$zero") == 0) zero = value;
	else if (strcmp(reg, "$pc") == 0) pc = value;
	else if (strcmp(reg, "$r0") == 0) r0 = value;
	else if (strcmp(reg, "$at") == 0) at = value;
	else if (strcmp(reg, "$v0") == 0) v0 = value;
	else if (strcmp(reg, "$v1") == 0) v1 = value;
	else if (strcmp(reg, "$a0") == 0) a0 = value;
	else if (strcmp(reg, "$a1") == 0) a1 = value;
	else if (strcmp(reg, "$a2") == 0) a2 = value;
	else if (strcmp(reg, "$a3") == 0) a3 = value;
	else if (strcmp(reg, "$t0") == 0) t0 = value;
	else if (strcmp(reg, "$t1") == 0) t1 = value;
	else if (strcmp(reg, "$t2") == 0) t2 = value;
	else if (strcmp(reg, "$t3") == 0) t3 = value;
	else if (strcmp(reg, "$t4") == 0) t4 = value;
	else if (strcmp(reg, "$t5") == 0) t5 = value;
	else if (strcmp(reg, "$t6") == 0) t6 = value;
	else if (strcmp(reg, "$t7") == 0) t7 = value;
	else if (strcmp(reg, "$s0") == 0) s0 = value;
	else if (strcmp(reg, "$s1") == 0) s1 = value;
	else if (strcmp(reg, "$s2") == 0) s2 = value;
	else if (strcmp(reg, "$s3") == 0) s3 = value;
	else if (strcmp(reg, "$s4") == 0) s4 = value;
	else if (strcmp(reg, "$s5") == 0) s5 = value;
	else if (strcmp(reg, "$s6") == 0) s6 = value;
	else if (strcmp(reg, "$s7") == 0) s7 = value;
	else if (strcmp(reg, "$t8") == 0) t8 = value;
	else if (strcmp(reg, "$t9") == 0) t9 = value;
	else if (strcmp(reg, "$k0") == 0) k0 = value;
	else if (strcmp(reg, "$k1") == 0) k1 = value;
	else if (strcmp(reg, "$gp") == 0) gp = value;
	else if (strcmp(reg, "$sp") == 0) sp = value;
	else if (strcmp(reg, "$fp") == 0) fp = value;
	else if (strcmp(reg, "$ra") == 0) ra = value;
}

int hexToDec(char hexVal[]){
	int len = strlen(hexVal);
	// Initializing base value to 1, i.e 16^0 
	int base = 1;
	int dec_val = 0;
	// Extracting characters as digits from last character 
	for (int i = len - 1; i >= 0; i--){
		// if character lies in '0'-'9', converting  
		// it to integral 0-9 by subtracting 48 from 
		// ASCII value. 
		if (hexVal[i] >= '0' && hexVal[i] <= '9'){
			dec_val += (hexVal[i] - 48) * base;
			// incrementing base by power 
			base = base * 16;
		}
		// if character lies in 'A'-'F' , converting  
		// it to integral 10 - 15 by subtracting 55  
		// from ASCII value 
		else if (hexVal[i] >= 'A' && hexVal[i] <= 'F'){
			dec_val += (hexVal[i] - 55) * base;
			// incrementing base by power 
			base = base * 16;
		}
		else if (hexVal[i] >= 'a' && hexVal[i] <= 'f') {
			dec_val += (hexVal[i] - 87) * base;
			// incrementing base by power 
			base = base * 16;
		}
	}
	return dec_val;
}

int label_decode(char labell[MAX_WORD_SIZE]) {
	for (int k = 0; k < MAX_LABELS; k++) {
		if (strcmp(labels[k].name, labell) == 0) {
			return labels[k].instruction;
			break;
		}
	}
	printf("\nERROR: Wrong Label (%s)\n",labell);
}

void print_registers_hex() {
	printf("\nREGISTERS: (Hex)\n\n");
	printf("\nPC= %X\tr0= %X\tat= %X\tv0= %X\tv1= %X\ta0= %X\t", pc, r0, at, v0, v1, a0);
	printf("\na1= %X\ta2= %X\ta3= %X\tt0= %X\tt1= %X\tt2= %X\t", a1, a2, a3, t0, t1, t2);
	printf("\nt3= %X\tt4= %X\tt5= %X\tt6= %X\tt7= %X\ts0= %X\t", t3, t4, t5, t6, t7, s0);
	printf("\ns1= %X\ts2= %X\ts3= %X\ts4= %X\ts5= %X\ts6= %X\t", s1, s2, s3, s4, s5, s6);
	printf("\ns7= %X\tt8= %X\tt9= %X\tk0= %X\tk1= %X\tgp= %X\t", s7, t8, t9, k0, k1, gp);
	printf("\nsp= %X\tfp= %X\tra= %X", sp, fp, ra);
}

void print_registers_dec() {
	printf("\nREGISTERS: (Dec)\n\n");
	printf("\nPC= %d\tr0= %d\tat= %d\tv0= %d\tv1= %d\ta0= %d\t", pc, r0, at, v0, v1, a0);
	printf("\na1= %d\ta2= %d\ta3= %d\tt0= %d\tt1= %d\tt2= %d\t", a1, a2, a3, t0, t1, t2);
	printf("\nt3= %d\tt4= %d\tt5= %d\tt6= %d\tt7= %d\ts0= %d\t", t3, t4, t5, t6, t7, s0);
	printf("\ns1= %d\ts2= %d\ts3= %d\ts4= %d\ts5= %d\ts6= %d\t", s1, s2, s3, s4, s5, s6);
	printf("\ns7= %d\tt8= %d\tt9= %d\tk0= %d\tk1= %d\tgp= %d\t", s7, t8, t9, k0, k1, gp);
	printf("\nsp= %d\tfp= %d\tra= %d", sp, fp, ra);
}

void print_memory() {
	printf("\t\tMEMORY\n\n");
	for (int w = 0; w <= memory_slots_used; w++) {
		printf("Address: %X\t\tContents (Decimal): %d\tContents (Hex): %X\n", memory[w].address, memory[w].contents, memory[w].contents);
	}
}

void export_cycle_info(int cycle) {
	//sortmemory();
	fprintf(output, "-----Cycle %d-----\nRegisters:\n", cycle);
	fprintf(output, "%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x", pc, r0, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra);
	fprintf(output, "\n\nMonitors:\n");
	fprintf(output, "%x\t", pc_monitor);
	if (address_monitor==INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", address_monitor);
	if (write_data_memory == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", write_data_memory);
	fprintf(output, "%s\t%s\t%s\t%s\t", memory_out, opcode_monitor, rs_monitor, rt_monitor);
	if (immediate_monitor == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", immediate_monitor);
	if (memory_data_reg == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", memory_data_reg);
	fprintf(output, "%s\t%s\t%s\t", read_reg1, read_reg2, write_reg);
	if (write_data_reg == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", write_data_reg);
	if (read_data1 == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", read_data1);
	if (read_data2 == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", read_data2);
	if (a == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", a);
	if (b == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", b);
	if (alu_in1 == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", alu_in1);
	if (alu_in2 == INT_MAX)
		fprintf(output, "-\t");
	else
		fprintf(output, "%x\t", alu_in2);
	fprintf(output, "%s\t%s", alu_out, alu_out_reg);
	// Control Signals
	fprintf(output, "\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%02d\t%02d\t%02d\t%d\t%d\t%d", pcwritecond, pcwrite, iord, memread, memwrite, memtoreg, irwrite, pcsource, aluop, alusrcb, alusrca, regwrite, regdst);
	fprintf(output, "\n\nMemory State:\n");
	for (int r = 0; r <= memory_slots_used; r++) {
		if (r != memory_slots_used) //gia na min tiponete extra tab sto telos tin grammis
			fprintf(output, "%x\t", memory[r].contents);
		else
			fprintf(output, "%x", memory[r].contents);
	}
	fprintf(output, "\n\n");
}

void export_final_info() {
	//sortmemory();
	fprintf(output, "-----Final State-----\nRegisters:\n");
	fprintf(output, "%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x", pc, r0, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra);
	fprintf(output, "\n\nMemory State:\n");
	for (int r = 0; r <= memory_slots_used; r++) {
		if (r != memory_slots_used) //gia na min tiponete extra tab sto telos tin grammis
			fprintf(output, "%x\t", memory[r].contents);
		else
			fprintf(output, "%x", memory[r].contents);
	}
	fprintf(output, "\n\nTotal Cycles:\n%d",current_cycle);
	fprintf(output, "\n\nTotal Execution Time:\n%.0f", (float)((float)(endt - start) / CLOCKS_PER_SEC)*1000000000);
	fclose(output);
}

void parser() {
	//Reading the file (Parser)
	while ((ch = getc(filepointer)) != EOF) {
		switch (ch) {
		case '.':
			fscanf(filepointer, "%s", word);
			if (strcmp(word, "data") == 0) { //memory begins here
				break;
			}
			if (strcmp(word, "text") == 0) { //code begins here
				fscanf(filepointer, "%s", word);
				while (word != EOF) {
					switch (word[0]) {
					case '\n':
						fscanf(filepointer, "%s", word);
						break;
					case '#': //ignore comment line
						while ((ch = getc(filepointer)) != '\n') {

						}
						fscanf(filepointer, "%s", word);
						break;
					default: //this line contains "main:", a comment or a label.
						if (word[0] == '#') { //ignore comment line

							break;
						}
						if (strchr(word, ':') != NULL) { //if this is true, it's a label
							label_counter++;
							labels[label_counter].instruction = instruction_counter + 1;
							i = 0;
							while (word[i] != ':') { //copy the label name
								labels[label_counter].name[i] = word[i];
								i++;
							}
							labels[label_counter].name[i] = '\0';
							printf("\n\nLabel %d is %s and points to instruction %d.\n", label_counter, labels[label_counter].name, labels[label_counter].instruction);
							fscanf(filepointer, "%s", word);
							break;
						}
						//else, it's an instruction
						i = 0;
						instruction_counter++;
						while (word[i] != '\0') { //copy the first word of the instruction
							instructions[instruction_counter][i] = word[i];
							i++;
						}
						while ((ch = getc(filepointer)) != '\n') {
							if ((ch == '#') || (ch == '\t')) { //if we find a comment, ignore it
								while (((ch = getc(filepointer)) != '\n') && (ch != EOF)) {
								}
								break;
							}
							if (ch == EOF) { //if we find the end of file
								break;
							}
							instructions[instruction_counter][i] = ch;
							i++;
						}
						instructions[instruction_counter][i] = '\0';
						printf("\nInstruction %d is: %s", instruction_counter, instructions[instruction_counter]);
						fscanf(filepointer, "%s", word);
						break;
					}
					if (ch == EOF) { //if we find the end of file
						break;
					}
				}
				if (ch == EOF) { //if we find the end of file
					break;
				}
			}
			break;
		case '#': //ignore comment line
			while ((ch = getc(filepointer)) != '\n') {

			}
			break;
		case ('\t' || ' '):
			ch = getc(filepointer);
			break;
		}
		if (ch == EOF) { //if we find the end of file
			break;
		}
	}
	fclose(filepointer);
}

void decode_r_type() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rd
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rd
		rd[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rd[k] = '\0';
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rs
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rs
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rt
		i++;
	}
	k = 0;

	while ((instructions[current_instruction][i] != '\0') && (instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t')) { //diavazume ton rt
		rt[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rt[k] = '\0';
}

void decode_i_type() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rs
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rs
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rt
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rt
		rt[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rt[k] = '\0';
	while ((instructions[current_instruction][i] == ' ') || (instructions[current_instruction][i] == ',')) { //mexri na ftasoume sto immediate
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t') && (instructions[current_instruction][i] != '\0')) { //diavazume to immediate
		rd[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rd[k] = '\0';
	if ((rd[0] == '0') && (rd[1] == 'x'))
		immediate = hexToDec(rd);
	else
		immediate = atoi(rd);
}

void decode_branch() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rs
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rs
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rt
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rt
		rt[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rt[k] = '\0';
	while ((instructions[current_instruction][i] == ' ') || (instructions[current_instruction][i] == ',')) { //mexri na ftasoume sto label
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != '\0') && (instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t') && (instructions[current_instruction][i] != '\0')) { //diavazume to label
		word[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	word[k] = '\0';
}

void decode_jump() {
	while ((instructions[current_instruction][i] == ' ') || (instructions[current_instruction][i] == ',')) { //mexri na ftasoume sto label
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != '\0') && (instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t')) { //diavazume to label
		word[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	word[k] = '\0';
}

void decode_jr() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume sto rs
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != '\0') && (instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t')) { //diavazume to rs
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
}

void decode_memory() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rt
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rt
		rt[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rt[k] = '\0';
	while ((instructions[current_instruction][i] == ' ') || (instructions[current_instruction][i] == ',')) { //mexri na ftasoume sto offset
		i++;
	}
	k = 0;
	while (instructions[current_instruction][i] != '(') { //diavazume to offset
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
	if ((rs[0] == '0') && (rs[1] == 'x'))
		immediate = hexToDec(rs); //offset
	else
		immediate = atoi(rs); //offset
	i++; //gia na ftasoume sto base
	k = 0;
	while (instructions[current_instruction][i] != ')') { //diavazume to base
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
}

void decode_lui() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rt
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rt
		rt[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rt[k] = '\0';
	while ((instructions[current_instruction][i] == ' ') || (instructions[current_instruction][i] == ',')) { //mexri na ftasoume sto immediate
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t') && (instructions[current_instruction][i] != '\0')) { //diavazume to immediate
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
	if ((rs[0] == '0') && (rs[1] == 'x'))
		immediate = hexToDec(rs); //offset
	else
		immediate = atoi(rs); //offset
}

void decode_shift() {
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rd
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rd
		rd[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rd[k] = '\0';
	while (instructions[current_instruction][i] != '$') { //mexri na ftasoume ston rt
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != ',')) { //diavazume ton rt
		rt[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rt[k] = '\0';
	while ((instructions[current_instruction][i] == ' ') || (instructions[current_instruction][i] == ',')) { //mexri na ftasoume sto shift amount
		i++;
	}
	k = 0;
	while ((instructions[current_instruction][i] != '\0') && (instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\n') && (instructions[current_instruction][i] != '\t') && (instructions[current_instruction][i] != '\0')) { //diavazume to shift amount
		rs[k] = instructions[current_instruction][i];
		i++;
		k++;
	}
	rs[k] = '\0';
	if ((rs[0] == '0') && (rs[1] == 'x'))
		immediate = hexToDec(rs);
	else
		immediate = atoi(rs);
}

void check_cycle_print() {
	if ((current_cycle == cycle1) || (current_cycle == cycle2)) {
		export_cycle_info(current_cycle);
	}
}

void control(int type) { //type: 1=load 2=store 3=r-type 4=branch 5=jump 6=i-type 7=jal 8=jr
	current_cycle++;
	printf("\nCurrent State: %d at Cycle: %d", current_state, current_cycle);
	switch (current_state) {
	case 0: // Cycle 1
		//Monitors
		pc_monitor = pc;
		pc = pc + 4;
		address_monitor = pc_monitor;
		write_data_memory = INT_MAX; // INT_MAX = "-" meta sto tipoma
		strcpy(memory_out, instructions[current_instruction]);
		strcpy(opcode_monitor, "-");
		strcpy(rs_monitor, "-");
		strcpy(rt_monitor, "-");
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, "-");
		strcpy(read_reg2, "-");
		strcpy(write_reg, "-");
		write_data_reg = INT_MAX;
		read_data1 = INT_MAX;
		read_data2 = INT_MAX;
		a = INT_MAX;
		b = INT_MAX;
		alu_in1 = pc_monitor;
		alu_in2 = 4;
		sprintf(alu_out, "%x", pc);
		sprintf(alu_out_reg, "%x", pc);
		//Control Signals
		memread = 1;
		alusrca = 0;
		iord = 0;
		irwrite = 1;
		alusrcb = 01;
		aluop = 00;
		pcwrite = 1;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state++;
		break;
	case 1: // Cycle 2
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		strcpy(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		if (type == 6) {
			strcpy(read_reg2, "-");
			strcpy(write_reg, rt);
		}
		else {
			strcpy(read_reg2, rt);
			strcpy(write_reg, rd);
		}
		write_data_reg = INT_MAX;
		if (strcmp(rs, "-") == 0) { //an exume shift i j
			read_data1 = INT_MAX;
			a = INT_MAX;
		}
		else {
			read_data1 = register_read(rs);
			a = read_data1;
		}
		if (strcmp(rt, "-") == 0 || strcmp(opcode, "lui") == 0) { //an exume j
			read_data2 = INT_MAX;
			b = INT_MAX;
		}
		else {
			read_data2 = register_read(rt);
			b = read_data1;
		}
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		if (type==4) {
			strcpy(alu_out, branch_label);
			strcpy(alu_out_reg, branch_label);
		}
		else {
			strcpy(alu_out, "-");
			strcpy(alu_out_reg, "-");
		}
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 0;
		irwrite = 0;
		alusrcb = 11;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		switch (type) {
		case 1:
			current_state = 2;
			break;
		case 2:
			current_state = 2;
			break;
		case 3:
			current_state = 6;
			break;
		case 4:
			current_state = 8;
			break;
		case 5:
			current_state = 9;
			break;
		case 6:
			current_state = 10;
			break;
		case 7:
			current_state = 12;
			break;
		case 8:
			current_state = 13;
			break;
		}
		break;
	case 2: // lw-sw Cycle 3
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		strcpy(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, "-");
		strcpy(write_reg, rt);
		write_data_reg = INT_MAX;
		read_data1 = register_read(rs);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = a;
		alu_in2 = immediate;
		sprintf(alu_out, "%x", alu_in1 + alu_in2);
		sprintf(alu_out_reg, "%x", alu_in1 + alu_in2);
		//Control Signals
		memread = 0;
		alusrca = 1;
		iord = 0;
		irwrite = 0;
		alusrcb = 10;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		switch (type) {
		case 1:
			current_state = 3;
			break;
		case 2:
			current_state = 5;
			break;
		}
		break;
	case 3: // lw Cycle 4
		//Monitors
		pc_monitor = pc;
		address_monitor = alu_in1+alu_in2;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "%x", lw(address_monitor, 0));
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = lw(address_monitor,0);
		strcpy(read_reg1, rs);
		strcpy(read_reg2, "-");
		strcpy(write_reg, rt);
		write_data_reg = INT_MAX;
		read_data1 = register_read(rs);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		sprintf(alu_out_reg, "%x",address_monitor);
		//Control Signals
		memread = 1;
		alusrca = 0;
		iord = 1;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state = 4;
		break;
	case 4: // lw Cycle 5
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = memory_data_reg;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, "-");
		strcpy(write_reg, rt);
		write_data_reg = memory_data_reg;
		read_data1 = register_read(rs);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		sprintf(alu_out_reg, "-");
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 1;
		regwrite = 1;
		regdst = 0;
		current_state = 0;
		break;
	case 5: // sw Cycle 4
		//Monitors
		pc_monitor = pc;
		address_monitor = alu_in1 + alu_in2;
		write_data_memory = register_read(rt);
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, "-");
		strcpy(write_reg, rt);
		write_data_reg = INT_MAX;
		read_data1 = register_read(rs);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		sprintf(alu_out_reg, "-");
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 1;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 1;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state = 0;
		break;
	case 6: // R-Type Cycle 3
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, rt);
		strcpy(write_reg, rd);
		write_data_reg = INT_MAX;
		if (strcmp(rs, "-") == 0)
			read_data1 = INT_MAX;
		else
			read_data1 = register_read(rs);
		read_data2 = register_read(rt);
		a = read_data1;
		b = read_data2;
		alu_in1 = a;
		alu_in2 = b;
		sprintf(alu_out, "%x",register_read(rd));
		sprintf(alu_out_reg, "%x", register_read(rd));
		//Control Signals
		memread = 0;
		alusrca = 1;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 10;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state = 7;
		break;
	case 7: // R-Type Cycle 4
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, rt);
		strcpy(write_reg, rd);
		write_data_reg = register_read(rd);
		if (strcmp(rs, "-") == 0)
			read_data1 = INT_MAX;
		else
			read_data1 = register_read(rs);
		read_data2 = register_read(rt);
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 1;
		regdst = 1;
		current_state = 0;
		break;
	case 8: // Branch Cycle 3
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, rt);
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, rt);
		strcpy(write_reg, "-");
		write_data_reg = INT_MAX;
		read_data1 = register_read(rs);
		read_data2 = register_read(rt);
		a = read_data1;
		b = read_data2;
		alu_in1 = a;
		alu_in2 = b;
		sprintf(alu_out, "%x",a-b);
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 1;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 01;
		pcwrite = 0;
		pcsource = 01;
		pcwritecond = 1;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state = 0;
		break;
	case 9: // Jump Cycle 3
		//Monitors
		pc_monitor = current_instruction * 4;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, "-");
		strcpy(rt_monitor, "-");
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, "-");
		strcpy(read_reg2, "-");
		strcpy(write_reg, "-");
		write_data_reg = INT_MAX;
		read_data1 = INT_MAX;
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 1;
		pcsource = 10;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state = 0;
		break;
	case 10: // I-Type Cycle 3
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		if (strcmp(opcode, "lui") == 0) {
			strcpy(rs_monitor, "-");
			strcpy(read_reg1, "-");
			read_data1 = INT_MAX;
		}
		else {
			strcpy(read_reg1, rs);
			strcpy(rs_monitor, rs);
			read_data1 = register_read(rs);
		}
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = INT_MAX;
		strcpy(read_reg2, "-");
		strcpy(write_reg, rt);
		write_data_reg = INT_MAX;
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = a;
		alu_in2 = immediate;
		sprintf(alu_out, "%x", register_read(rt));
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 1;
		iord = 0;
		irwrite = 0;
		alusrcb = 10;
		aluop = 10;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 0;
		current_state = 11;
		break;
	case 11: // I-Type Cycle 4
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		if (strcmp(opcode, "lui") == 0) {
			strcpy(rs_monitor, "-");
			strcpy(read_reg1, "-");
			read_data1 = INT_MAX;
		}
		else {
			strcpy(read_reg1, rs);
			strcpy(rs_monitor, rs);
			read_data1 = register_read(rs);
		}
		strcpy(rt_monitor, rt);
		immediate_monitor = immediate;
		memory_data_reg = INT_MAX;
		strcpy(read_reg2, "-");
		strcpy(write_reg, rt);
		write_data_reg = register_read(rt);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 1;
		regdst = 0;
		current_state = 0;
		break;
	case 12: // Jal Cycle 3
		//Monitors
		pc_monitor = current_instruction * 4;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, "-");
		strcpy(rt_monitor, "-");
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, "-");
		strcpy(read_reg2, "-");
		strcpy(write_reg, "$ra");
		write_data_reg = register_read("$ra");
		read_data1 = INT_MAX;
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 0;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 1;
		pcsource = 10;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 1;
		regdst = 00;
		current_state = 0;
		break;
	case 13: // Jr Cycle 3
		//Monitors
		pc_monitor = current_instruction * 4;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, "-");
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, "-");
		strcpy(write_reg, "-");
		write_data_reg = INT_MAX;
		read_data1 = register_read(rs);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 1;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 10;
		pcwrite = 0;
		pcsource = 00;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 00;
		current_state = 14;
		break;
	case 14: // Jr Cycle 4
		//Monitors
		pc_monitor = pc;
		address_monitor = INT_MAX;
		write_data_memory = INT_MAX;
		sprintf(memory_out, "-");
		strcpy(opcode_monitor, opcode);
		strcpy(rs_monitor, rs);
		strcpy(rt_monitor, "-");
		immediate_monitor = INT_MAX;
		memory_data_reg = INT_MAX;
		strcpy(read_reg1, rs);
		strcpy(read_reg2, "-");
		strcpy(write_reg, "-");
		write_data_reg = INT_MAX;
		read_data1 = register_read(rs);
		read_data2 = INT_MAX;
		a = read_data1;
		b = read_data2;
		alu_in1 = INT_MAX;
		alu_in2 = INT_MAX;
		sprintf(alu_out, "-");
		strcpy(alu_out_reg, alu_out);
		//Control Signals
		memread = 0;
		alusrca = 00;
		iord = 0;
		irwrite = 0;
		alusrcb = 00;
		aluop = 00;
		pcwrite = 1;
		pcsource = 01;
		pcwritecond = 0;
		memwrite = 0;
		memtoreg = 0;
		regwrite = 0;
		regdst = 00;
		current_state = 0;
		break;
	}
	//export_cycle_info(current_cycle);
	check_cycle_print();
}

int main() {
	filepointer = fopen("multi_cycle_test.s", "r");
	//Initializaton of values
	for (i = 0; i < MAX_MEMORY_SIZE; i++) {
		memory[i].address = 0;
	}
	gp = 0x10008000;
	sp = 0x7ffffffc;
	zero = 0;
	pc = 0;
	output = fopen("output_file.txt", "w");
	fprintf(output, "Name: Michalis Piponidis\nID: 912526\n\n");
	printf("\nInsert Cycle 1: ");
	scanf("%d", &cycle1);
	printf("\nInsert Cycle 2: ");
	scanf("%d", &cycle2);
	start = clock();
	parser();
	instruction_counter--;
	//Execution of Instructions
	current_cycle = 0;
	while (end == 0) { //end becomes 1 when sll $zero, $zero, 0 is found
		i = 0;
		while ((instructions[current_instruction][i] != ' ') && (instructions[current_instruction][i] != '\t')) { //isolate the opcode (instruction)
			opcode[i] = instructions[current_instruction][i];
			i++;
		}
		opcode[i] = '\0';
		if (strcmp(opcode, "add") == 0) { //add
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, add(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "addi") == 0) { //addi
			decode_i_type();
			strcpy(rd, "-");
			control(6);
			control(6);
			control(6);
			register_write(rs, addi(register_read(rt), immediate));
			control(6);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) immediate= %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "addiu") == 0) { //addiu
			decode_i_type();
			strcpy(rd, "-");
			control(6);
			control(6);
			control(6);
			register_write(rs, addiu(register_read(rt), immediate));
			control(6);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) immediate= %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), immediate);
			
		}
		else if (strcmp(opcode, "addu") == 0) { //addu
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, addu(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "and") == 0) { //and
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, and (register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "andi") == 0) { //andi
			decode_i_type();
			strcpy(rd, "-");
			control(6);
			control(6);
			control(6);
			register_write(rs, andi(register_read(rt), immediate));
			control(6);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) immediate= %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "beq") == 0) { //beq
			decode_branch();
			strcpy(rd, "-");
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(4);
			control(4);
			branch_instruction = label_decode(word);
			if (beq(register_read(rs), register_read(rt))) {
				current_instruction = branch_instruction - 1; //-1 epeidi kanume +1 sto telos tou loop
			}
			control(4);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) label= %s. Branch: %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), word, (register_read(rs) == register_read(rt)));
		}
		else if (strcmp(opcode, "bne") == 0) { //bne
			decode_branch();
			strcpy(rd, "-");
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(4);
			control(4);
			branch_instruction = label_decode(word);
			if (bne(register_read(rs), register_read(rt))) {
				current_instruction = branch_instruction - 1; //-1 epeidi kanume +1 sto telos tou loop
			}
			control(4);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) label= %s. Branch: %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), word, (register_read(rs) != register_read(rt)));
		}
		else if (strcmp(opcode, "j") == 0) { //j
			decode_jump();
			strcpy(rd, "-");
			strcpy(rs, "-");
			strcpy(rt, "-");
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(5);
			control(5);
			current_instruction = label_decode(word) - 1; //-1 epeidi kanume +1 sto telos tou loop
			control(5);
			printf("\nInstruction %d is %s with label= %s. Current Instruction = %d\n", current_instruction, opcode, word, current_instruction);
		}
		else if (strcmp(opcode, "jal") == 0) { //jal
			decode_jump();
			strcpy(rd, "-");
			immediate = label_decode(word) * 4;
			control(7);
			control(7);
			ra = pc;
			current_instruction = label_decode(word) - 1; //-1 epeidi kanume +1 sto telos tou loop
			control(7);
			printf("\nInstruction %d is %s with label= %s. Current Instruction = %d\n", current_instruction, opcode, word, current_instruction);
		}
		else if (strcmp(opcode, "jr") == 0) { //jr
			decode_jr();
			strcpy(rd, "-");
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(8);
			control(8);
			control(8);
			pc = register_read(rs);
			current_instruction = (pc / 4) - 1; //-1 epeidi kanume +1 sto telos tou loop
			control(8);
			printf("\nInstruction %d is %s with label= %s. Current Instruction = %d\n", current_instruction, opcode, word, current_instruction);
		}
		else if (strcmp(opcode, "lui") == 0) { //lui
		decode_lui();
		strcpy(rd, "-");
		strcpy(rs, "-");
		control(6);
		control(6);
		control(6);
		register_write(rt, sll(immediate, 16));
		control(6);
		printf("\nInstruction %d is %s with label= %s. Current Instruction = %d\n", current_instruction, opcode, word, current_instruction);
		}
		else if (strcmp(opcode, "lw") == 0) { //lw
			decode_memory();
			strcpy(rd, "-");
			control(1);
			control(1);
			control(1);
			control(1);
			register_write(rt, lw(register_read(rs), immediate));
			control(1);
			printf("\nInstruction %d is %s with rt= %s (%d) base= %s (%d) offset= %d.\n", current_instruction, opcode, rt, register_decode(rt), rs, register_decode(rs), immediate);
		}
		else if (strcmp(opcode, "nor") == 0) { //nor
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, nor(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "or") == 0) { //or
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, or (register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "ori") == 0) { //ori
			decode_i_type();
			strcpy(rd, "-");
			control(6);
			control(6);
			control(6);
			register_write(rs, ori(register_read(rt), immediate));
			control(6);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) immediate= %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "slt") == 0) { //slt
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, slt(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "slti") == 0) { //slti
			decode_i_type();
			strcpy(rd, "-");
			control(6);
			control(6);
			control(6);
			register_write(rs, slti(register_read(rt), immediate));
			control(6);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) immediate= %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "sltiu") == 0) { //sltiu
			decode_i_type();
			strcpy(rd, "-");
			control(6);
			control(6);
			control(6);
			register_write(rs, sltiu(register_read(rt), immediate));
			control(6);
			printf("\nInstruction %d is %s with rs= %s (%d) rt= %s (%d) immediate= %d\n", current_instruction, opcode, rs, register_decode(rs), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "sltu") == 0) { //sltu
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, sltu(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "sll") == 0) { //sll
			decode_shift();
			strcpy(rs, "-");
			control(3);
			control(3);
			control(3);
			//Sinthiki teliomatos programmatos
			if ((strcmp("$zero", rd) == 0) && (strcmp("$zero", rt) == 0) && (immediate == 0)) {
				control(3);
				end = 1;
				break;
			}
			register_write(rd, sll(register_read(rt), immediate));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rt= %s (%d) shift amount= %d\n", current_instruction, opcode, rd, register_decode(rd), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "srl") == 0) { //srl
			decode_shift();
			strcpy(rs, "-");
			control(3);
			control(3);
			control(3);
			register_write(rd, srl(register_read(rt), immediate));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rt= %s (%d) shift amount= %d\n", current_instruction, opcode, rd, register_decode(rd), rt, register_decode(rt), immediate);
		}
		else if (strcmp(opcode, "sw") == 0) { //sw
			decode_memory();
			strcpy(rd, "-");
			control(2);
			control(2);
			control(2);
			sw(register_read(rt), register_read(rs), immediate);
			control(2);
			printf("\nInstruction %d is %s with rt= %s (%d) base= %s (%d) offset= %d.\n", current_instruction, opcode, rt, register_decode(rt), rd, register_decode(rd), immediate);
		}
		else if (strcmp(opcode, "sub") == 0) { //sub
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, sub(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		else if (strcmp(opcode, "subu") == 0) { //subu
			decode_r_type();
			immediate = INT_MAX; // Gia na ginei to monitor 8 pavla
			control(3);
			control(3);
			control(3);
			register_write(rd, subu(register_read(rs), register_read(rt)));
			control(3);
			printf("\nInstruction %d is %s with rd= %s (%d) rs= %s (%d) rt= %s (%d)\n", current_instruction, opcode, rd, register_decode(rd), rs, register_decode(rs), rt, register_decode(rt));
		}
		current_instruction++;
		pc = current_instruction * 4;
		//print_registers_hex();
		//printf("\nCYCLE %d\n\n", current_cycle);
	}
	endt = clock();
	print_registers_hex();
	printf("\nFINAL CYCLES %d\n\n", current_cycle);
	export_final_info();
	printf("\nThe program has ended.\n");
	printf("\nTime (seconds): %f\n", ((float)(endt - start) / CLOCKS_PER_SEC));
}