#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include<string>

enum op {
ADD, SLLI, SRLI, SRAI, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
JALR, LB, LH, LW, LBU, LHU, ADDI, SLTI, SLTIU, XORI, ORI, ANDI,
SB, SH, SW,
LUI, AUIPC,
BEQ, BNE, BLT, BGE, BLTU, BGEU,
JAL, 
PSEUDO_JAL, 
};

const std::string opName[38] = {
"add", "slli", "srli", "srai", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and", 
"jalr", "lb", "lh", "lw", "lbu", "lhu", "addi", "slti", "sltiu", "xori", "ori", "andi", 
"sb", "sh", "sw",
"lui", "auipc", 
"beq", "bne", "blt", "bge", "bltu", "bgeu", 
"jal", 
"jal read_int"
};

enum caller_reg {
ra, t1, t2, a0, a1, a2, a3, a4, a5, a6, a7, t3, t4, t5, t6
};

enum callee_reg {
s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11
};

const std::string caller_regname[15] = {
"x1", "x6", "x7", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x28", "x29", "x30", "x31"
};

const std::string callee_regname[11] = {
"x9", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27"
};
#endif