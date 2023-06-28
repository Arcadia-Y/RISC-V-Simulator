#include "../include/reservation_station.hpp"

void Reservation_Station::push(const Entry& x)
{
    next.push(x);
}

void Reservation_Station::update()
{
    exec_flag = write_flag = false;
    buffer = next;
}

void Reservation_Station::clear()
{
    next.clear();
}

void Reservation_Station::calculate(Entry& x)
{
    x.status = true;
    switch (x.op)
    {
        case lui:
        case jal:
        case jalr:
            break;
        case auipc:
        case addi:
        case add:
            x.vj = x.vj + x.vk;
            break;
        case sub:
            x.vj = x.vj - x.vk;
            break;
        case beq:
            x.vj = x.vj == x.vk;
            break;
        case bne:
            x.vj = x.vj != x.vk;
            break;
        case blt:
        case slti:
            x.vj = x.vj < x.vk;
            break;
        case bge:
            x.vj = x.vj >= x.vk;
            break;
        case bltu:
        case sltiu:
            x.vj = (unsigned) x.vj < (unsigned) x.vk;
            break;
        case bgeu:
            x.vj = (unsigned) x.vj >= (unsigned) x.vk;
            break;
        case xori:
        case Xor:
            x.vj = x.vj ^ x.vk;
            break;
        case ori:
        case Or:
            x.vj = x.vj | x.vk;
            break;
        case andi:
        case And:
            x.vj = x.vj & x.vk;
            break;
        case sll: 
        case slli:
            x.vj = ((unsigned)x.vj) << x.vk;
            break;
        case srl:
        case srli:
            x.vj = ((unsigned)x.vj) >> x.vk;
            break; 
        case sra:
        case srai:
            x.vj = x.vj >> x.vk;
            break;
        default: // load/store
            x.vj = x.vj + x.a;
    }
}

void Reservation_Station::execute()
{
    // check cdb for forwarding
    if (!cdb->empty())
        for (auto it = next.begin(); it != next.end(); ++it)
        {
            if (it->qj >= 0)
            {
                auto found = cdb->find(it->qj);
                if (found != cdb->end())
                {
                    it->qj = -1;
                    it->vj = found->second;
                }
            }
            if (it->qk >= 0)
            {
                auto found = cdb->find(it->qk);
                if (found != cdb->end())
                {
                    it->qk = -1;
                    it->vk = found->second;
                }
            }
        }
    // execute
    if (!exec_flag)
        for (auto it = next.begin(); it != next.end(); ++it)
            if (it->qj < 0 && it->qk < 0 && !it->status)
            {
                calculate(*it);
                exec_flag = true;
                break;
            }
    // write back
    if (!write_flag)
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
            if (it->status)
            {
                (*cdb)[it->dest] = it->vj;
                write_flag = true;
                next.erase(it.getid());
                break;
            }
}

void Load_Store_Buffer::execute()
{
    // check cdb for forwarding
    if (!cdb->empty())
        for (auto it = next.begin(); it != next.end(); ++it)
        {
            if (it->qj >= 0)
            {
                auto found = cdb->find(it->qj);
                if (found != cdb->end())
                {
                    it->qj = -1;
                    it->vj = found->second;
                }
            }
            if (it->qk >= 0)
            {
                auto found = cdb->find(it->qk);
                if (found != cdb->end())
                {
                    it->qk = -1;
                    it->vk = found->second;
                }
            }
        }
    // execute
    if (!exec_flag)
        for (auto it = next.begin(); it != next.end(); ++it)
            if (it->qj < 0 && it->qk < 0 && !it->status)
            {
                calculate(*it);
                exec_flag = true;
                break;
            }
    // write back
    if (!write_flag)
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
            if (it->status)
            {
                auto rob_it = (*rob).next.access(it->dest);
                switch (it->op)
                {
                case sw:
                case sh:
                case sb:
                    rob_it->value = it->vk;
                    rob_it->dest = it->vj;
                    rob_it->state = true;
                    break;
                default: // load
                    rob_it->value = it->vj;
                    rob_it->state = true;
                }
                write_flag = true;
                next.erase(it.getid());
                break;
            }
}
