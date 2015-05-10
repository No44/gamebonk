#include <exception>
#include <iostream>
#include <sstream>
#include <cassert>

#include "Cartridge.hpp"
#include "Object.hpp"

namespace DBonk
{

    void Object::load(const GBonk::Cartridge& c)
    {
        unsigned int pc = 0x150;
        const uint8_t* ROM = c.ROM();
        unsigned int romsize = c.ROMSize();

        while (pc < romsize)
        {
            Instruction ins(ROM + pc, pc);

            if (ins.len() == 0) {
                std::cerr << "Unknown instruction " << std::hex << (int)*(ROM + pc) << " at address " << pc << std::endl;
                std::cerr << "Previous, Instruction and following bytes:" << std::hex << " 0x" << (unsigned int)*(ROM + pc - 1) << " 0x" << (unsigned int)*(ROM + pc) << " 0x" << (unsigned int)*(ROM + pc + 1) << " 0x" << (unsigned int)*(ROM + pc + 2);
                throw std::runtime_error(std::string("Unknown instruction at address ") + std::to_string(pc));
            }
            else {
                ass_.addInstruction(ins);
                pc += ins.len();
            }
        }

        if (pc != romsize)
        {
            std::stringstream b;
            b << "Unexpected eof at addr " << std::hex <<  pc << ", expected " << romsize;
            std::cerr << b.str() << std::endl;
            throw std::runtime_error(b.str());
        }
    }


}
