#include <sstream>
#include <iomanip>

#include "utility.hpp"
#include "Assembly.hpp"

namespace DBonk
{

    std::string Assembly::toString() const
    {
        std::stringstream builder;
        for (auto& ins : ins_)
        {
            builder << "[0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << ins.addr() << "]";

            builder << ins.toString();

            builder << '\n';
        }
        return builder.str();
    }

    void Assembly::addInstruction(const Instruction& i)
    {
        ins_.push_back(i);
    }

}
