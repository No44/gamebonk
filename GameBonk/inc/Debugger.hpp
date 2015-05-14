#ifndef _GBONK_DEBUGGER_HPP_
#define _GBONK_DEBUGGER_HPP_

namespace GBonk
{

    class CPU;

    class Disassembler
    {
    public:
        Disassembler(CPU&);

        void step();


    private:
        CPU& cpu_;
    };

}

#endif
