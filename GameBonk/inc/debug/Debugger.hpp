#ifndef _GBONK_DEBUG_DEBUGGER_HPP_
#define _GBONK_DEBUG_DEBUGGER_HPP_

#include "debug/MemoryDumper.hpp"
#include "debug/DebuggerHost.hpp"

namespace GBonk
{

    class CPU;

    namespace Debug
    {

        class Debugger
        {
        public:
            Debugger(CPU&);

            void setHost(DebuggerHost*);
            void update();


        private:
            CPU& cpu_;
            DebuggerHost* host_;
            MemoryDumper cpuMem_;
            MemoryDumper extRam_;
        };

    }
}

#endif
