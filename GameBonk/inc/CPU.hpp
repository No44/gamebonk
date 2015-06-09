#ifndef GBONK_CPU_HPP
#define GBONK_CPU_HPP

#include <cstdint>
#include <chrono>

#include "video/VideoSystem.hpp"
#include "MMU.hpp"
#include "IORegistersRepository.hpp"
#include "ClockScheduler.hpp"
//#include "debug/Debugger.hpp"


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
    namespace Debug
    {
        class Debugger;
    }

    class Cartridge;

    enum class CPUFlags : unsigned int
    {
        Z = 1 << 7, // zero
        N = 1 << 6, // subtract
        H = 1 << 5, // half-carry
        C = 1 << 4, // carry
    };

    class CPU
    {
    public:
        static const unsigned int CPU_FREQ;
        static const unsigned int CYCLES_PER_MSEC;

        CPU();

        void load(Cartridge&);

        struct Registers
        {
            REGPAIR(A, F);
            REGPAIR(B, C);
            REGPAIR(D, E);
            REGPAIR(H, L);
            uint16_t sp;
            uint16_t pc;
        };

        // returns instruction's length.
        // should be added to PC to get the next instruction.
        struct OpFormat {
            unsigned int cycles;
            unsigned int bytes;
        };

        Registers registers_;


        void prepareLaunch();
        void run();
        void runOne();
        void runDuring(unsigned int cycles);
        void halt() { /*todo*/ }
        void stop() { /*todo*/ }

        enum Keys
        {
            KEY_RIGHT = 0,
            KEY_LEFT,
            KEY_UP,
            KEY_DOWN,
            KEY_A,
            KEY_B,
            KEY_SELECT,
            KEY_START
        };

        enum InterruptId
        {
            INT_VBLANK = 0,
            INT_LCDC,
            INT_TIMER,
            INT_SERIAL,
            INT_P10
        };
        static const unsigned int InterruptAddr[INT_P10 + 1];
        static const unsigned int InterruptFlag[INT_P10 + 1];

        void interrupt(InterruptId interrupt);

        // immediatly disable interrupts
        void disableInterrupts();
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
        void cbTimer_();
        void cbDiv_();

        void launchSequence_();

        friend OpFormat runCurrentOp(CPU&);

        bool interruptMasterEnable_;

        IORegistersRepository ioregs_;
        MMU mmu_;
        Video::VideoSystem video_;
        Cartridge* game_;
        ClockScheduler sched_;

        enum KeyOutPortIdx{
            P14 = 0,
            P15 = 1,
        };
        static const unsigned int KeyInputPortIdcs[2];
        std::array<bool, 8> keys_;
        std::chrono::steady_clock::time_point frameTime_;
        friend class Debug::Debugger;

    private:
        // IORegisters
        uint8_t* P1_; // FF00
        uint8_t* DIV_; // FF04
        uint8_t* TIMA_; // FF05
        uint8_t* TMA_; // FF06
        uint8_t* TAC_; // FF07
        
        uint8_t* IF_; // FF0F
        uint8_t* STAT_; // ff41
        uint8_t* LY_; // FF44, current line
        uint8_t* IE_; // FFFF
    };

}

#endif
