#ifndef GBONK_AMBC_HPP
#define GBONK_AMBC_HPP

#include <cstdint>
#include <string>

namespace GBonk
{

    class Cartridge;

    // Only takes care of:
    // - writing memory with special effects
    // - reading from RAM/ROM banks
    class AMemoryBankController
    {
    public:
        enum Type
        {
            Type_ROM_ONLY = 0,
            Type_ROM_MBC1 = 1,
            Type_ROM_MBC1_RAM = 2,
            Type_ROM_MBC1_RAM_BATT = 3,
            Type_ROM_MBC2 = 5,
            Type_ROM_MBC2_BATT = 6,
            Type_ROM_RAM = 8,
            Type_ROM_RAM_BATT = 9,
            Type_ROM_MMMD1 = 0xB,
            Type_ROM_MMMD1_SRAM = 0xC,
            Type_ROM_MMMD1_SRAM_BATT = 0xD,
            Type_ROM_MBC3_TIMER_BATT = 0xF,
            Type_ROM_MBC3_TIMER_RAM_BATT = 0x10,
            Type_ROM_MBC3 = 0x11,
            Type_ROM_MBC3_RAM = 0x12,
            Type_ROM_MBC3_RAM_BATT = 0x13,
            Type_ROM_MBC5 = 0x19,
            Type_ROM_MBC5_RAM = 0x1A,
            Type_ROM_MBC5_RAM_BATT = 0x1B,
            Type_ROM_MBC5_RUMBLE = 0x1C,
            Type_ROM_MBC5_RUMBLE_SRAM = 0x1D,
            Type_ROM_MBC5_RUMBLE_SRAM_BATT = 0x1E,
            Type_POCKET_CAM = 0x1F,
            Type_BANDAI_TAMA5 = 0xFD,
            Type_HUDSON_HUC3 = 0xFE,
            Type_HUDSON_HUC1 = 0xFF,
        };

        AMemoryBankController(const Cartridge&);
        virtual ~AMemoryBankController();

        Type type() const { return type_; }

        virtual void write(unsigned int val, uint32_t addr) = 0;
        virtual void writew(unsigned int val, uint32_t addr) = 0;
        virtual unsigned int read(uint32_t addr) const = 0;
        virtual unsigned int readw(uint32_t addr) const = 0;

        static std::string typeToString(Type);
        static AMemoryBankController* makeMBC(const Cartridge&);

    protected:
        Type type_;
        const uint8_t* cartridgeROM_;
        uint8_t* externalRAM_;
    };

    typedef AMemoryBankController AMBC;
}

#endif