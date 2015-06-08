#ifndef _GBONK_DEBUG_DEBUGGERHOST_HPP_
#define _GBONK_DEBUG_DEBUGGERHOST_HPP_

#include <list>
#include <utility>
#include <algorithm>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Display.H>
#include "debug/Instruction.hpp"
#include "CPU.hpp"

class Fl_Button;
class Fl_Box;
class Fl_Check_Button;
class Fl_Int_Input;

namespace GBonk
{

    namespace Debug
    {

        class Debugger;

        class DebuggerHost : public Fl_Window
        {
        public:
            static const unsigned int Height = 600;
            static const unsigned int Width = 700;

            struct RegDisplay;

            class SourceDisplay : public Fl_Text_Display
            {
            public:
                SourceDisplay(DebuggerHost& parent, int x, int y, int w, int h);
                int handle(int ev) override;
                void setInstructions(const std::vector<Instruction>& instr);
                void setCurrent(const Instruction& ins);
                bool displays(const Instruction& ins) const;

            private:
                static const unsigned int Lines = 28;

                void addInstructions_(const std::vector<Instruction>& instr);

                //                   pos of line beginning in buffer
                typedef std::pair<int, Instruction> PosdIns;
                std::vector<PosdIns> displayed_;
                DebuggerHost& host_;
                int currentInsIdx_;
                friend class DebuggerHost;
            };

            DebuggerHost();
            ~DebuggerHost();

            void open();
            void pumpEvents();
            
            void disable();
            void enable();

            Debugger* debugger() { return dbg_; }
            void debugger(Debugger& d);

            void updateRegisters(CPU::Registers& regs);

        private:
            template<CPUFlags FLAG>
            static void checkBttnToggleFlag_(Fl_Widget*, void* host);
            static void cbSeek_(Fl_Widget*, void*);

            Debugger* dbg_;
            SourceDisplay* srcView_;
            Fl_Int_Input* seek_;
            Fl_Button* bttnStep_;
            Fl_Button* bttnCont_;
            Fl_Button* bttnPause_;
            RegDisplay* af_;
            Fl_Box* flagsBox_;
            std::map<CPUFlags, Fl_Check_Button*> flagsBttn_;
            RegDisplay* bc_;
            RegDisplay* de_;
            RegDisplay* hl_;
            CPU::Registers* activeRegisters_;

            friend class Debugger;
        };

    }

}
#endif
