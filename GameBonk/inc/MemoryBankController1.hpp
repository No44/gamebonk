#ifndef GBONK_MBC_HPP
#define GBONK_MBC_HPP

#include <cstdint>

#include "AMemoryBankController.hpp"

namespace GBonk
{
    class Cartridge;

    class MemoryBankController1 : public AMBC
    {
    public:
        MemoryBankController1(const Cartridge&);
        MemoryBankController1(MemoryBankController1&&);

        void write(unsigned int val, uint32_t addr) override;
        unsigned int read(uint32_t addr) const override;
        unsigned int readw(uint32_t addr) const override;

    private:
        bool modeROM_;
        uint32_t romOffset_;
        uint32_t ramOffset_;
        bool enableRAMBank_;
    };

    typedef MemoryBankController1 MBC1;
}


#endif