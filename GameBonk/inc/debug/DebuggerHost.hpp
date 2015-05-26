#ifndef _GBONK_DEBUG_DEBUGGERHOST_HPP_
#define _GBONK_DEBUG_DEBUGGERHOST_HPP_

#include <list>
#include <utility>
#include <algorithm>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Display.H>

#include "debug/Debugger.hpp"
#include "debug/Instruction.hpp"

namespace GBonk
{

    namespace Debug
    {

        class DebuggerHost : public Fl_Window
        {
        public:
            class SourceDisplay : public Fl_Text_Display
            {
            public:
                SourceDisplay(DebuggerHost& parent, int x, int y, int w, int h);
                int handle(int ev) override;
                void setInstructions(const std::vector<Instruction>& instr);
                void setCurrent(const Instruction& ins);
                bool displays(const Instruction& ins) const;

            private:
                void addInstructions_(const std::vector<Instruction>& instr);

                //                   pos of line beginning in buffer
                typedef std::pair<int, Instruction> PosdIns;
                std::vector<PosdIns> displayed_;
                DebuggerHost& host_;
                friend class DebuggerHost;
            };

            DebuggerHost();

            void open();
            void pumpEvents();
            
            Debugger* debugger() { return dbg_; }
            void debugger(Debugger& d);

        private:
            Debugger* dbg_;
            SourceDisplay* srcView_;
            friend class Debugger;
        };

    }

}
#endif
