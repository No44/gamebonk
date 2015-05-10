#ifndef DBONK_ASSEMBLY_HPP_
#define DBONK_ASSEMBLY_HPP_

#include <list>
#include <string>

#include "Instruction.hpp"

namespace DBonk
{

    class Assembly
    {
    public:

        void addInstruction(const Instruction&);
        std::string toString() const;

    private:
        std::list<Instruction> ins_;
    };


}

#endif
