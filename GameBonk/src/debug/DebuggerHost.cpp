#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include <FL/Fl_Text_Buffer.H>



#include "debug/DebuggerHost.hpp"

namespace GBonk
{

    namespace Debug
    {

        DebuggerHost::SourceDisplay::SourceDisplay(DebuggerHost& host, int x, int y, int w, int h)
            : Fl_Text_Display(x, y, w, h),
            host_(host),
            currentInsIdx_(-1)
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
                //std::cout << "Click released at " << row + mVScrollBar->value() - 1 << " " << col << std::endl;
                if (col <= 1 && row < displayed_.size())
                {
                    int insIdx = row + mVScrollBar->value() - 1;
                    PosdIns& ins = displayed_[insIdx];
                    int pos = line_start(xy_to_position(x, y));
                    if (!host_.debugger()->isBreakpointSet(ins.second.addr()))
                    {
                        host_.debugger()->setBreakpoint(ins.second.addr());
                        if (currentInsIdx_ == -1 || displayed_[currentInsIdx_].first != pos)
                            buffer()->replace(pos, pos + 1, "O");
                    }
                    else
                    {
                        host_.debugger()->disableBreakpoint(ins.second.addr());
                        if (currentInsIdx_ == -1 || displayed_[currentInsIdx_].first != pos)
                            buffer()->replace(pos, pos + 1, " ");
                    }
                    
                }
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
            currentInsIdx_ = -1;
            displayed_.clear();
            addInstructions_(ins);
            setCurrent(ins[0]);
        }

        void DebuggerHost::SourceDisplay::setCurrent(const Instruction& ins)
        {
            Fl_Text_Buffer* buf = buffer();
            auto it = std::lower_bound(displayed_.begin(), displayed_.end(), std::make_pair(0, ins),
                [](const PosdIns& a, const PosdIns& b) {return a.second.addr() < b.second.addr(); });
            assert(it->second.addr() == ins.addr());
            
            buf->replace(it->first, it->first + 1, ">");
            if (currentInsIdx_ != -1)
            {
                PosdIns& current = displayed_[currentInsIdx_];
                if (host_.dbg_->isBreakpointSet(current.second.addr()))
                    buf->replace(current.first, current.first + 1, "O");
                else
                    buf->replace(current.first, current.first + 1, " ");
            }
            currentInsIdx_ = it - displayed_.begin();
        }

        bool DebuggerHost::SourceDisplay::displays(const Instruction& ins) const
        {
            return std::binary_search(displayed_.begin(), displayed_.end(), std::make_pair(0, ins),
                [](const PosdIns& a, const PosdIns& b)
            {
                return a.second.addr() < b.second.addr();
            });
        }

        void DebuggerHost::SourceDisplay::addInstructions_(const std::vector<Instruction>& ins)
        {
            Fl_Text_Buffer* buf = buffer();
            std::stringstream lines;
            int posLine = 0;

            for (auto& i : ins)
            {
                displayed_.push_back(std::make_pair(posLine, i));

                std::stringstream line;
                line << "  ";
                line << "[0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i.addr() << "]";
                line << " " << i.toString();
                line << '\n';
                posLine += line.str().length();

                lines << line.str();
            }
            buf->append(lines.str().c_str());
        }



        DebuggerHost::DebuggerHost()
            : Fl_Window(Width, Height, "Debugger")
        {
            SourceDisplay* source = new SourceDisplay(*this, 20, 5, 400, 495);
            Fl_Text_Buffer* b = new Fl_Text_Buffer();
            source->buffer(b);
            source->textfont(FL_COURIER);
            end();
            srcView_ = source;
        }

        void DebuggerHost::debugger(Debugger& d)
        {
            dbg_ = &d;
            srcView_->setInstructions(d.getMoreInstructions(40));
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
