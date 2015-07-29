#include <cstring>
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

    unsigned int MMU::write(unsigned int value, uint32_t addr)
    {
        unsigned int old = 0;
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
            old = mbc_->write(value, addr);
            break;
        case 0xC000:
        case 0xD000:
            // need to echo
            old = systemMem_[addr];
            systemMem_[addr] = value;
            if (addr < 0xDDFF)
                systemMem_[addr + 0x2000] = value;
            break;
        case 0x8000:
        case 0x9000:
        default:
            old = systemMem_[addr];
            systemMem_[addr] = value;
            break;
        };
        return old;
    }

    unsigned int MMU::writew(unsigned int value, uint32_t addr)
    {
        unsigned int old = 0;
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
            old = mbc_->write(value, addr);
            break;
        case 0xC000:
        case 0xD000:
            // need to echo
            old = systemMem_[addr + 1] << 8 | systemMem_[addr];
            systemMem_[addr] = value & 0xFF;
            systemMem_[addr + 1] = (value & 0xFF00) >> 8;
            if (addr < 0xDDFF)
            {
                systemMem_[addr] = value & 0xFF;
                systemMem_[addr + 1] = (value & 0xFF00) >> 8;
            }
            break;
        case 0x8000:
        case 0x9000:
        default:
            old = systemMem_[addr + 1] << 8 | systemMem_[addr];
            systemMem_[addr] = value & 0xFF;
            systemMem_[addr + 1] = (value & 0xFF00) >> 8;
            break;
        }
        return old;
    }

    unsigned int MMU::read(uint32_t addr)
    {
        switch (addr & 0xF000)
        {
        // Reads to switchable ROM/RAM banks need to consider
        // associated offsets.
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        case 0xA000:
        case 0xB000:
            return mbc_->read(addr);
        default:
            return systemMem_[addr];
        }
    }

    unsigned int MMU::readw(uint32_t addr)
    {
        switch (addr & 0xF000)
        {
            // Reads to switchable ROM/RAM banks need to consider
            // associated offsets.
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        case 0xA000:
        case 0xB000:
            return mbc_->readw(addr);
        default:
            return systemMem_[addr] | (systemMem_[addr + 1] << 8);
        }
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
