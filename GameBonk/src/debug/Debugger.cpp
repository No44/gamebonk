#include <algorithm>

#include "CPU.hpp"
#include "debug/DebuggerHost.hpp"
#include "debug/Debugger.hpp"
#include "ROMReader.hpp"


namespace GBonk
{
    namespace Debug
    {
        
        Debugger::Debugger(DebuggerHost& host, CPU& cpu)
            : cpu_(cpu),
            host_(host),
            nextInstrAddr_(cpu.registers_.pc),
            break_(true)
        {

        }

        void Debugger::setBreakpoint(unsigned int addr)
        {
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

        void Debugger::stepUpdateDisplay_()
        {
            Instruction current(ROMReader(cpu_, cpu_.registers_.pc));

            if (!host_.srcView_->displays(current))
            {
                // if the host doesn't display this isntruction, we have
                // to give out a new set
                nextInstrAddr_ = cpu_.registers_.pc;
                host_.srcView_->setInstructions(getMoreInstructions(40));
            }
            host_.srcView_->setCurrent(current);
        }

        void Debugger::step()
        {
            cpu_.runOne();
            stepUpdateDisplay_();
        }

        void Debugger::brk()
        {
            break_ = true;
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
            {
                break_ = true;
                stepUpdateDisplay_();
            }
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
