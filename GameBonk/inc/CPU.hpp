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
        // returns instruction's length.
        // should be added to PC to get the next instruction.
        struct OpFormat {
            unsigned int cycles;
            unsigned int bytes;
        };

        static const unsigned int InterruptPeriod = 0;

        Registers registers_;
        Clock last_instr_;
        Clock globalClock_;

        void run();
        void write(unsigned int value, uint32_t addr);
        void writew(unsigned int value, uint32_t addr);
        unsigned int read(uint32_t addr);
        unsigned int readw(uint32_t addr);

    private:
        void launchSequence_();

        enum class Flags
        {
          Z = 1 << 7, // zero
          N = 1 << 6, // subtract
          H = 1 << 5, // half-carry
          C = 1 << 4, // carry
        };

        friend OpFormat runCurrentOp(CPU&);

        Cartridge* game_;
        MMU mmu_;
        Video::VideoSystem video_;
    };

}

#endif
