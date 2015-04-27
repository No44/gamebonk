#ifndef GBONK_CPU_HPP
#define GBONK_CPU_HPP

#include <cstdint>

#include "video/VideoSystem.hpp"
#include "MMU.hpp"


#define REGPAIR(FIRST, SECOND) union { \
       struct {               \
         uint8_t FIRST;       \
         uint8_t SECOND;      \
       };                     \
       uint16_t  pair;        \
                              \
} FIRST##SECOND               \


namespace GBonk
{
    // clock speed:4.2 MHZ
    // HSYNC: 9200KHz
    // VSYNC: 60HZ

    class Cartridge;

    class CPU
    {
    public:
        CPU();

        void load(Cartridge&);

        struct Registers
        {
            REGPAIR(A,F);
            REGPAIR(B,C);
            REGPAIR(D,E);
            REGPAIR(H,L);
            uint16_t sp;
            uint16_t pc;
        };
        struct Clock {
            uint64_t m;
            uint64_t t;
        };

        static const unsigned int InterruptPeriod = 0;

        Registers registers_;
        Clock last_instr_;
        Clock globalClock_;

        void run();
        void write(unsigned int value, uint32_t addr);

    private:
        void launchSequence_();

        static constexpr unsigned int F_ZEROFLAG = 1 << 7;
        static constexpr unsigned int F_SUBFLAG = 1 << 6;
        static constexpr unsigned int F_HCARRY = 1 << 5;
        static constexpr unsigned int F_CARRY = 1 << 4;
        static constexpr unsigned int F_VALUEPART = F_CARRY - 1;

        // returns instruction's length.
        // should be added to PC to get the next instruction.
        struct OpFormat {
          unsigned int cycles;
          unsigned int bytes;
        };

        friend OpFormat runCurrentOp(CPU&);

        Cartridge* game_;
        MMU mmu_;
        Video::VideoSystem video_;
    };

}

#endif
