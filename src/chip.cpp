#include "../include/chip.hpp"
#include <random>
#include <algorithm>
#define RANDOM_ORDER

void Chip::read_ins()
{
    std::string str;
    int status = 0, address = 0;
    while (!std::cin.eof())
    {
        std::cin >> str;
        if (str[0] == '@')
            address = std::stoi(str.substr(1), 0, 16);
        else
        {
            ram[address + status] = std::stoi(str, 0, 16);
            if (++status == 4)
            {
                status = 0;
                address += 4;
            }
        }
    }
    // load first instruction
    ins = ram[0];
    ins |= ((unsigned int)ram[1]) << 8;
    ins |= ((unsigned int)ram[2]) << 16;
    ins |= ((unsigned int)ram[3]) << 24;
}

int Chip::run()
{
    #ifdef RANDOM_ORDER
    std::random_device rd;
    std::mt19937 ram(rd());
    while (!end_flag)
    {
        int number1[] = {0, 1, 2, 3};
        std::shuffle(number1, number1+4, ram);
        for (int i = 0; i < 4; i++)
            switch (number1[i])
            {
            case 0:
                decode_and_issue();
                break;
            case 1:
                alu.execute();
                break;
            case 2:
                lsb.execute();
                break;
            case 3:
                commit();
            }

        // now cdb is updated
        if (!clear_flag)
        {
            int number2[] = {0, 1, 2};
            std::shuffle(number2, number2+3, ram);
            for (int i = 0; i < 3; i++)
                switch (number2[i])
                {
                case 0:
                    alu.execute();
                    break;
                case 1:
                    lsb.execute();
                    break;
                case 2:
                    write_back();
                }
        }
        else 
        {
            clear_flag = false;
            alu.clear();
            lsb.clear();
            rob.clear();
            for (int i = 1; i < 32; i++)
                next_reg[i].status = -1;
        }
        fetch();
        update();
    }

    #else
    while (!end_flag)
    {
        decode_and_issue();
        alu.execute();
        lsb.execute();
        commit();

        // now cdb is updated
        if (!clear_flag)
        {
            alu.execute();
            lsb.execute();
            write_back();
        }
        else 
        {
            clear_flag = false;
            alu.clear();
            lsb.clear();
            rob.clear();
            for (int i = 1; i < 32; i++)
                next_reg[i].status = -1;
        }
        fetch();
        update();
    }
    #endif

    return ((unsigned int)reg[10].data) & 255u;
}

Prediction_Report Chip::count_success()
{
    return pc.count_success();
}

int Chip::sign_extend(int x, int digit)
{
    int high = x >> (digit - 1);
    if (!high) return x;
    high = -1;
    high <<= digit - 1;
    return x | high;
}

void Chip::fetch()
{
    next_ins = ram[pc.get_next()];
    next_ins |= ((unsigned int)ram[pc.get_next()+1]) << 8;
    next_ins |= ((unsigned int)ram[pc.get_next()+2]) << 16;
    next_ins |= ((unsigned int)ram[pc.get_next()+3]) << 24;
}

void Chip::update()
{
    rob.update();
    lsb.update();
    alu.update();
    for (int i = 0; i < 32; i++)
        reg[i] = next_reg[i];
    cdb.clear();
    pc.update();
    ins = next_ins;
}

void Chip::decode_and_issue()
{
    if (clear_flag) return;
    Reservation_Station::Entry rs_entry;
    Reorder_Buffer::Entry rob_entry;
    decode(rs_entry, rob_entry);
    #ifdef MYDEBUG
    if (!rob.buffer.empty())
    {
        pc.set(pc.get());
        return;
    }
    #endif
    if (rs_entry.op == end)
    {
        pc.set(pc.get());
        if (rob.buffer.empty())
            end_flag = true;
        return;
    }
    if (!rob.buffer.available())
    {
        pc.set(pc.get());
        return;
    }
    enum itype {loadstore, branch, jump, other};
    itype t;
    switch (rs_entry.op)
    {
    case lb:
    case lh:
    case lw:
    case lbu:
    case lhu:
    case sb:
    case sh:
    case sw:
        t = loadstore;
        break;
    case beq:
    case bgeu:
    case bge:
    case blt:
    case bltu:
    case bne: 
        t = branch;
        break;
    case jal:
    case jalr:
        t = jump;
        break;
    default:
        t = other;
    }
    if (t == loadstore && !lsb.available())
    {
        pc.set(pc.get());
        return;
    }
    else if (t != jump && !alu.available())
    {
        pc.set(pc.get());
        return;
    }
    if (t == branch)
        rob_entry.dest = pc.branch(pc.get()+rs_entry.a);
    int qj = rs_entry.qj, qk = rs_entry.qk;
    if (qj >= 0)
    {
        int status = reg[qj].status;
        auto item = rob.buffer.access(status);
        bool load;
        switch (item->ins)
        {
        case lb:
        case lbu:
        case lh:
        case lhu:
        case lw:
            load = true;
            break;
        default:
            load = false;
        }
        if (status == -1)
        {
            rs_entry.vj = reg[qj].data;
            rs_entry.qj = -1;
        }
        else if (!load && item->state)
        {
            rs_entry.vj = item->value;
            rs_entry.qj = -1;
        }
        else rs_entry.qj = status;
    }
    if (qk >= 0)
    {
        int status = reg[qk].status;
        auto item = rob.buffer.access(status);
        bool load;
        switch (item->ins)
        {
        case lb:
        case lbu:
        case lh:
        case lhu:
        case lw:
            load = true;
            break;
        default:
            load = false;
        }
        if (status == -1)
        {
            rs_entry.vk = reg[qk].data;
            rs_entry.qk = -1;
        }
        else if (!load && item->state)
        {
            rs_entry.vk = item->value;
            rs_entry.qk = -1;
        }
        else rs_entry.qk = status;
    }
    if (rs_entry.op == jalr)
    {
        if (rs_entry.qk >= 0)
        {
            pc.set(pc.get());
            return;
        }
        rs_entry.vk += rs_entry.a;
        rs_entry.vk -= rs_entry.vk % 2;
        pc.set(rs_entry.vk);
    }
    int rename = rob.next.end().getid();
    rs_entry.dest = rename;
    if (t != branch && rob_entry.dest)
        next_reg[rob_entry.dest].status = rename;
    rob.next.push(rob_entry);
    if (t == loadstore) lsb.push(rs_entry);
    else if (t == jump)
    {
        auto item = rob.next.access(rename);
        item->value = rs_entry.vj;
        item->state = 1;
    }
    else alu.push(rs_entry);
}

void Chip::decode(Reservation_Station::Entry& rs_entry, Reorder_Buffer::Entry& rob_entry)
{
    if (ins == 0xff00513)
    {
        rs_entry.op = rob_entry.ins = end;
        return;
    }
    int opcode = ins & 0b1111111;
    int rd = (ins >> 7) & 0b11111;
    int func3 = (ins >> 12) & 0b111;
    int rs1 = (ins >> 15) & 0b11111;
    int rs2 = (ins >> 20) & 0b11111;
    int func7 = (ins >> 25) & 0b1111111;
    int imm;
    switch (opcode)
    {
    case 0b0110111:
        rs_entry.op = rob_entry.ins = lui;
        rs_entry.vj = sign_extend(ins >> 12, 20) << 12;
        rob_entry.dest = rd;
        break;
    case 0b0010111:
        rs_entry.op = rob_entry.ins = auipc;
        rs_entry.vj = pc.get();
        rs_entry.vk = sign_extend(ins >> 12, 20);
        rob_entry.dest = rd;
        break;
    case 0b1101111:
        rs_entry.op = rob_entry.ins = jal;
        rs_entry.vj = pc.get() + 4;
        rob_entry.dest = rd;
        imm = ((ins >> 21) & ((1 << 10) - 1)) << 1;
        imm |= ((ins >> 20) & 1) << 11;
        imm |= ((ins >> 12) & ((1 << 8) - 1)) << 12;
        imm |= ((ins >> 31) & 1) << 20;
        imm = sign_extend(imm, 21);
        pc.set(pc.get() + imm);
        break;
    case 0b1100111:
        rs_entry.op = rob_entry.ins = jalr;
        rs_entry.vj = pc.get() + 4;
        rob_entry.dest = rd;
        imm = (ins >> 20) & ((1 << 12) - 1);
        imm = sign_extend(imm, 12);
        rs_entry.qk = rs1;
        rs_entry.a = imm;
        break;
    case 0b1100011: // branch
        imm = (ins >> 7) & ((1 << 5) - 2);
        imm |= ((ins >> 7) & 1) << 11;
        imm |= ((ins >> 25) & ((1 << 6) - 1)) << 5;
        imm |= ((ins >> 31) & 1) << 12;
        imm = sign_extend(imm, 13);
        rs_entry.qj = rs1;
        rs_entry.qk = rs2;
        rs_entry.a = imm;
        switch (func3)
        {
        case 0b000:
            rs_entry.op = rob_entry.ins = beq;
            break;
        case 0b001:
            rs_entry.op = rob_entry.ins = bne;
            break;
        case 0b100:
            rs_entry.op = rob_entry.ins = blt;
            break;
        case 0b101:
            rs_entry.op = rob_entry.ins = bge;
            break;
        case 0b110:
            rs_entry.op = rob_entry.ins = bltu;
            break;
        case 0b111:
            rs_entry.op = rob_entry.ins = bgeu;
            break;
        }
        break;
    case 0b00000011: // load
        imm = (ins >> 20) & ((1 << 12) - 1);
        imm = sign_extend(imm, 12);
        rs_entry.qj = rs1;
        rs_entry.a = imm;
        rob_entry.dest = rd;
        switch (func3)
        {
        case 0b000:
            rs_entry.op = rob_entry.ins = lb;
            break;
        case 0b001:
            rs_entry.op = rob_entry.ins = lh;
            break;
        case 0b010:
            rs_entry.op = rob_entry.ins = lw;
            break;
        case 0b100:
            rs_entry.op = rob_entry.ins = lbu;
            break;
        case 0b101:
            rs_entry.op = rob_entry.ins = lhu;
            break;
        }
        break;
    case 0b0100011: //store
        imm = (ins >> 7) & 0b11111;
        imm |=  ((ins >> 25) & ((1 << 7) - 1)) << 5;
        imm = sign_extend(imm, 12);
        rs_entry.qj = rs1;
        rs_entry.a = imm;
        rs_entry.qk = rs2;
        rob_entry.dest = 0;
        switch (func3)
        {
        case 0b000:
            rs_entry.op = rob_entry.ins = sb;
            break;
        case 0b001:
            rs_entry.op = rob_entry.ins = sh;
            break;
        case 0b010:
            rs_entry.op = rob_entry.ins = sw;
            break;
        }
        break;
    case 0b0010011: // I-type
        rs_entry.qj = rs1;
        rob_entry.dest = rd;
        rs_entry.vk = sign_extend((ins >> 20) & ((1 << 12) - 1), 12);
        switch (func3)
        {
        case 0b000:
            rs_entry.op = rob_entry.ins = addi;
            break;
        case 0b010:
            rs_entry.op = rob_entry.ins = slti;
            break;
        case 0b011:
            rs_entry.op = rob_entry.ins = sltiu;
            break;
        case 0b100:
            rs_entry.op = rob_entry.ins = xori;
            break;
        case 0b110:
            rs_entry.op = rob_entry.ins = ori;
            break;
        case 0b111:
            rs_entry.op = rob_entry.ins = andi;
            break;
        case 0b001:
            rs_entry.op = rob_entry.ins = slli;
            rs_entry.vk = (ins >> 20) & 0b11111;
            break;
        case 0b101:
            if (!func7)
            {
                rs_entry.op = rob_entry.ins = srli;
                rs_entry.vk = (ins >> 20) & 0b11111;
            }
            else
            {
                rs_entry.op = rob_entry.ins = srai;
                rs_entry.vk = (ins >> 20) & 0b11111;
            }
            break;
        }
        break;
    case 0b0110011: // R-type
        rob_entry.dest = rd;
        rs_entry.qj = rs1;
        rs_entry.qk = rs2;
        switch (func3)
        {
        case 0b000:
            rs_entry.op = rob_entry.ins = func7 ? sub : add;
            break;
        case 0b001:
            rs_entry.op = rob_entry.ins = sll;
            break;
        case 0b010:
            rs_entry.op = rob_entry.ins = slt;
            break;
        case 0b011:
            rs_entry.op = rob_entry.ins = sltu;
            break;
        case 0b100:
            rs_entry.op = rob_entry.ins = Xor;
            break;
        case 0b101:
            rs_entry.op = rob_entry.ins = func7 ? sra : srl;
            break;
        case 0b110:
            rs_entry.op = rob_entry.ins = Or;
            break;
        case 0b111:
            rs_entry.op = rob_entry.ins = And;
            break;
        }
        break;
    }
}

void Chip::commit()
{
    if (!rob.buffer.empty())
    {
        auto it = rob.buffer.begin();
        int imm;
        if (it->state)
        {
            auto next_it = rob.next.access(it.getid());
            switch (it->ins)
            {
            case beq:
            case bgeu:
            case bge:
            case blt:
            case bltu:
            case bne:
                if (!pc.verify(it->value))
                {
                    pc.set(it->dest);
                    clear_flag = true;
                }
                rob.next.pop();
                break;
            case sb:
                if (++(next_it->state) > 3)
                {
                    ram[it->dest] = it->value & ((1 << 8) - 1);
                    rob.next.pop();
                }
                break;
            case sh:
                if (++(next_it->state) > 3)
                {
                    ram[it->dest] = it->value & ((1 << 8) - 1);
                    ram[it->dest+1] = (it->value >> 8) & ((1 << 8) - 1);
                    rob.next.pop();
                }
                break;
            case sw:
                if (++(next_it->state) > 3)
                {
                    ram[it->dest] = it->value & ((1 << 8) - 1);
                    ram[it->dest+1] = (it->value >> 8) & ((1 << 8) - 1);
                    ram[it->dest+2] = (it->value >> 16) & ((1 << 8) - 1);
                    ram[it->dest+3] = (it->value >> 24) & ((1 << 8) - 1);
                    rob.next.pop();
                }
                break;
            case lb:
            case lbu:
                if (++(next_it->state) > 3)
                {
                    cdb[it.getid()] = next_reg[it->dest].data = it->ins == lb ? (signed char)ram[it->value] : (unsigned char) ram[it->value];
                    if (next_reg[it->dest].status == it.getid())
                        next_reg[it->dest].status = -1;
                    rob.next.pop();
                }
                break;
            case lh:
            case lhu:
                if (++(next_it->state) > 3)
                {
                    imm = ram[it->value];
                    imm |= ((int)ram[it->value+1]) << 8;
                    imm = it->ins == lh ? imm : sign_extend(imm, 16);
                    cdb[it.getid()] = next_reg[it->dest].data = imm;
                    if (next_reg[it->dest].status == it.getid())
                        next_reg[it->dest].status = -1;
                    rob.next.pop();
                }
                break;
            case lw:
                if (++(next_it->state) > 3)
                {
                    imm = ram[it->value];
                    imm |= ((int)ram[it->value+1]) << 8;
                    imm |= ((int)ram[it->value+2]) << 16;
                    imm |= ((int)ram[it->value+3]) << 24;
                    cdb[it.getid()] = next_reg[it->dest].data = imm;
                    if (next_reg[it->dest].status == it.getid())
                        next_reg[it->dest].status = -1;
                    rob.next.pop();
                }
                break;
            default:
                if (it->dest)
                {
                    next_reg[it->dest].data = it->value;
                    if (next_reg[it->dest].status == it.getid())
                        next_reg[it->dest].status = -1;
                }
                rob.next.pop();
            }
        }
    }
}

void Chip::write_back()
{
    for (auto it = cdb.begin(); it != cdb.end(); ++it)
    { 
        auto item = rob.next.access(it->first);
        if (item->state) continue;
        item->value = it->second;
        item->state = true;
    }
}
