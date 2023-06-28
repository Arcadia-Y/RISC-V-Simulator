#ifndef INS_HPP
#define INS_HPP

enum Ins 
{
    lui, auipc, jal, jalr, beq, bne, 
    blt, bge, bltu, bgeu, lb, lh,
    lw, lbu, lhu, sb, sh, sw,
    addi, slti, sltiu, xori, ori, andi,
    slli, srli, srai, add, sub, sll,
    slt, sltu, Xor, srl, sra, Or, And, end
};

#endif
