#ifndef GBONK_CPU_HPP
#define GBONK_CPU_HPP

#include <cstdint>

#include "video/VideoSystem.hpp"
#include "MMU.hpp"

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

        union RegPair{
            struct {
                uint8_t first;
                uint8_t second;
            };
            uint16_t pair;
        };
        struct Registers
        {
            RegPair AF;
            RegPair BC;
            RegPair DE;
            RegPair HL;
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

        Cartridge* game_;
        MMU mmu_;
        Video::VideoSystem video_;
    };

}

#endif