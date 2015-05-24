#ifndef GBONK_CPU_HPP
#define GBONK_CPU_HPP

#include <cstdint>

#include "video/VideoSystem.hpp"
#include "MMU.hpp"
#include "IORegistersRepository.hpp"
#include "ClockScheduler.hpp"
#include "debug/Debugger.hpp"


#define REGPAIR(FIRST, SECOND)                      \
struct _##FIRST##SECOND {                           \
    /* FIRST: most significant*/                    \
    uint8_t FIRST;                                  \
    uint8_t SECOND;                                 \
    _##FIRST##SECOND& operator=(uint16_t v)         \
    {                                               \
        this-> SECOND = v & 0xFF;                   \
        this-> FIRST = v >> 8;                      \
        return *this;                               \
    }                                               \
    operator uint16_t() const                       \
    {                                               \
        return this-> SECOND | (this-> FIRST << 8); \
    }                                               \
    _##FIRST##SECOND operator--(int)                \
    {                                               \
        _##FIRST##SECOND r = *this;                 \
        uint16_t v = *this;                         \
        v--;                                        \
        *this = v;                                  \
        return r;                                   \
    }                                               \
    _##FIRST##SECOND operator++(int)                \
    {                                               \
        _##FIRST##SECOND r = *this;                 \
        uint16_t v = *this;                         \
        v++;                                        \
        *this = v;                                  \
        return r;                                   \
    }                                               \
};                                                  \
_##FIRST##SECOND FIRST##SECOND                      \


namespace GBonk
{
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

        Registers registers_;
        Clock last_instr_;
        Clock globalClock_;

        void prepareLaunch();
        void run();
        void runOne();
        void halt() { }
        void stop() { }

        enum InterruptId
        {
            INT_VBLANK = 0x40,
            INT_LCDC = 0x48,
            INT_TIMER = 0x50,
            INT_SERIAL = 0x58,
            INT_P10 = 0x60,
        };
        void interrupt(InterruptId interrupt);
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

        void ioregWrite(unsigned int& value, uint32_t addr8b);
        void write(unsigned int value, uint32_t addr);
        void writew(unsigned int value, uint32_t addr);
        unsigned int read(uint32_t addr);
        unsigned int readw(uint32_t addr);

    private:
        void cbDrawScanLine_();
        void cbVBlank_();
        void cbTimerOverflow_();

        void launchSequence_();

        friend OpFormat runCurrentOp(CPU&);

        bool interruptMasterEnable_;
        uint32_t interruptsEnabled_;

        IORegistersRepository ioregs_;
        MMU mmu_;
        Video::VideoSystem video_;
        Cartridge* game_;
        ClockScheduler sched_;
        friend class Debug::Debugger;
    };

}

#endif
