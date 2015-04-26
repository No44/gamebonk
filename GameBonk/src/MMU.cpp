#include "MMU.hpp"

namespace GBonk
{

    MMU::MMU()
        : systemMem_(new uint8_t[TOTAL_ADDRESSABLE_MEMORY + 1])
    {
        std::memset(systemMem_, 0, TOTAL_ADDRESSABLE_MEMORY + 1);
    }

    MMU::~MMU()
    {
        delete[] systemMem_;
        delete mbc_;
    }

    void MMU::write(unsigned int value, uint32_t addr)
    {
        switch (addr & 0xF000)
        {
        // Writes to ROM area access MBC registers.
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        // Writes to switchable RAM bank need to take RAM offset
        // into account.
        case 0xA000:
        case 0xB000:
            mbc_->write(value, addr);
            return;
        case 0xC000:
        case 0xD000:
            // need to echo
            systemMem_[addr] = value;
            if (addr < 0xDDFF)
                systemMem_[addr + 0x2000] = value;
            return;
        default:
            // todo: a garder ?
            systemMem_[addr] = value;
            return;
        };
    }

    unsigned int MMU::read(uint32_t addr)
    {
        switch (addr & 0xF000)
        {
        // Reads to switchable ROM/RAM banks need to consider
        // associated offsets.
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        case 0xA000:
        case 0xB000:
            return mbc_->read(addr);
        default:
            return systemMem_[addr];
        };
    }

    void MMU::setMBC(AMBC* mbc)
    {
        mbc_ = mbc;
    }

    void MMU::rawWrite(unsigned int val, uint32_t addr)
    {
        systemMem_[addr] = val;
    }

}