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
        unsigned int byte = 0;

        switch (addr & 0xF000)
        {
            /*
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
            byte = cartridgeROM_[addr];
            break;
            */
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            byte = cartridgeROM_[romOffset_ + (addr & 0x3FFF)];
            break;
        case 0xA000:
        case 0xB000:
            byte = externalRAM_[ramOffset_ + (addr & 0x1FFF)];
            break;
        }
        return byte;
    }

    unsigned int MBC1::readw(uint32_t addr) const
    {
        unsigned int byte = 0;

        switch (addr & 0xF000)
        {
            /*
            case 0x0000:
            case 0x1000:
            case 0x2000:
            case 0x3000:
            byte = cartridgeROM_[addr];
            break;
            */
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            byte = cartridgeROM_[romOffset_ + (addr & 0x3FFF)];
            break;
        case 0xA000:
        case 0xB000:
            byte = externalRAM_[ramOffset_ + (addr & 0x1FFF)];
            break;
        }
        return byte;
    }

    void MBC1::write(unsigned int val, uint32_t addr)
    {
        switch (addr & 0xF000)
        {
        case 0x0000:
        case 0x1000:
            enableRAMBank_ = (val & 0x0F) == 0x0A;
            break;
        case 0x2000:
        case 0x3000:
            {
                // ROM bank select
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
        // todo: RAM writes
        }
    }



}


/*
if (mode)
{
mmodeROM_ = (512 * (1 << 10)); // 4Mbit
mmodeRAM_ = 32 * (1 << 10); // 32Kbyte
}
else
{
mmodeROM_ = (32 * (1 << 10)) << 6; // 16Mbit
mmodeRAM_ = 8 * (1 << 10); //8Kbyte
}

*/