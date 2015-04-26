

#include "MMU.hpp"
#include "CPU.hpp"
#include "Cartridge.hpp"

namespace GBonk
{

    CPU::CPU()
        : video_(mmu_.memory() + MMU::VIDEO_RAM, mmu_.memory() + MMU::SPRITE_ATTRIB_MEMORY)
    {
    }

    void CPU::load(Cartridge& game)
    {
        game_ = &game;
    }

    void CPU::run()
    {
        launchSequence_();
        mmu_.setMBC(GBonk::AMBC::makeMBC(*game_));

        int counter = CPU::InterruptPeriod;
        uint16_t& PC = registers_.pc;

        for (;;)
        {
            // Read and run operation at PC
            // decrement COUNTER from the amount of
            // cycles the operation last

            if (counter <= 0)
            {
                // run interrupts
                counter += CPU::InterruptPeriod;
                // if (exitRequested)
                // break;
            }
        }
    }

    

    void CPU::write(unsigned int value, uint32_t addr)
    {
        switch (addr & 0xF000)
        {
        case 0xF000:
            // IO Ports
            switch (addr & 0xF00)
            {
            case 0xF00:
                switch (addr & 0xFF)
                {
                case 0x00:
                    // Joypad
                    break;
                case 0x40:
                    video_.setLCDC(value);
                    break;
                };
            default:
                break;
            };
            break;
        };
        mmu_.write(value, addr);
    }

    void CPU::launchSequence_()
    {
        registers_.pc = static_cast<uint16_t>(game_->startAddress());
        registers_.AF.pair = 1;
        registers_.AF.second = 0xB0;
        registers_.BC.pair = 0x13;
        registers_.DE.pair = 0xD8;
        registers_.HL.pair = 0x14D;
        registers_.sp = 0xFFFE;

        mmu_.rawWrite(0, 0xFF05);
        mmu_.rawWrite(0, 0xFF06);
        mmu_.rawWrite(0, 0xFF07);
        mmu_.rawWrite(0x80, 0xFF10);
        mmu_.rawWrite(0xBF, 0xFF11);
        mmu_.rawWrite(0xF3, 0xFF12);
        mmu_.rawWrite(0xBF, 0xFF14);
        mmu_.rawWrite(0x3F, 0xFF16);
        mmu_.rawWrite(0x00, 0xFF17);
        mmu_.rawWrite(0xBF, 0xFF19);
        mmu_.rawWrite(0x7F, 0xFF1A);
        mmu_.rawWrite(0xFF, 0xFF1B);
        mmu_.rawWrite(0x9F, 0xFF1C);
        mmu_.rawWrite(0xBF, 0xFF1E);
        mmu_.rawWrite(0xFF, 0xFF20);
        mmu_.rawWrite(0x00, 0xFF21);
        mmu_.rawWrite(0x00, 0xFF22);
        mmu_.rawWrite(0xBF, 0xFF23);
        mmu_.rawWrite(0x77, 0xFF24);
        mmu_.rawWrite(0xF3, 0xFF25);
        mmu_.rawWrite(0xF1, 0xFF26);
        mmu_.rawWrite(0x91, 0xFF40);
        mmu_.rawWrite(0x00, 0xFF42);
        mmu_.rawWrite(0x00, 0xFF43);
        mmu_.rawWrite(0x00, 0xFF45);
        mmu_.rawWrite(0xFC, 0xFF47);
        mmu_.rawWrite(0xFF, 0xFF48);
        mmu_.rawWrite(0xFF, 0xFF49);
        mmu_.rawWrite(0x00, 0xFF4A);
        mmu_.rawWrite(0x00, 0xFF4B);
        mmu_.rawWrite(0x00, 0xFFFF);
    }

}