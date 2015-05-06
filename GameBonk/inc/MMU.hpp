#ifndef GBONK_MMU_HPP
#define GBONK_MMU_HPP

#include <cstdint>

#include "MemoryBankController1.hpp"

namespace GBonk
{

    // Memory Management Unit
    // It's a Unit that manages memory
    class MMU
    {
    public:
        MMU();
        ~MMU();

        static const uint32_t TOTAL_ADDRESSABLE_MEMORY = 0xFFFF;
        static const uint32_t INTERNAL_RAM_WTF = 0xFF80;
        static const uint32_t IO_PORTS = 0xFF00;
        static const uint32_t SPRITE_ATTRIB_MEMORY = 0xFE00;
        static const uint32_t INTERNAL_RAM_ECHO = 0xE000;
        static const uint32_t INTERNAL_RAM = 0xC000;
        static const uint32_t SWITCHABLE_RAM_BANK = 0xA000;
        static const uint32_t VIDEO_RAM = 0x8000;
        static const uint32_t SWITCHABLE_ROM_BANK = 0x4000;
        static const uint32_t ROM_BANK_0 = 0x0;
        
        void setMBC(AMBC* mbc);
        void rawWrite(unsigned int value, uint32_t addr);
        void write(unsigned int value, uint32_t addr);
        void writew(unsigned int value, uint32_t addr);
        unsigned int read(uint32_t addr);
        unsigned int readw(uint32_t addr);

        uint8_t* memory() { return systemMem_; }

    private:
        uint8_t* systemMem_;
        AMBC* mbc_;
    };

}

#endif
