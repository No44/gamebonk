#include <algorithm>

#include "CPU.hpp"
#include "debug/Debugger.hpp"
#include "ROMReader.hpp"


namespace GBonk
{
    namespace Debug
    {
        
        Debugger::Debugger(DebuggerHost& host, CPU& cpu)
            : cpu_(cpu),
            host_(host),
            break_(true)
        {

        }

        void Debugger::setBreakpoint(unsigned int addr)
        {
            //upperbound
            auto it = std::upper_bound(breakpoints_.begin(), breakpoints_.end(), addr);
            breakpoints_.insert(it, addr);
        }

        void Debugger::disableBreakpoint(unsigned int addr)
        {
            auto it = std::upper_bound(breakpoints_.begin(), breakpoints_.end(), addr);
            if (it == breakpoints_.end() || *it != addr)
                return;
            breakpoints_.erase(it);
        }

        void Debugger::step()
        {
            cpu_.runOne();
        }

        void Debugger::cont()
        {
            break_ = false;
        }

        void Debugger::run()
        {
            if (break_)
                return;

            cpu_.runOne();
            if (std::binary_search(breakpoints_.begin(), breakpoints_.end(), cpu_.registers_.pc))
                break_ = true;
        }

        std::vector<Instruction> Debugger::getMoreInstructions(int amount)
        {
            std::vector<Instruction> res;

            res.reserve(amount);
            for (int i = 0; i < amount; ++i)
            {
                Instruction ins(ROMReader(cpu_, nextInstrAddr_));

                res.push_back(ins);
                nextInstrAddr_ += ins.len();
            }
            return res;
        }

    }

}
