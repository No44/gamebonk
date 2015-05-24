#ifndef _GBONK_DEBUG_DEBUGGERHOST_HPP_
#define _GBONK_DEBUG_DEBUGGERHOST_HPP_

#include <list>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>


#include "debug/Debugger.hpp"
#include "debug/Instruction.hpp"

namespace GBonk
{

    namespace Debug
    {

        class DebuggerHost : public Fl_Window
        {
            class SourceDisplay;


        public:
            DebuggerHost();

            void open();
            void pumpEvents();
            
            Debugger* debugger() { return dbg_; }
            void debugger(Debugger& d) { dbg_ = &d; }

        private:
            Debugger* dbg_;

            SourceDisplay* srcView_;
        };

    }

}
#endif
