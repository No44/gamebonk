#include "debug/DebuggerHost.hpp"

namespace GBonk
{

    namespace Debug
    {

        void DebuggerHost::pumpEvents()
        {
            while (Fl::wait(0) > 0);
        }
    }

}
