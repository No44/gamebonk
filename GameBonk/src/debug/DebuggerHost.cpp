#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstdint>

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>


#include "debug/DebuggerHost.hpp"
#include "debug/Debugger.hpp"

namespace GBonk
{

    namespace Debug
    {

        struct DebuggerHost::RegDisplay
        {
            RegDisplay(int x, int y, const std::string& reg1, const std::string& reg2);

            void setValues(uint8_t& first, uint8_t& second, unsigned int combined);

            static const unsigned int Height;
            std::string labels_[3];
            Fl_Int_Input* reg1_;
            Fl_Int_Input* reg2_;
            Fl_Int_Input* comb_;
            uint8_t* firstVal_;
            uint8_t* secVal_;
        private:
            static void setValue_(Fl_Int_Input*, unsigned int v, int w);
            static void cbInputReg1_(Fl_Widget*, void*);
            static void cbInputReg2_(Fl_Widget*, void*);
            static void cbInputComb_(Fl_Widget*, void*);
        };

        const unsigned int DebuggerHost::RegDisplay::Height = 30;

        DebuggerHost::RegDisplay::RegDisplay(int x, int y, const std::string& r1, const std::string& r2)
        {
            labels_[0] = r1 + ":";
            reg1_ = new Fl_Int_Input(x, y, 40, 25, labels_[0].c_str());
            reg1_->labelfont(FL_COURIER);
            reg1_->textfont(FL_COURIER);
            reg1_->value("0");
            reg1_->maximum_size(4);
            reg1_->callback(&cbInputReg1_, this);

            labels_[1] = r2 + ":";
            reg2_ = new Fl_Int_Input(x + 65, y, 40, 25, labels_[1].c_str());
            reg2_->labelfont(FL_COURIER);
            reg2_->textfont(FL_COURIER);
            reg2_->value("0");
            reg2_->maximum_size(4);
            reg2_->callback(&cbInputReg2_, this);

            labels_[2] = (r1 + r2) + ":";
            comb_ = new Fl_Int_Input(x + 140, y, 60, 25, labels_[2].c_str());
            comb_->labelfont(FL_COURIER);
            comb_->textfont(FL_COURIER);
            comb_->value("0");
            comb_->maximum_size(6);
            comb_->callback(&cbInputComb_, this);
        }

        void DebuggerHost::RegDisplay::cbInputReg1_(Fl_Widget*, void* r)
        {
            DebuggerHost::RegDisplay* rd = static_cast<DebuggerHost::RegDisplay*>(r);
            std::stringstream s;
            unsigned int v = 0;

            s << std::hex << rd->reg1_->value();
            s >> v;
            *(rd->firstVal_) = v;
            setValue_(rd->comb_, (*(rd->firstVal_) << 8) | *(rd->secVal_), 4);
        }

        void DebuggerHost::RegDisplay::cbInputReg2_(Fl_Widget*, void* r)
        {
            DebuggerHost::RegDisplay* rd = static_cast<DebuggerHost::RegDisplay*>(r);
            std::stringstream s;
            unsigned int v = 0;

            s << std::hex << rd->reg2_->value();
            s >> v;
            *(rd->secVal_) = v;
            setValue_(rd->comb_, (*(rd->firstVal_) << 8) | *(rd->secVal_), 4);
        }

        void DebuggerHost::RegDisplay::cbInputComb_(Fl_Widget*, void* r)
        {
            DebuggerHost::RegDisplay* rd = static_cast<DebuggerHost::RegDisplay*>(r);
            std::stringstream s;
            unsigned int v = 0;

            s << std::hex << rd->comb_->value();
            s >> v;
            *(rd->firstVal_) = v >> 8;
            *(rd->secVal_) = v & 0xFF;
            setValue_(rd->reg1_, *(rd->firstVal_), 2);
            setValue_(rd->reg2_, *(rd->secVal_), 2);
        }

        void DebuggerHost::RegDisplay::setValues(uint8_t& f, uint8_t& s, unsigned int c)
        {
            setValue_(reg1_, f, 2);
            firstVal_ = &f;
            setValue_(reg2_, s, 2);
            secVal_ = &s;
            setValue_(comb_, c, 4);
        }

        void DebuggerHost::RegDisplay::setValue_(Fl_Int_Input* widget, unsigned int v, int w)
        {
            std::stringstream builder;

            builder << std::uppercase << std::setw(w) << std::setfill('0') << std::hex << v;
            widget->value(builder.str().c_str());
        }


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
            //setCurrent(ins[0]);
        }

        void DebuggerHost::SourceDisplay::setCurrent(const Instruction& ins)
        {
            Fl_Text_Buffer* buf = buffer();
            auto it = std::lower_bound(displayed_.begin(), displayed_.end(), std::make_pair(0, ins),
                [](const PosdIns& a, const PosdIns& b) {return a.second.addr() < b.second.addr(); });
            assert(it->second.addr() == ins.addr());
            
            if (currentInsIdx_ != -1)
            {
                PosdIns& current = displayed_[currentInsIdx_];
                if (host_.dbg_->isBreakpointSet(current.second.addr()))
                    buf->replace(current.first, current.first + 1, "O");
                else
                    buf->replace(current.first, current.first + 1, " ");
            }
            buf->replace(it->first, it->first + 1, ">");
            currentInsIdx_ = it - displayed_.begin();
            if (currentInsIdx_ < mVScrollBar->value() - 1
                || currentInsIdx_ >= mVScrollBar->value() - 1 + Lines)
                scroll(currentInsIdx_ + 1, 0);
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


        static void stepCb_(Fl_Widget*, void* h)
        {
            DebuggerHost* host = static_cast<DebuggerHost*>(h);

            host->debugger()->step();
        }

        static void contCb_(Fl_Widget*, void* h)
        {
            DebuggerHost* host = static_cast<DebuggerHost*>(h);

            host->disable();
            host->debugger()->cont();
        }

        static void pauseCb_(Fl_Widget*, void* h)
        {
            DebuggerHost* host = static_cast<DebuggerHost*>(h);

            host->enable();
            host->debugger()->brk();
        }

        void DebuggerHost::cbSeek_(Fl_Widget*, void* h)
        {
            DebuggerHost* host = static_cast<DebuggerHost*>(h);
            unsigned int addr = 0;
            std::stringstream s;

            s << std::hex << host->seek_->value();
            s >> addr;
            host->dbg_->loadFrom(addr);
            host->srcView_->setInstructions(host->dbg_->getMoreInstructions(40));
        }

        template<CPUFlags FLAG>
        void DebuggerHost::checkBttnToggleFlag_(Fl_Widget*, void* h)
        {
            DebuggerHost* host = static_cast<DebuggerHost*>(h);

            host->dbg_->toggleFlag(FLAG);
        }

        DebuggerHost::DebuggerHost()
            : Fl_Window(Width, Height, "Debugger")
        {
            SourceDisplay* source = new SourceDisplay(*this, 20, 5, 400, 495);
            source->hide_cursor();
            Fl_Text_Buffer* b = new Fl_Text_Buffer();
            source->buffer(b);
            source->textfont(FL_COURIER);
            
            seek_ = new Fl_Int_Input(65, 500, 75, 25, "Seek:");
            seek_->labelfont(FL_COURIER);
            seek_->textfont(FL_COURIER);
            seek_->maximum_size(6);
            seek_->callback(&DebuggerHost::cbSeek_, this);

            Fl_Button* buttonStep = new Fl_Button(20, 530, 100, 30, "Step");
            buttonStep->shortcut(FL_F + 10);
            buttonStep->callback(stepCb_, this);
            buttonStep->labelfont(FL_COURIER);

            Fl_Button* buttonContinue = new Fl_Button(125, 530, 100, 30, "Continue");
            buttonContinue->shortcut(FL_F + 5);
            buttonContinue->callback(contCb_, this);
            buttonContinue->labelfont(FL_COURIER);

            Fl_Button* buttonPause = new Fl_Button(230, 530, 100, 30, "Pause");
            buttonPause->callback(pauseCb_, this);
            buttonPause->deactivate();
            buttonPause->labelfont(FL_COURIER);
            
            af_ = new RegDisplay(440, 5, "A", "F");
            af_->comb_->hide();
            flagsBox_ = new Fl_Box(390, RegDisplay::Height - 15, 100, 50, "Flags:");
            flagsBox_->labelfont(FL_COURIER);
            flagsBox_->labelsize(10);
            static const CPUFlags flagsv[4] = {CPUFlags::Z, CPUFlags::N, CPUFlags::H, CPUFlags::C};
            static const char* flagsl[4] = { "Z", "N", "H", "C" };
            static const Fl_Callback_p cbs[4] = {&DebuggerHost::checkBttnToggleFlag_<CPUFlags::Z>,
                &DebuggerHost::checkBttnToggleFlag_<CPUFlags::N>,
                &DebuggerHost::checkBttnToggleFlag_<CPUFlags::H>, 
                &DebuggerHost::checkBttnToggleFlag_<CPUFlags::C>};
            for (int i = 0, posx = 460; i < 4; ++i, posx += 25)
            {
                flagsBttn_[flagsv[i]] = new Fl_Check_Button(posx, RegDisplay::Height + 6, 10, 10, flagsl[i]);
                flagsBttn_[flagsv[i]]->labelfont(FL_COURIER);
                flagsBttn_[flagsv[i]]->labelsize(10);
                flagsBttn_[flagsv[i]]->callback(cbs[i], this);
            }

            bc_ = new RegDisplay(440, 25 + RegDisplay::Height, "B", "C");
            de_ = new RegDisplay(440, 25 + RegDisplay::Height*2, "D", "E");
            hl_ = new RegDisplay(440, 25 + RegDisplay::Height*3, "H", "L");
            
            end();
            srcView_ = source;
            bttnStep_ = buttonStep;
            bttnCont_ = buttonContinue;
            bttnPause_ = buttonPause;
        }

        DebuggerHost::~DebuggerHost()
        {
            delete af_;
            delete bc_;
            delete de_;
            delete hl_;
        }

        void DebuggerHost::debugger(Debugger& d)
        {
            dbg_ = &d;
        }


        void DebuggerHost::updateRegisters(CPU::Registers& regs)
        {
            activeRegisters_ = &regs;

            af_->setValues(regs.AF.A, regs.AF.F, regs.AF);
            flagsBttn_[CPUFlags::Z]->value(!!(regs.AF.F & static_cast<unsigned int>(CPUFlags::Z)));
            flagsBttn_[CPUFlags::N]->value(!!(regs.AF.F & static_cast<unsigned int>(CPUFlags::N)));
            flagsBttn_[CPUFlags::H]->value(!!(regs.AF.F & static_cast<unsigned int>(CPUFlags::H)));
            flagsBttn_[CPUFlags::C]->value(!!(regs.AF.F & static_cast<unsigned int>(CPUFlags::C)));

            bc_->setValues(regs.BC.B, regs.BC.C, regs.BC);
            de_->setValues(regs.DE.D, regs.DE.E, regs.DE);
            hl_->setValues(regs.HL.H, regs.HL.L, regs.HL);
        }

        void DebuggerHost::open()
        {
            show();
        }

        void DebuggerHost::pumpEvents()
        {
            int v;
            while ((v = Fl::wait(0)) > 0);
                //std::cout << "ev:" << v;
        }

        void DebuggerHost::disable()
        {
            srcView_->deactivate();
            bttnStep_->deactivate();
            bttnCont_->deactivate();
            bttnPause_->activate();
        }

        void DebuggerHost::enable()
        {
            srcView_->activate();
            bttnStep_->activate();
            bttnCont_->activate();
            bttnPause_->deactivate();
        }

    }

}
