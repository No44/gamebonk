#include <iostream>
#include <sstream>
#include <iomanip>

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>

#include "debug/DebuggerHost.hpp"

namespace GBonk
{

    namespace Debug
    {

        class DebuggerHost::SourceDisplay : public Fl_Text_Display
        {
        public:
            SourceDisplay(DebuggerHost& parent, int x, int y, int w, int h);
            int handle(int ev) override;
            void setInstructions(const std::vector<Instruction>& instr);

        private:
            void addInstructions_(const std::vector<Instruction>& instr);

            std::vector<Instruction> displayed_;
            DebuggerHost& host_;
            friend class DebuggerHost;
        };

        DebuggerHost::SourceDisplay::SourceDisplay(DebuggerHost& host, int x, int y, int w, int h)
            : Fl_Text_Display(x, y, w, h),
            host_(host)
        {
        }

        int DebuggerHost::SourceDisplay::handle(int ev)
        {
            if (ev == FL_RELEASE)
            {
                int button = Fl::event_button();
                int x = Fl::event_x();
                int y = Fl::event_y();
                int row = 0;
                int col = 0;

                xy_to_rowcol(x, y, &row, &col);
                std::cout << "Click released at " << row + mVScrollBar->value() - 1 << " " << col << std::endl;
                if (col <= 1)
                    host_.debugger()->setBreakpoint(displayed_[row + mVScrollBar->value()].addr());
                // todo: set breakpoint at location
            }
            int v =  Fl_Text_Display::handle(ev);
            if (ev == FL_MOUSEWHEEL)
            {
                int requiredInstr = mVScrollBar->value() + mNVisibleLines;
                if (requiredInstr > displayed_.size())
                {
                    auto additional = host_.debugger()->getMoreInstructions(requiredInstr - displayed_.size());
                    addInstructions_(additional);
                }
            }
            return v;
        }

        void DebuggerHost::SourceDisplay::setInstructions(const std::vector<Instruction>& ins)
        {
            Fl_Text_Buffer* buf = buffer();

            buf->remove(0, buf->length());
            addInstructions_(ins);
        }

        void DebuggerHost::SourceDisplay::addInstructions_(const std::vector<Instruction>& ins)
        {
            Fl_Text_Buffer* buf = buffer();
            std::stringstream lines;
            for (auto& i : ins)
            {
                lines << "  ";
                lines << "[0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i.addr() << "]";
                lines << " " << i.toString();
                lines << '\n';
            }
            buf->append(lines.str().c_str());
        }



        DebuggerHost::DebuggerHost()
            : Fl_Window(200, 500, "Debugger")
        {
            SourceDisplay* source = new SourceDisplay(*this, 20, 0, 180, 500);
            Fl_Text_Buffer* b = new Fl_Text_Buffer();
            const char *text =
                "0\n"
                "1\n"
                "2\n"
                "3\n"
                "4\n"
                "5\n"
                "6\n"
                "7\n"
                "8\n"
                "9\n"
                "10\n"
                "11\n"
                "12\n"
                "13\n"
                "14\n"
                "15\n"
                "16\n"
                "17\n"
                "18\n"
                "19\n"
                "20\n"
                "21\n"
                "22\n"
                "23\n"
                "24\n"
                "25\n"
                "26\n"
                "27\n"
                "28\n"
                "29\n"
                "30\n"
                "31\n"
                "32\n"
                "33\n"
                "34\n"
                "35\n"
                "36\n"
                "37\n"
                "38\n"
                "39\n"
                "40\n"
                ;
            b->text(text);
            source->buffer(b);
            end();

            srcView_ = source;
        }

        void DebuggerHost::open()
        {
            show();
            //Fl::run();
        }

        void DebuggerHost::pumpEvents()
        {
            while (Fl::wait(0) > 0);
        }
    }

}
