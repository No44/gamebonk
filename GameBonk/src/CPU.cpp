#include <iostream>
#include <iomanip>

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


    static unsigned int runOp()
    {
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

    unsigned int CPU::read(uint32_t addr)
    {
        // todo: ports ?
        return mmu_.read(addr);
    }

    unsigned int CPU::readw(uint32_t addr)
    {
        // todo: ports ?
        return mmu_.readw(addr);
    }

    void CPU::writew(unsigned int value, uint32_t addr)
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
        }
        mmu_.writew(value, addr);
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
        registers_.pc = 0x100;
        registers_.AF.pair = 1;
        registers_.AF.A = 0xB0;
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

#define FSET(FLAG) { r.AF.F |= static_cast<unsigned int>(CPU::Flags::FLAG); }
#define FRESET(FLAG) { r.AF.F &= ~static_cast<unsigned int>(CPU::Flags::FLAG); }
#define FASSIGN(FLAG, V) { r.AF.F ^= (- (!!V) ^ r.AF.F) & static_cast<unsigned int>(CPU::Flags::FLAG); }
#define HCARRY(X, Y) ((((X) & 0xF) + ((Y) & 0xF)) & 0x10)
#define CARRY(X, Y) ((((X) & 0xFF) + ((Y)& 0xFF)) & 0x100)
#define FCARRIES(X, Y) { FASSIGN(H, HCARRY(X, Y)); FASSIGN(C, CARRY(X, Y)); }
#define HBORROW(IN, RES) (((IN) & 0xF0) != ((RES) & 0xF0))
#define BORROW(IN, RES) ((RES) >= (IN))
#define FBORROWS(IN, RES) { FASSIGN(H, !HBORROW(IN, RES)); FASSIGN(C, !BORROW(IN, RES)); }

// LD
#define _LD(DST, SRC, LEN, CYCLES) { DST = SRC; return {CYCLES, LEN}; }
// LD to memory location
#define _8BLDMEM(DST, SRC, LEN, CYCLES) { cpu.write(SRC, DST); return {CYCLES, LEN}; }
#define _16BLDMEM(DST, SRC, LEN, CYCLES) { cpu.writew(SRC, DST); return {CYCLES, LEN}; }
#define _PUSH16(VAL, LEN, CYCLES) { r.sp -= 2; cpu.writew(VAL, r.sp); return {CYCLES, LEN}; }
#define _POP16(DST, LEN, CYCLES) { DST = cpu.readw(r.sp); r.sp += 2; return {CYCLES, LEN}; }
#define _ADD8(DST, SRC, LEN, CYCLES) { FCARRIES(DST, SRC); DST += SRC; FASSIGN(Z, !DST); FRESET(N); }
#define _SUB8(DST, SRC, LEN, CYCLES) { int res = DST - SRC; FBORROWS(DST, res); DST = res; FASSIGN(Z, !res); FSET(N); }
#define _AND(DST, SRC, LEN, CYCLES) {DST &= SRC; FASSIGN(Z, !DST); FRESET(N); FSET(H); FRESET(C); }
#define _OR(DST, SRC, LEN, CYCLES) {DST |= SRC; FASSIGN(Z, !DST); FRESET(N); FRESET(H); FRESET(C); }
#define _XOR(DST, SRC, LEN, CYCLES) {DST ^= SRC; FASSIGN(Z, !DST); FRESET(N); FRESET(H); FRESET(C); }
#define _CP();

    static CPU::OpFormat runCurrentOp(CPU& cpu)
    {
      CPU::Registers& r = cpu.registers_;
      MMU& mmu = cpu.mmu_;

      unsigned int op = mmu.read(cpu.registers_.pc);
      //case 0x:
      switch (op)
      {
      // 8-bit loads
      case 0x06: _LD(r.BC.B, cpu.read(r.pc + 1), 2, 8);
      case 0x0E: _LD(r.BC.C, cpu.read(r.pc + 1), 2, 8);
      case 0x16: _LD(r.DE.D, cpu.read(r.pc + 1), 2, 8);
      case 0x1E: _LD(r.DE.E, cpu.read(r.pc + 1), 2, 8);
      case 0x26: _LD(r.HL.H, cpu.read(r.pc + 1), 2, 8);
      case 0x2E: _LD(r.HL.L, cpu.read(r.pc + 1), 2, 8);
      case 0x7F: _LD(r.AF.A, r.AF.A, 1, 4);
      case 0x78: _LD(r.AF.A, r.BC.B, 1, 4);
      case 0x79: _LD(r.AF.A, r.BC.C, 1, 4);
      case 0x7A: _LD(r.AF.A, r.DE.D, 1, 4);
      case 0x7B: _LD(r.AF.A, r.DE.E, 1, 4);
      case 0x7C: _LD(r.AF.A, r.HL.H, 1, 4);
      case 0x7D: _LD(r.AF.A, r.HL.L, 1, 4);
      case 0x7E: _LD(r.AF.A, cpu.read(r.HL.pair), 1, 8);
      case 0x40: _LD(r.BC.B, r.BC.B, 1, 4);
      case 0x41: _LD(r.BC.B, r.BC.C, 1, 4);
      case 0x42: _LD(r.BC.B, r.DE.D, 1, 4);
      case 0x43: _LD(r.BC.B, r.DE.E, 1, 4);
      case 0x44: _LD(r.BC.B, r.HL.H, 1, 4);
      case 0x45: _LD(r.BC.B, r.HL.L, 1, 4);
      case 0x46: _LD(r.BC.B, cpu.read(r.HL.pair), 1, 8);
      case 0x48: _LD(r.BC.C, r.BC.B, 1, 4);
      case 0x49: _LD(r.BC.C, r.BC.C, 1, 4);
      case 0x4A: _LD(r.BC.C, r.DE.D, 1, 4);
      case 0x4B: _LD(r.BC.C, r.DE.E, 1, 4);
      case 0x4C: _LD(r.BC.C, r.HL.H, 1, 4);
      case 0x4D: _LD(r.BC.C, r.HL.L, 1, 4);
      case 0x4E: _LD(r.BC.C, cpu.read(r.HL.pair), 1, 8);
      case 0x50: _LD(r.DE.D, r.BC.B, 1, 4);
      case 0x51: _LD(r.DE.D, r.BC.C, 1, 4);
      case 0x52: _LD(r.DE.D, r.DE.D, 1, 4);
      case 0x53: _LD(r.DE.D, r.DE.E, 1, 4);
      case 0x54: _LD(r.DE.D, r.HL.H, 1, 4);
      case 0x55: _LD(r.DE.D, r.HL.L, 1, 4);
      case 0x56: _LD(r.DE.D, cpu.read(r.HL.pair), 1, 8);
      case 0x58: _LD(r.DE.E, r.BC.B, 1, 4);
      case 0x59: _LD(r.DE.E, r.BC.C, 1, 4);
      case 0x5A: _LD(r.DE.E, r.DE.D, 1, 4);
      case 0x5B: _LD(r.DE.E, r.DE.E, 1, 4);
      case 0x5C: _LD(r.DE.E, r.HL.H, 1, 4);
      case 0x5D: _LD(r.DE.E, r.HL.L, 1, 4);
      case 0x5E: _LD(r.DE.E, cpu.read(r.HL.pair), 1, 8);
      case 0x60: _LD(r.HL.H, r.BC.B, 1, 4);
      case 0x61: _LD(r.HL.H, r.BC.C, 1, 4);
      case 0x62: _LD(r.HL.H, r.DE.D, 1, 4);
      case 0x63: _LD(r.HL.H, r.DE.E, 1, 4);
      case 0x64: _LD(r.HL.H, r.HL.H, 1, 4);
      case 0x65: _LD(r.HL.H, r.HL.L, 1, 4);
      case 0x66: _LD(r.HL.H, cpu.read(r.HL.pair), 1, 8);
      case 0x68: _LD(r.HL.L, r.BC.B, 1, 4);
      case 0x69: _LD(r.HL.L, r.BC.C, 1, 4);
      case 0x6A: _LD(r.HL.L, r.DE.D, 1, 4);
      case 0x6B: _LD(r.HL.L, r.DE.E, 1, 4);
      case 0x6C: _LD(r.HL.L, r.HL.H, 1, 4);
      case 0x6D: _LD(r.HL.L, r.HL.L, 1, 4);
      case 0x6E: _LD(r.HL.L, cpu.read(r.HL.pair), 1, 8);
      case 0x70: _8BLDMEM((r.HL.pair), r.BC.B, 1, 8);
      case 0x71: _8BLDMEM((r.HL.pair), r.BC.C, 1, 8);
      case 0x72: _8BLDMEM((r.HL.pair), r.DE.D, 1, 8);
      case 0x73: _8BLDMEM((r.HL.pair), r.DE.E, 1, 8);
      case 0x74: _8BLDMEM((r.HL.pair), r.HL.H, 1, 8);
      case 0x75: _8BLDMEM((r.HL.pair), r.HL.L, 1, 8);
      case 0x36: _8BLDMEM((r.HL.pair), cpu.read(r.pc + 1), 2, 12);
      case 0x0A: _LD(r.AF.A, cpu.read(r.BC.pair), 1, 8);
      case 0x1A: _LD(r.AF.A, cpu.read(r.DE.pair), 1, 8);
      case 0xFA: _LD(r.AF.A, cpu.read(cpu.readw(r.pc + 1)), 3, 16); // !! conflict ?
      case 0x3E: _LD(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x47: _LD(r.BC.B, r.AF.A, 1, 4);
      case 0x4F: _LD(r.BC.C, r.AF.A, 1, 4);
      case 0x57: _LD(r.DE.D, r.AF.A, 1, 4);
      case 0x5F: _LD(r.DE.E, r.AF.A, 1, 4);
      case 0x67: _LD(r.HL.H, r.AF.A, 1, 4);
      case 0x6F: _LD(r.HL.L, r.AF.A, 1, 4);
      case 0x02: _8BLDMEM((r.BC.pair), r.AF.A, 1, 8);
      case 0x12: _8BLDMEM((r.DE.pair), r.AF.A, 1, 8);
      case 0x77: _8BLDMEM((r.HL.pair), r.AF.A, 1, 8);
      case 0xEA: _8BLDMEM((cpu.readw(r.pc+1)), r.AF.A, 3, 16);
      case 0xF2: _LD(r.AF.A, cpu.read(0xFF00 + r.BC.C), 1, 8);
      case 0xE2: _8BLDMEM((0xFF00 + r.BC.C), r.AF.A, 1, 8);
      case 0x3A: _LD(r.AF.A, cpu.read(r.HL.pair--), 1, 8);
      case 0x32: _8BLDMEM(r.HL.pair--, r.AF.A, 1, 8);
      case 0x2A: _LD(r.AF.A, cpu.read(r.HL.pair++), 1, 8);
      case 0x22: _8BLDMEM(r.HL.pair++, r.AF.A, 1, 8);
      case 0xE0: _8BLDMEM((0xFF00 + cpu.read(r.pc + 1)), r.AF.A, 2, 12);
      case 0xF0: _LD(r.AF.A, cpu.read(0xFF00 + cpu.read(r.pc + 1)), 2, 12);
      case 0x01: _LD(r.BC.pair, cpu.readw(r.pc + 1), 3, 12);
      case 0x11: _LD(r.DE.pair, cpu.readw(r.pc + 1), 3, 12);
      case 0x21: _LD(r.HL.pair, cpu.readw(r.pc + 1), 3, 12);
      case 0x31: _LD(r.sp, cpu.readw(r.pc + 1), 3, 12);
      case 0xF9: _LD(r.sp, r.HL.pair, 1, 8);
      case 0xF8: {
        FRESET(Z);
        FRESET(N);
        unsigned int op = r.sp;
        int rh = (int)cpu.read(r.pc + 1);
        FASSIGN(H, HCARRY(op, rh));
        FASSIGN(C, CARRY(op, rh));
        _LD(r.HL.pair, op + rh, 2, 12);
      }
      case 0x08: _8BLDMEM((cpu.readw(r.pc + 1)), r.sp, 3, 20);
      // conflit sur les 4 en dessous: verifier
      case 0xF5: _PUSH16(r.AF.pair, 1, 16);
      case 0xC5: _PUSH16(r.BC.pair, 1, 16);
      case 0xD5: _PUSH16(r.DE.pair, 1, 16);
      case 0xE5: _PUSH16(r.HL.pair, 1, 16);
      case 0xF1: _POP16(r.AF.pair, 1, 12);
      case 0xC1: _POP16(r.BC.pair, 1, 12);
      case 0xD1: _POP16(r.DE.pair, 1, 12);
      case 0xE1: _POP16(r.HL.pair, 1, 12);
      case 0x87: _ADD8(r.AF.A, r.AF.A, 1, 4);
      case 0x80: _ADD8(r.AF.A, r.BC.B, 1, 4);
      case 0x81: _ADD8(r.AF.A, r.BC.C, 1, 4);
      case 0x82: _ADD8(r.AF.A, r.DE.D, 1, 4);
      case 0x83: _ADD8(r.AF.A, r.DE.E, 1, 4);
      case 0x84: _ADD8(r.AF.A, r.HL.H, 1, 4);
      case 0x85: _ADD8(r.AF.A, r.HL.L, 1, 4);
      case 0x86: _ADD8(r.AF.A, cpu.read(r.HL.pair), 1, 8);
      case 0xC6: _ADD8(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x8F: _ADD8(r.AF.A, r.AF.A + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x88: _ADD8(r.AF.A, r.BC.B + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x89: _ADD8(r.AF.A, r.BC.C + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8A: _ADD8(r.AF.A, r.DE.D + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8B: _ADD8(r.AF.A, r.DE.E + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8C: _ADD8(r.AF.A, r.HL.H + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8D: _ADD8(r.AF.A, r.HL.L + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8E: _ADD8(r.AF.A, cpu.read(r.HL.pair) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 8);
      case 0xCE: _ADD8(r.AF.A, cpu.read(r.pc + 1) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 2, 8);
      case 0x97: _SUB8(r.AF.A, r.AF.A, 1, 4);
      case 0x90: _SUB8(r.AF.A, r.BC.B, 1, 4);
      case 0x91: _SUB8(r.AF.A, r.BC.C, 1, 4);
      case 0x92: _SUB8(r.AF.A, r.DE.D, 1, 4);
      case 0x93: _SUB8(r.AF.A, r.DE.E, 1, 4);
      case 0x94: _SUB8(r.AF.A, r.HL.H, 1, 4);
      case 0x95: _SUB8(r.AF.A, r.HL.L, 1, 4);
      case 0x96: _SUB8(r.AF.A, cpu.read(r.HL.pair), 1, 8);
      case 0xD6: _SUB8(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x9F: _SUB8(r.AF.A, r.AF.A + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x98: _SUB8(r.AF.A, r.BC.B + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x99: _SUB8(r.AF.A, r.BC.C + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9A: _SUB8(r.AF.A, r.DE.D + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9B: _SUB8(r.AF.A, r.DE.E + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9C: _SUB8(r.AF.A, r.HL.H + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9D: _SUB8(r.AF.A, r.HL.L + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9E: _SUB8(r.AF.A, cpu.read(r.HL.pair) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 8);
      case 0xA7: _AND(r.AF.A, r.AF.A, 1, 4);
      case 0xA0: _AND(r.AF.A, r.BC.B, 1, 4);
      case 0xA1: _AND(r.AF.A, r.BC.C, 1, 4);
      case 0xA2: _AND(r.AF.A, r.DE.D, 1, 4);
      case 0xA3: _AND(r.AF.A, r.DE.E, 1, 4);
      case 0xA4: _AND(r.AF.A, r.HL.H, 1, 4);
      case 0xA5: _AND(r.AF.A, r.HL.L, 1, 4);
      case 0xA6: _AND(r.AF.A, cpu.read(r.HL.pair), 1, 8);
      case 0xE6: _AND(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0xB7: _OR(r.AF.A, r.AF.A, 1, 4);
      case 0xB0: _OR(r.AF.A, r.BC.B, 1, 4);
      case 0xB1: _OR(r.AF.A, r.BC.C, 1, 4);
      case 0xB2: _OR(r.AF.A, r.DE.D, 1, 4);
      case 0xB3: _OR(r.AF.A, r.DE.E, 1, 4);
      case 0xB4: _OR(r.AF.A, r.HL.H, 1, 4);
      case 0xB5: _OR(r.AF.A, r.HL.L, 1, 4);
      case 0xB6: _OR(r.AF.A, cpu.read(r.HL.pair), 1, 8);
      case 0xF6: _OR(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0xAF: _XOR(r.AF.A, r.AF.A, 1, 4);
      case 0xA8: _XOR(r.AF.A, r.BC.B, 1, 4);
      case 0xA9: _XOR(r.AF.A, r.BC.C, 1, 4);
      case 0xAA: _XOR(r.AF.A, r.DE.D, 1, 4);
      case 0xAB: _XOR(r.AF.A, r.DE.E, 1, 4);
      case 0xAC: _XOR(r.AF.A, r.HL.H, 1, 4);
      case 0xAD: _XOR(r.AF.A, r.HL.L, 1, 4);
      case 0xAE: _XOR(r.AF.A, cpu.read(r.HL.pair), 1, 8);
      case 0xEE: _XOR(r.AF.A, cpu.read(r.pc + 1), 2, 8);

      default:
        std::cerr << "Undefined opcode " << std::hex << op << ", treating as NOP" << std::endl;
        return {4,1};
      }
    }

}
