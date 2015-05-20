#ifndef _GBONK_DEBUG_DEBUGGERHOST_HPP_
#define _GBONK_DEBUG_DEBUGGERHOST_HPP_

#include <fltk/Fl.H>
#include <fltk/Fl_Window.H>


namespace GBonk
{

    namespace Debug
    {

        class DebuggerHost : public Fl_Window
        {
        public:

            void pumpEvents();
        };

    }

}
#endif
