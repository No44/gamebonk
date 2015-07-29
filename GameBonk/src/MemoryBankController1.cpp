#include <iostream>
#include <iomanip>

#include "Cartridge.hpp"
#include "MemoryBankController1.hpp"

namespace GBonk
{

    MBC1::MemoryBankController1(const Cartridge& c)
        : AMBC(c),
        modeROM_(true),
        romOffset_(0x4000),
        ramOffset_(0)
    {
    }

    unsigned int MBC1::read(uint32_t addr) const
    {
        switch (addr & 0xF000)
        {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
            return cartridgeROM_[addr];
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            return cartridgeROM_[romOffset_ + (addr & 0x3FFF)];
        case 0xA000:
        case 0xB000:
            return externalRAM_[ramOffset_ + (addr & 0x1FFF)];
        default:
            std::cerr << "MBC1 shouldn't have to read location " << std::hex << addr << std::endl;
            abort();
        }
        return 0;
    }

    unsigned int MBC1::readw(uint32_t addr) const
    {
        unsigned int value = 0;
        unsigned int baseaddr = 0;

        switch (addr & 0xF000)
        {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
            return cartridgeROM_[addr] | (cartridgeROM_[addr + 1] << 8);
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            baseaddr = romOffset_ + (addr & 0x3FFF);
            value =  (cartridgeROM_[baseaddr + 1] << 8) | cartridgeROM_[baseaddr];
            return value;
        case 0xA000:
        case 0xB000:
            baseaddr = ramOffset_ + (addr & 0x1FFF);
            value = (externalRAM_[baseaddr + 1] << 8) | externalRAM_[baseaddr];
            return value;
        default:
            std::cerr << "MBC1 shouldn't have to readw location " << std::hex << addr << std::endl;
            abort();
        }
        return 0;
    }

    unsigned int MBC1::write(unsigned int val, uint32_t addr)
    {
        unsigned int old = 0;
        switch (addr & 0xF000)
        {
        case 0x0000:
        case 0x1000:
            old = enableRAMBank_;
            enableRAMBank_ = (val & 0x0F) == 0x0A;
            break;
        case 0x2000:
        case 0x3000:
        {
            // ROM bank select
            old = romOffset_ / 0x4000;
            unsigned int bank = val & 0x1F;
            switch (bank)
            {
            case 0:
            case 1:
                bank = 1;
            default:
                romOffset_ = bank * 0x4000;
                break;
            };
            break;
        }
        case 0x4000:
        case 0x5000: 
        {
            int bank = val & 3;
            if (modeROM_ == false)
                ramOffset_ = bank * 0x2000;
            else
            {
                int romBank = romOffset_ / 0x4000;
                romBank = romBank & 0x1F + (bank << 5);
                romOffset_ = romBank * 0x4000;
            }
            break;
        }
        case 0x6000:
        case 0x7000:
        {
            // mode switch
            int mode = val & 1;
            if (mode)
                modeROM_ = false;
            else
                modeROM_ = true;
            break;
        }
        case 0xA000:
        case 0xB000:
            old = externalRAM_[ramOffset_ + addr & 0x1FFF];
            if (enableRAMBank_)
                externalRAM_[ramOffset_ + addr & 0x1FFF] = val;
            break;
        default:
            std::cerr << "MBC1 shouldn't have to write to location " << std::hex << addr << std::endl;
            abort();
        }
        return old;
    }

    unsigned int MBC1::writew(unsigned int val, uint32_t addr)
    {
        unsigned int old = 0;
        switch (addr & 0xF000)
        {
        case 0x0000:
        case 0x1000:
            old = enableRAMBank_;
            enableRAMBank_ = (val & 0x0F) == 0x0A;
            break;
        case 0x2000:
        case 0x3000:
        {
            // ROM bank select
            unsigned int old = romOffset_ / 0x4000;
            unsigned int bank = val & 0x1F;
            switch (bank)
            {
            case 0:
            case 1:
                bank = 1;
            default:
                romOffset_ = bank * 0x4000;
                break;
            };
            break;
        }
        case 0x4000:
        case 0x5000:
        {
            int bank = val & 3;
            if (modeROM_ == false)
                ramOffset_ = bank * 0x2000;
            else
            {
                int romBank = romOffset_ / 0x4000;
                romBank = romBank & 0x1F + (bank << 5);
                romOffset_ = romBank * 0x4000;
            }
            break;
        }
        case 0x6000:
        case 0x7000:
        {
            // mode switch
            int mode = val & 1;
            if (mode)
                modeROM_ = false;
            else
                modeROM_ = true;
            break;
        }
        case 0xA000:
        case 0xB000:
        {
            if (!enableRAMBank_)
                break;
            uint32_t baseAddr = ramOffset_ + (addr & 0x1FFF);
            old = externalRAM_[baseAddr + 1] << 8 | externalRAM_[baseAddr];
            externalRAM_[baseAddr] = val & 0xFF;
            externalRAM_[baseAddr + 1] = (val & 0xFF00) >> 8;
            break;
        }
        default:
            std::cerr << "MBC1 shouldn't have to writew to location " << std::hex << addr << std::endl;
            abort();
        }
        return old;
    }
}
