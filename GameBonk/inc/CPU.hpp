#ifndef GBONK_CPU_HPP
#define GBONK_CPU_HPP

#include <cstdint>

#include "video/VideoSystem.hpp"
#include "MMU.hpp"
#include "IORegistersRepository.hpp"


#define REGPAIR(FIRST, SECOND) union { \
       struct {               \
         uint8_t SECOND;      \
         uint8_t FIRST;       \
       };                     \
       uint16_t  pair;        \
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

        enum class Flags : unsigned int
        {
            Z = 1 << 7, // zero
            N = 1 << 6, // subtract
            H = 1 << 5, // half-carry
            C = 1 << 4, // carry
        };
        struct Registers
        {
            REGPAIR(A, F);
            REGPAIR(B, C);
            REGPAIR(D, E);
            REGPAIR(H, L);
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

        static const unsigned int CLOCK_SPEED = 4194304;
        static const unsigned int HORIZ_SYNC = 0;
        static const unsigned int InterruptPeriod = 0;

        Registers registers_;
        Clock last_instr_;
        Clock globalClock_;

        void run();
        void halt() { }
        void stop() { }
        // interrupts will be disabled after the next
        // instruction is ran
        void prepareDisableInterrupts() {}
        // immediatly disable interrupts
        void disableInterrupts();
        // interrupts will be enabled after the next
        // instruction is ran
        void prepareEnableInterrupts() {}
        // immediatly enable interrupts
        void enableInterrupts();

        void write(unsigned int value, uint32_t addr);
        void writew(unsigned int value, uint32_t addr);
        unsigned int read(uint32_t addr);
        unsigned int readw(uint32_t addr);

    private:
        void launchSequence_();

        friend OpFormat runCurrentOp(CPU&);

        bool interruptMasterEnable_;
        uint32_t interruptsEnabled_;

        IORegistersRepository ioregs_;
        MMU mmu_;
        Video::VideoSystem video_;
        Cartridge* game_;
    };

}

#endif
