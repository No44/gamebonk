#ifndef _GBONK_DEBUG_DEBUGGER_HPP_
#define _GBONK_DEBUG_DEBUGGER_HPP_

#include <vector>
#include <algorithm>

#include "debug/MemoryDumper.hpp"
#include "debug/Instruction.hpp"
#include "CPU.hpp"

namespace GBonk
{

    class CPU;
    enum class CPUFlags : unsigned int;

    namespace Debug
    {
        class DebuggerHost;
        class Debugger
        {
        public:
            Debugger(DebuggerHost&, CPU&);

            bool isBreakpointSet(unsigned int addr) { return std::binary_search(breakpoints_.begin(), breakpoints_.end(), addr); }
            void setBreakpoint(unsigned int addr);
            void disableBreakpoint(unsigned int addr);

            // called by debuggerhost
            void step();
            void cont();
            void brk();
            void toggleFlag(CPUFlags f);
            void loadFrom(unsigned int addr) { nextInstrAddr_ = addr; }

            // called by engine
            void run();

            std::vector<Instruction> getMoreInstructions(int amount);

        private:
            void stepUpdateDisplay_();

            CPU& cpu_;
            DebuggerHost& host_;
            unsigned int nextInstrAddr_;
            std::vector<unsigned int> breakpoints_;
            bool break_;
        };

    }
}

#endif
