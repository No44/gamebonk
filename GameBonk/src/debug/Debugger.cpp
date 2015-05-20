#include "CPU.hpp"
#include "debug/Debugger.hpp"

namespace GBonk
{
    namespace Debug
    {
        
        Debugger::Debugger(CPU& cpu)
            : cpu_(cpu)
        {

        }

        void Debugger::setHost(DebuggerHost* h)
        {
            host_ = h;
        }

        void Debugger::update()
        {

        }

    }

}
