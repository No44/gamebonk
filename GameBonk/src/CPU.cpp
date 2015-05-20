#if defined _WIN32 || _WIN64
#include <windows.h>
#endif


#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>

#include "video/Driver.hpp"
#include "debug/Instruction.hpp"
#include "MMU.hpp"
#include "CPU.hpp"
#include "Cartridge.hpp"

#ifndef _WIN32

void OutputDebugString(const std::string& str)
{
  std::cerr << str << std::endl;
}

#endif

namespace GBonk
{

    CPU::CPU()
        : interruptMasterEnable_(true),
        interruptsEnabled_(0),
        video_(mmu_.memory())
    {
    }

    void CPU::load(Cartridge& game)
    {
        game_ = &game;
    }

    void CPU::run()
    {
        // TODO: TEMP
        Video::Driver d;

        {
            std::stringstream builder;
            builder << "System memory base: 0x" << std::hex << (uint64_t)this->mmu_.memory() << std::endl;
            builder << "Cartridge ROM: 0x" << std::hex << (uint64_t)game_->ROM() << std::endl;
            OutputDebugString(builder.str().c_str());
        }

        d.openWindow();

        launchSequence_();
        mmu_.setMBC(GBonk::AMBC::makeMBC(*game_));

        unsigned long long counter = 0;
        uint16_t& PC = registers_.pc;
        bool render = false;
        while (d.pumpEvents() == false)
        {
            //std::cout << "Running instruction " << std::hex << read(registers_.pc) << " at " << std::hex << registers_.pc << std::endl;
            {
                Debug::Instruction instr(game_->ROM() + registers_.pc, registers_.pc);
                std::cout << "[0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << registers_.pc << "]";
                std::cout << instr.toString() << std::endl;
            }
            CPU::OpFormat f = runCurrentOp(*this);

            registers_.pc += f.bytes;
            counter += f.cycles;
            
            video_.cheatDrawAll();
            if (render)
                d.render();
            // Read and run operation at PC
            // decrement COUNTER from the amount of
            // cycles the operation last

            /*
            if (counter <= 0)
            {
                // run interrupts
                
                if runInterrupt:
                    if_ = true;
                    interruptMasterEnable_ = false;
                    r.sp -= 2;
                    writew(r.pc, r.sp);
                    r.pc = interrupt_start_address;
                
                
                counter += CPU::InterruptPeriod;
                // if (exitRequested)
                // break;
            }
            */
        }
    }

    void CPU::interrupt(InterruptId i)
    {
        registers_.sp -= 2;
        writew(registers_.pc, registers_.sp);
        registers_.pc = static_cast<uint32_t>(i);
    }

    unsigned int CPU::read(uint32_t addr)
    {
        return mmu_.read(addr);
    }

    unsigned int CPU::readw(uint32_t addr)
    {
        return mmu_.readw(addr);
    }

    void CPU::ioregWrite(unsigned int& value, uint32_t addr)
    {
        switch (addr & 0xFF)
        {
        case 0x00:
            // Joypad
            break;
        case 0x04:
            // DIV
            value = 0;
            break;
        case 0x05:
            // TIMA
            break;
        case 0x06:
            // TMA
            break;
        case 0x40:
            video_.updateLCDC();
            break;
        };
    }

    void CPU::writew(unsigned int value, uint32_t addr)
    {
        switch (addr & 0xFF00)
        {
        case 0xFF00:
            ioregWrite(value, addr & 0xFF);
            ioregWrite(value, (addr & 0xFF) + 1);
            break;
        default:
            break;
        }
        mmu_.writew(value, addr);
    }

    void CPU::write(unsigned int value, uint32_t addr)
    {
        switch (addr & 0xFF00)
        {
        case 0xFF00:
            ioregWrite(value, addr & 0xFF);
            break;
        default:
            break;
        }
        mmu_.write(value, addr);
    }

    void CPU::launchSequence_()
    {
        registers_.pc = 0x100;
        registers_.AF = 1;
        registers_.AF.F = 0xB0;
        registers_.BC = 0x13;
        registers_.DE = 0xD8;
        registers_.HL = 0x14D;
        registers_.sp = 0xFFFE;

        // can probably turn that to regular write and let CPU write handle REG writes
        write(0, 0xFF05); // TIMA
        write(0, 0xFF06); // TMA
        write(0, 0xFF07);
        write(0x80, 0xFF10);
        write(0xBF, 0xFF11);
        write(0xF3, 0xFF12);
        write(0xBF, 0xFF14);
        write(0x3F, 0xFF16);
        write(0x00, 0xFF17);
        write(0xBF, 0xFF19);
        write(0x7F, 0xFF1A);
        write(0xFF, 0xFF1B);
        write(0x9F, 0xFF1C);
        write(0xBF, 0xFF1E);
        write(0xFF, 0xFF20);
        write(0x00, 0xFF21);
        write(0x00, 0xFF22);
        write(0xBF, 0xFF23);
        write(0x77, 0xFF24);
        write(0xF3, 0xFF25);
        write(0xF1, 0xFF26);
        write(0x91, 0xFF40);
        video_.updateLCDC();
        write(0x00, 0xFF42);
        write(0x00, 0xFF43);
        write(0x00, 0xFF45);
        write(0xFC, 0xFF47);
        write(0xFF, 0xFF48);
        write(0xFF, 0xFF49);
        write(0x00, 0xFF4A);
        write(0x00, 0xFF4B);
        write(0x00, 0xFFFF);
    }

#define FSET(FLAG) { r.AF.F |= static_cast<unsigned int>(CPU::Flags::FLAG); }
#define FRESET(FLAG) { r.AF.F &= ~static_cast<unsigned int>(CPU::Flags::FLAG); }
#define FTOGGLE(FLAG) { r.AF.F ^= static_cast<unsigned int>(CPU::Flags::FLAG); }
#define FASSIGN(FLAG, V) { r.AF.F ^= (- !!(V) ^ r.AF.F) & static_cast<unsigned int>(CPU::Flags::FLAG); }
#define FISSET(FLAG) (r.AF.F & static_cast<unsigned int>(CPU::Flags::FLAG))
#define _8BHCARRY(X, Y) ((((X) & 0xF) + ((Y) & 0xF)) & 0x10)
#define _8BCARRY(X, Y) ((((X) & 0xFF) + ((Y)& 0xFF)) & 0x100)
#define F8BCARRIES(X, Y) { FASSIGN(H, _8BHCARRY(X, Y)); FASSIGN(C, _8BCARRY(X, Y)); }
#define _16BHCARRY(X, Y) _8BHCARRY((X) >> 8, (Y) >> 8)
#define _16BCARRY(X, Y) _8BCARRY((X) >> 8, (Y) >> 8)
#define F16BCARRIES(X, Y) { FASSIGN(H, _16BHCARRY(X, Y)); FASSIGN(C, _16BCARRY(X, Y));}
#define HBORROW(IN, RES) (((IN) & 0xF0) != ((RES) & 0xF0))
#define BORROW(IN, RES) ((RES) >= (IN))
#define FBORROWS(IN, RES) { FASSIGN(H, !HBORROW(IN, RES)); FASSIGN(C, !BORROW(IN, RES)); }

#define _LD(DST, SRC, LEN, CYCLES) { DST = SRC; return {CYCLES, LEN}; }
#define _8BLDMEM(DST, SRC, LEN, CYCLES) { cpu.write(SRC, DST); return {CYCLES, LEN}; }
#define _16BLDMEM(DST, SRC, LEN, CYCLES) { cpu.writew(SRC, DST); return {CYCLES, LEN}; }
#define _PUSH16(VAL, LEN, CYCLES) { r.sp -= 2; cpu.writew(VAL, r.sp); return {CYCLES, LEN}; }
#define _POP16(DST, LEN, CYCLES) { DST = cpu.readw(r.sp); r.sp += 2; return {CYCLES, LEN}; }
#define _ADD8(DST, SRC, LEN, CYCLES) { F8BCARRIES(DST, SRC); DST += SRC; FASSIGN(Z, !(DST)); FRESET(N); return {CYCLES, LEN}; }
#define _ADD16(DST, SRC, LEN, CYCLES)  { F16BCARRIES(DST, SRC); DST = DST + SRC; FRESET(N); return {CYCLES, LEN}; }
#define _SUB8(DST, SRC, LEN, CYCLES) { int res = (DST) - (SRC); FBORROWS(DST, res); DST = res; FASSIGN(Z, !res); FSET(N); return {CYCLES, LEN}; }
#define _AND(DST, SRC, LEN, CYCLES) {DST &= SRC; FASSIGN(Z, !DST); FRESET(N); FSET(H); FRESET(C); return {CYCLES, LEN}; }
#define _OR(DST, SRC, LEN, CYCLES) {DST |= SRC; FASSIGN(Z, !DST); FRESET(N); FRESET(H); FRESET(C); return {CYCLES, LEN}; }
#define _XOR(DST, SRC, LEN, CYCLES) {DST ^= SRC; FASSIGN(Z, !DST); FRESET(N); FRESET(H); FRESET(C); return {CYCLES, LEN}; }
#define _CP(LH, RH, LEN, CYCLES) { int res = (LH) - (RH); FASSIGN(Z, !res); FSET(N); FASSIGN(H, !HBORROW(LH, res)); FASSIGN(C, LH < RH); return {CYCLES, LEN}; }
#define _INC(DST, LEN, CYCLES) { uint8_t ini = DST; DST++; FASSIGN(Z, !DST); FRESET(N); FASSIGN(H, _8BHCARRY(ini, 1)); return {CYCLES, LEN}; }
#define _INCMEM(DST, LEN, CYCLES) { uint8_t ini = cpu.read(DST); cpu.write(ini + 1, DST); FASSIGN(Z, !(ini + 1)); FRESET(N); FASSIGN(H, _8BHCARRY(ini, 1)); return {CYCLES, LEN};}
#define _DEC(DST, LEN, CYCLES) _SUB8(DST, 1, LEN, CYCLES)
#define _DECMEM(DST, LEN, CYCLES) { int ini = cpu.read(DST); cpu.write(ini - 1, DST); FASSIGN(Z, !(ini - 1)); FSET(N); FASSIGN(H, !BORROW(ini, ini - 1)); return {CYCLES, LEN}; }
#define _DECREG(DST, LEN, CYCLES) {DST--; return {CYCLES, LEN}; }
#define _INCREG(DST, LEN, CYCLES) {DST++; return {CYCLES, LEN}; }
#define _SWAP(DST, LEN, CYCLES) { DST = (((DST) & 0x0F) << 8) | (((DST) & 0xF0) >> 8); FASSIGN(Z, !(DST)); FRESET(N); FRESET(C); FRESET(H); return {CYCLES, LEN}; }
#define _SWAPMEM(DST, LEN, CYCLES) { unsigned int val = cpu.read(DST); val = ((val & 0x0F) << 8) | ((val & 0xF0) >> 8); cpu.write(val, DST); FASSIGN(Z, !val); FRESET(N); FRESET(H); FRESET(C); return {CYCLES, LEN}; }
#define _CPL(DST, LEN, CYCLES) { DST = ~DST; FSET(N); FSET(H); return {CYCLES, LEN}; }
#define _CCF() { FTOGGLE(C); FRESET(N); FRESET(H); return {4, 1}; }
#define _SCF() { FSET(C); FRESET(N); FRESET(H); return {4, 1}; }
#define _NOP() { return {4, 1}; }
#define _HALT() { cpu.halt(); return {4, 1}; }
#define _STOP() { cpu.stop(); return {4, 2}; }
#define _DI_() { cpu.prepareDisableInterrupts(); return{4, 1}; }
#define _EI() { cpu.prepareEnableInterrupts(); return {4, 1}; }
#define _RLC(DST, LEN, CYCLES) { uint32_t lb = (unsigned int)(DST) >> 7; DST <<= 1; DST |= lb; FASSIGN(C, lb); FASSIGN(Z, !DST); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _RLCMEM(DST, LEN, CYCLES) { uint32_t v = cpu.read(DST); uint32_t lb = v >> 7; v <<= 1; v |= lb; cpu.write(v, DST); FASSIGN(C, lb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {CYCLES, LEN};}
#define _RL(DST, LEN, CYCLES) { uint32_t lb = (unsigned int)(DST) >> 7; DST <<= 1; DST |= !!(r.AF.F & (unsigned int)CPU::Flags::C); FASSIGN(C, lb); FASSIGN(Z, !DST); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _RLMEM(DST, LEN, CYCLES) {uint32_t v = cpu.read(DST); uint32_t lb = v >> 7; v <<= 1; v |= !!(r.AF.F & (unsigned int)CPU::Flags::C); cpu.write(v, DST); FASSIGN(C, lb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {CYCLES, LEN};}
#define _RRC(DST, LEN, CYCLES) {uint32_t fb = (DST) & 1; DST >>= 1; DST |= fb << 7; FASSIGN(C, fb); FASSIGN(Z, !DST); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _RRCMEM(DST, LEN, CYCLES) { uint32_t v = cpu.read(DST); uint32_t fb = (v) & 1; v >>= 1; v |= fb << 7; cpu.write(v, DST); FASSIGN(C, fb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _RR(DST, LEN, CYCLES) { uint32_t fb = (DST) & 1; DST >>= 1; DST |= (!!(r.AF.F & (unsigned int)CPU::Flags::C)) << 7; FASSIGN(C, fb); FASSIGN(Z, !DST); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _RRMEM(DST, LEN, CYCLES) { uint32_t v = cpu.read(DST); uint32_t fb = (v) & 1; v >>= 1; v |= (!!(r.AF.F & (unsigned int)CPU::Flags::C)) << 7; cpu.write(v, DST); FASSIGN(C, fb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {CYCLES, LEN};}
#define _SL(DST, LEN, CYCLES) { uint32_t lb = (unsigned int)(DST) >> 7; DST <<= 1; DST &= ~1; FASSIGN(C, lb); FASSIGN(Z, !DST); FRESET(N); FRESET(H); return {4, 1}; }
#define _SLMEM(DST, LEN, CYCLES) { uint32_t v = cpu.read(DST); uint32_t lb = (unsigned int)(v) >> 7; v <<= 1; v &= ~1; cpu.write(v, DST); FASSIGN(C, lb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {4, 1}; }
#define _SRA(DST, LEN, CYCLES) {DST = (int8_t)(DST) >> 1; uint32_t fb = (DST) & 1; FASSIGN(C, fb); FASSIGN(Z, !(DST)); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _SRAMEM(DST, LEN, CYCLES) {uint8_t v = cpu.read(DST); v = (int8_t)(v) >> 1; cpu.write(v, DST); uint32_t fb = (v) & 1; FASSIGN(C, fb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _SRL(DST, LEN, CYCLES) { DST = (unsigned int)DST >> 1; uint32_t fb = (DST) & 1; FASSIGN(C, fb); FASSIGN(Z, !(DST)); FRESET(N); FRESET(H); return {CYCLES, LEN}; }
#define _SRLMEM(DST, LEN, CYCLES) { uint32_t v = cpu.read(DST); v >>= 1; cpu.write(v, DST); uint32_t fb = v & 1; FASSIGN(C, fb); FASSIGN(Z, !v); FRESET(N); FRESET(H); return {CYCLES, LEN};}
#define _BIT(DST, B, LEN, CYCLES) { FASSIGN(Z, !((DST) & (1 << B))); FRESET(N); FSET(H); return {CYCLES, LEN}; }
#define _BITMEM(DST, B, LEN, CYCLES) { uint32_t v = cpu.read(DST); FASSIGN(Z, !(v & (1 << B))); FRESET(N); FSET(H); return { CYCLES, LEN }; }
#define _SET(DST, B, LEN, CYCLES) { DST |= 1 << B; return {CYCLES, LEN}; }
#define _SETMEM(DST, B, LEN, CYCLES) { uint32_t v = cpu.read(DST); v |= 1 << B; cpu.write(v, DST); return {CYCLES, LEN}; }
#define _RES(DST, B, LEN, CYCLES) { DST &= ~(1 << B); return {CYCLES, LEN}; }
#define _RESMEM(DST, B, LEN, CYCLES) { uint32_t v = cpu.read(DST); v &= ~(1 << B); cpu.write(v, DST); return {CYCLES, LEN};}
#define _JP() { uint32_t addr = cpu.readw(r.pc + 1); r.pc = addr; return {12, 0}; }
#define _JPNZ() { if (FISSET(Z)) return {12, 3}; _JP() }
#define _JPZ() { if (!FISSET(Z)) return {12, 3}; _JP() }
#define _JPNC() { if (FISSET(C)) return {12, 3}; _JP() }
#define _JPC() { if (!FISSET(C)) return {12, 3}; _JP() }
#define _JPHL() { r.pc = r.HL; return {4, 0}; }
#define _JR() { int val = (int8_t)cpu.read(r.pc + 1); r.pc += val; return {8, 2}; } // todo : vraiment garder le +2 ?
#define _JRNZ() { if (FISSET(Z)) return {8, 2}; _JR() }
#define _JRZ() { if (!FISSET(Z)) return {8, 2}; _JR() }
#define _JRNC() { if (FISSET(C)) return {8, 2}; _JR() }
#define _JRC() { if (!FISSET(C)) return {8, 2}; _JR() }
#define _CALL() { uint32_t nextIns = r.pc + 3; r.sp -= 2; cpu.writew(nextIns, r.sp); _JP() }
#define _CALLNZ() { if (FISSET(Z)) return {12, 3}; _CALL() }
#define _CALLZ() { if (!FISSET(Z)) return {12, 3}; _CALL() }
#define _CALLNC() { if (FISSET(C)) return {12, 3}; _CALL() }
#define _CALLC() { if (!FISSET(C)) return {12, 3}; _CALL() }
#define _RST(ADDR) { r.sp -= 2; cpu.writew(r.pc + 1, r.sp); r.pc = ADDR; return {32, 0}; }
#define _RET() { r.pc = cpu.readw(r.sp); r.sp += 2; return {8, 0};}
#define _RETNZ() { if (FISSET(Z)) return {8, 1}; _RET() }
#define _RETZ() { if (!FISSET(Z)) return {8, 1}; _RET() }
#define _RETNC() { if (FISSET(C)) return {8, 1}; _RET() }
#define _RETC() { if (!FISSET(C)) return {8, 1}; _RET() }
#define _RETI() { cpu.prepareEnableInterrupts(); _RET() }

    static inline CPU::OpFormat DAA(CPU& cpu, CPU::Registers& r)
    {
        uint32_t upper = (r.AF.A & 0xF0) >> 8;
        uint32_t lower = (r.AF.A & 0xF);
        bool C = !!(r.AF.F & (unsigned int)CPU::Flags::C);
        bool H = !!(r.AF.F & (unsigned int)CPU::Flags::H);
        bool N = !!(r.AF.F & (unsigned int)CPU::Flags::N);

        if (!N)
        {
            if (!C)
            {
                if (upper <= 0x9)
                {
                    if (!H)
                    {
                        if (lower <= 0x9)
                        {
                            FRESET(C);
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                    else /* if H */
                    {
                        if (lower <= 0x3)
                        {
                            FRESET(C);
                            r.AF.A += 0x06;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                }
                if (upper <= 0x8)
                {
                    if (!H)
                    {
                        if (lower >= 0xA)
                        {
                            FRESET(C);
                            r.AF.A += 0x06;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                }
                if (upper >= 0xA)
                {
                    if (!H)
                    {
                        if (lower <= 0x9)
                        {
                            FSET(C);
                            r.AF.A += 0x60;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                    else
                    {
                        if (lower <= 0x3)
                        {
                            FSET(C);
                            r.AF.A += 0x66;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                }
                if (upper >= 0x9)
                {
                    if (!H)
                    {
                        if (lower >= 0xA)
                        {
                            FSET(C);
                            r.AF.A += 0x66;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                }
            }
            else /* if C */
            {
                if (upper <= 0x2)
                {
                    if (!H)
                    {
                        if (lower <= 0x9)
                        {
                            FSET(C);
                            r.AF.A += 0x60;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                        else /* if (lower >= 0xA) */
                        {
                            FSET(C);
                            r.AF.A += 0x66;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                }
                else if (upper <= 0x3)
                {
                    if (H)
                    {
                        if (lower <= 0x3)
                        {
                            FSET(C);
                            r.AF.A += 0x66;
                            FASSIGN(Z, !r.AF.A);
                            return{ 4, 1 };
                        }
                    }
                }
            }
        }
        FASSIGN(Z, !r.AF.A);
        return{ 4, 1 };
    }

    static inline CPU::OpFormat bitOp(CPU& cpu, CPU::Registers& r)
    {
      //CPU::Registers& r = cpu.registers_;
      unsigned int op = cpu.read(r.pc + 1);

      switch (op)
      {
      case 0x37: _SWAP(r.AF.A, 2, 8);
      case 0x30: _SWAP(r.BC.B, 2, 8);
      case 0x31: _SWAP(r.BC.C, 2, 8);
      case 0x32: _SWAP(r.DE.D, 2, 8);
      case 0x33: _SWAP(r.DE.E, 2, 8);
      case 0x34: _SWAP(r.HL.H, 2, 8);
      case 0x35: _SWAP(r.HL.L, 2, 8);
      case 0x36: _SWAPMEM((r.HL), 2, 16);
      case 0x07: _RLC(r.AF.A, 2, 8);
      case 0x00: _RLC(r.BC.B, 2, 8);
      case 0x01: _RLC(r.BC.C, 2, 8);
      case 0x02: _RLC(r.DE.D, 2, 8);
      case 0x03: _RLC(r.DE.E, 2, 8);
      case 0x04: _RLC(r.HL.H, 2, 8);
      case 0x05: _RLC(r.HL.L, 2, 8);
      case 0x06: _RLCMEM((r.HL), 2, 16);
      case 0x17: _RL(r.AF.A, 2, 8);
      case 0x10: _RL(r.BC.B, 2, 8);
      case 0x11: _RL(r.BC.C, 2, 8);
      case 0x12: _RL(r.DE.D, 2, 8);
      case 0x13: _RL(r.DE.E, 2, 8);
      case 0x14: _RL(r.HL.H, 2, 8);
      case 0x15: _RL(r.HL.L, 2, 8);
      case 0x16: _RLMEM((r.HL), 2, 16);
      case 0x0F: _RRC(r.AF.A, 2, 8);
      case 0x08: _RRC(r.BC.B, 2, 8);
      case 0x09: _RRC(r.BC.C, 2, 8);
      case 0x0A: _RRC(r.DE.D, 2, 8);
      case 0x0B: _RRC(r.DE.E, 2, 8);
      case 0x0C: _RRC(r.HL.H, 2, 8);
      case 0x0D: _RRC(r.HL.L, 2, 8);
      case 0x0E: _RRCMEM((r.HL), 2, 16);
      case 0x1F: _RR(r.AF.A, 2, 8);
      case 0x18: _RR(r.BC.B, 2, 8);
      case 0x19: _RR(r.BC.C, 2, 8);
      case 0x1A: _RR(r.DE.D, 2, 8);
      case 0x1B: _RR(r.DE.E, 2, 8);
      case 0x1C: _RR(r.HL.H, 2, 8);
      case 0x1D: _RR(r.HL.L, 2, 8);
      case 0x1E: _RRMEM((r.HL),2,16);
      case 0x27: _SL(r.AF.A, 2, 8);
      case 0x20: _SL(r.BC.B, 2, 8);
      case 0x21: _SL(r.BC.C, 2, 8);
      case 0x22: _SL(r.DE.D, 2, 8);
      case 0x23: _SL(r.DE.E, 2, 8);
      case 0x24: _SL(r.HL.H, 2, 8);
      case 0x25: _SL(r.HL.L, 2, 8);
      case 0x26: _SLMEM((r.HL), 2, 16);
      case 0x2F: _SRA(r.AF.A, 2, 8);
      case 0x28: _SRA(r.BC.B, 2, 8);
      case 0x29: _SRA(r.BC.C, 2, 8);
      case 0x2A: _SRA(r.DE.D, 2, 8);
      case 0x2B: _SRA(r.DE.E, 2, 8);
      case 0x2C: _SRA(r.HL.H, 2, 8);
      case 0x2D: _SRA(r.HL.L, 2, 8);
      case 0x2E: _SRAMEM((r.HL), 2, 16);
      case 0x3F: _SRL(r.AF.A, 2, 8);
      case 0x38: _SRL(r.BC.B, 2, 8);
      case 0x39: _SRL(r.BC.C, 2, 8);
      case 0x3A: _SRL(r.DE.D, 2, 8);
      case 0x3B: _SRL(r.DE.E, 2, 8);
      case 0x3C: _SRL(r.HL.H, 2, 8);
      case 0x3D: _SRL(r.HL.L, 2, 8);
      case 0x3E: _SRLMEM((r.HL), 2, 16);
      case 0x47: _BIT(r.AF.A, 0, 2, 8);
      case 0x40: _BIT(r.BC.B, 0, 2, 8);
      case 0x41: _BIT(r.BC.C, 0, 2, 8);
      case 0x42: _BIT(r.DE.D, 0, 2, 8);
      case 0x43: _BIT(r.DE.E, 0, 2, 8);
      case 0x44: _BIT(r.HL.H, 0, 2, 8);
      case 0x45: _BIT(r.HL.L, 0, 2, 8);
      case 0x46: _BITMEM((r.HL), 0, 2, 16);
      case 0x4F: _BIT(r.AF.A, 1, 2, 8);
      case 0x48: _BIT(r.BC.B, 1, 2, 8);
      case 0x49: _BIT(r.BC.C, 1, 2, 8);
      case 0x4A: _BIT(r.DE.D, 1, 2, 8);
      case 0x4B: _BIT(r.DE.E, 1, 2, 8);
      case 0x4C: _BIT(r.HL.H, 1, 2, 8);
      case 0x4D: _BIT(r.HL.L, 1, 2, 8);
      case 0x4E: _BITMEM((r.HL), 1, 2, 16);
      case 0x57: _BIT(r.AF.A, 2, 2, 8);
      case 0x50: _BIT(r.BC.B, 2, 2, 8);
      case 0x51: _BIT(r.BC.C, 2, 2, 8);
      case 0x52: _BIT(r.DE.D, 2, 2, 8);
      case 0x53: _BIT(r.DE.E, 2, 2, 8);
      case 0x54: _BIT(r.HL.H, 2, 2, 8);
      case 0x55: _BIT(r.HL.L, 2, 2, 8);
      case 0x56: _BITMEM((r.HL), 2, 2, 16);
      case 0x5F: _BIT(r.AF.A, 3, 2, 8);
      case 0x58: _BIT(r.BC.B, 3, 2, 8);
      case 0x59: _BIT(r.BC.C, 3, 2, 8);
      case 0x5A: _BIT(r.DE.D, 3, 2, 8);
      case 0x5B: _BIT(r.DE.E, 3, 2, 8);
      case 0x5C: _BIT(r.HL.H, 3, 2, 8);
      case 0x5D: _BIT(r.HL.L, 3, 2, 8);
      case 0x5E: _BITMEM((r.HL), 3, 2, 16);
      case 0x67: _BIT(r.AF.A, 4, 2, 8);
      case 0x60: _BIT(r.BC.B, 4, 2, 8);
      case 0x61: _BIT(r.BC.C, 4, 2, 8);
      case 0x62: _BIT(r.DE.D, 4, 2, 8);
      case 0x63: _BIT(r.DE.E, 4, 2, 8);
      case 0x64: _BIT(r.HL.H, 4, 2, 8);
      case 0x65: _BIT(r.HL.L, 4, 2, 8);
      case 0x66: _BITMEM((r.HL), 4, 2, 16);
      case 0x6F: _BIT(r.AF.A, 5, 2, 8);
      case 0x68: _BIT(r.BC.B, 5, 2, 8);
      case 0x69: _BIT(r.BC.C, 5, 2, 8);
      case 0x6A: _BIT(r.DE.D, 5, 2, 8);
      case 0x6B: _BIT(r.DE.E, 5, 2, 8);
      case 0x6C: _BIT(r.HL.H, 5, 2, 8);
      case 0x6D: _BIT(r.HL.L, 5, 2, 8);
      case 0x6E: _BITMEM((r.HL), 5, 2, 16);
      case 0x77: _BIT(r.AF.A, 6, 2, 8);
      case 0x70: _BIT(r.BC.B, 6, 2, 8);
      case 0x71: _BIT(r.BC.C, 6, 2, 8);
      case 0x72: _BIT(r.DE.D, 6, 2, 8);
      case 0x73: _BIT(r.DE.E, 6, 2, 8);
      case 0x74: _BIT(r.HL.H, 6, 2, 8);
      case 0x75: _BIT(r.HL.L, 6, 2, 8);
      case 0x76: _BITMEM((r.HL), 6, 2, 16);
      case 0x7F: _BIT(r.AF.A, 7, 2, 8);
      case 0x78: _BIT(r.BC.B, 7, 2, 8);
      case 0x79: _BIT(r.BC.C, 7, 2, 8);
      case 0x7A: _BIT(r.DE.D, 7, 2, 8);
      case 0x7B: _BIT(r.DE.E, 7, 2, 8);
      case 0x7C: _BIT(r.HL.H, 7, 2, 8);
      case 0x7D: _BIT(r.HL.L, 7, 2, 8);
      case 0x7E: _BITMEM((r.HL), 7, 2, 16);
      case 0xC7: _SET(r.AF.A, 0, 2, 8);
      case 0xC0: _SET(r.BC.B, 0, 2, 8);
      case 0xC1: _SET(r.BC.C, 0, 2, 8);
      case 0xC2: _SET(r.DE.D, 0, 2, 8);
      case 0xC3: _SET(r.DE.E, 0, 2, 8);
      case 0xC4: _SET(r.HL.H, 0, 2, 8);
      case 0xC5: _SET(r.HL.L, 0, 2, 8);
      case 0xC6: _SETMEM((r.HL), 0, 2, 16);
      case 0xCC: _SET(r.HL.H, 1, 2, 8);
      case 0xD0: _SET(r.BC.B, 2, 2, 8);
      case 0xD8: _SET(r.BC.B, 3, 2, 8);
      case 0xF0: _SET(r.BC.B, 6, 2, 8);
      case 0xF8: _SET(r.BC.B, 7, 2, 8);
      case 0xDE: _SETMEM((r.HL), 7, 3, 16);
      case 0xFE: _SETMEM((r.HL), 7, 2, 16);
      case 0x87: _RES(r.AF.A, 0, 2, 8);
      case 0x80: _RES(r.BC.B, 0, 2, 8);
      case 0x81: _RES(r.BC.C, 0, 2, 8);
      case 0x82: _RES(r.DE.D, 0, 2, 8);
      case 0x83: _RES(r.DE.E, 0, 2, 8);
      case 0x84: _RES(r.HL.H, 0, 2, 8);
      case 0x85: _RES(r.HL.L, 0, 2, 8);
      case 0x86: _RESMEM((r.HL), 0, 2, 16);
      case 0x9E: _RESMEM((r.HL), 3, 2, 16);
      case 0xBE: _RESMEM((r.HL), 7, 2, 16);
      default:
          std::cerr << "Uninplemented CB op: " << std::hex << op << std::endl;
          abort();
      }
      return{ 4, 2 }; // todo: ?
    }

    static inline CPU::OpFormat gbOp(CPU& cpu, CPU::Registers& r)
    {
        unsigned int op = cpu.read(r.pc + 1);
        switch (op)
        {
        case 0x00: _STOP();
        default:
            std::cerr << "Uninplemented GB op: " << std::hex << op << std::endl;
            abort();
        }
        return{ 4, 2 }; // todo: ?
    }

    CPU::OpFormat runCurrentOp(CPU& cpu)
    {
      CPU::Registers& r = cpu.registers_;
      MMU& mmu = cpu.mmu_;

      unsigned int op = mmu.read(cpu.registers_.pc);
      switch (op)
      {
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
      case 0x7E: _LD(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0x40: _LD(r.BC.B, r.BC.B, 1, 4);
      case 0x41: _LD(r.BC.B, r.BC.C, 1, 4);
      case 0x42: _LD(r.BC.B, r.DE.D, 1, 4);
      case 0x43: _LD(r.BC.B, r.DE.E, 1, 4);
      case 0x44: _LD(r.BC.B, r.HL.H, 1, 4);
      case 0x45: _LD(r.BC.B, r.HL.L, 1, 4);
      case 0x46: _LD(r.BC.B, cpu.read(r.HL), 1, 8);
      case 0x48: _LD(r.BC.C, r.BC.B, 1, 4);
      case 0x49: _LD(r.BC.C, r.BC.C, 1, 4);
      case 0x4A: _LD(r.BC.C, r.DE.D, 1, 4);
      case 0x4B: _LD(r.BC.C, r.DE.E, 1, 4);
      case 0x4C: _LD(r.BC.C, r.HL.H, 1, 4);
      case 0x4D: _LD(r.BC.C, r.HL.L, 1, 4);
      case 0x4E: _LD(r.BC.C, cpu.read(r.HL), 1, 8);
      case 0x50: _LD(r.DE.D, r.BC.B, 1, 4);
      case 0x51: _LD(r.DE.D, r.BC.C, 1, 4);
      case 0x52: _LD(r.DE.D, r.DE.D, 1, 4);
      case 0x53: _LD(r.DE.D, r.DE.E, 1, 4);
      case 0x54: _LD(r.DE.D, r.HL.H, 1, 4);
      case 0x55: _LD(r.DE.D, r.HL.L, 1, 4);
      case 0x56: _LD(r.DE.D, cpu.read(r.HL), 1, 8);
      case 0x58: _LD(r.DE.E, r.BC.B, 1, 4);
      case 0x59: _LD(r.DE.E, r.BC.C, 1, 4);
      case 0x5A: _LD(r.DE.E, r.DE.D, 1, 4);
      case 0x5B: _LD(r.DE.E, r.DE.E, 1, 4);
      case 0x5C: _LD(r.DE.E, r.HL.H, 1, 4);
      case 0x5D: _LD(r.DE.E, r.HL.L, 1, 4);
      case 0x5E: _LD(r.DE.E, cpu.read(r.HL), 1, 8);
      case 0x60: _LD(r.HL.H, r.BC.B, 1, 4);
      case 0x61: _LD(r.HL.H, r.BC.C, 1, 4);
      case 0x62: _LD(r.HL.H, r.DE.D, 1, 4);
      case 0x63: _LD(r.HL.H, r.DE.E, 1, 4);
      case 0x64: _LD(r.HL.H, r.HL.H, 1, 4);
      case 0x65: _LD(r.HL.H, r.HL.L, 1, 4);
      case 0x66: _LD(r.HL.H, cpu.read(r.HL), 1, 8);
      case 0x68: _LD(r.HL.L, r.BC.B, 1, 4);
      case 0x69: _LD(r.HL.L, r.BC.C, 1, 4);
      case 0x6A: _LD(r.HL.L, r.DE.D, 1, 4);
      case 0x6B: _LD(r.HL.L, r.DE.E, 1, 4);
      case 0x6C: _LD(r.HL.L, r.HL.H, 1, 4);
      case 0x6D: _LD(r.HL.L, r.HL.L, 1, 4);
      case 0x6E: _LD(r.HL.L, cpu.read(r.HL), 1, 8);
      case 0x70: _8BLDMEM((r.HL), r.BC.B, 1, 8);
      case 0x71: _8BLDMEM((r.HL), r.BC.C, 1, 8);
      case 0x72: _8BLDMEM((r.HL), r.DE.D, 1, 8);
      case 0x73: _8BLDMEM((r.HL), r.DE.E, 1, 8);
      case 0x74: _8BLDMEM((r.HL), r.HL.H, 1, 8);
      case 0x75: _8BLDMEM((r.HL), r.HL.L, 1, 8);
      case 0x36: _8BLDMEM((r.HL), cpu.read(r.pc + 1), 2, 12);
      case 0x0A: _LD(r.AF.A, cpu.read(r.BC), 1, 8);
      case 0x1A: _LD(r.AF.A, cpu.read(r.DE), 1, 8);
      case 0xFA: _LD(r.AF.A, cpu.read(cpu.readw(r.pc + 1)), 3, 16); // !! conflict ?
      case 0x3E: _LD(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x47: _LD(r.BC.B, r.AF.A, 1, 4);
      case 0x4F: _LD(r.BC.C, r.AF.A, 1, 4);
      case 0x57: _LD(r.DE.D, r.AF.A, 1, 4);
      case 0x5F: _LD(r.DE.E, r.AF.A, 1, 4);
      case 0x67: _LD(r.HL.H, r.AF.A, 1, 4);
      case 0x6F: _LD(r.HL.L, r.AF.A, 1, 4);
      case 0x02: _8BLDMEM((r.BC), r.AF.A, 1, 8);
      case 0x12: _8BLDMEM((r.DE), r.AF.A, 1, 8);
      case 0x77: _8BLDMEM((r.HL), r.AF.A, 1, 8);
      case 0xEA: _8BLDMEM((cpu.readw(r.pc + 1)), r.AF.A, 3, 16);
      case 0xF2: _LD(r.AF.A, cpu.read(0xFF00 + r.BC.C), 1, 8);
      case 0xE2: _8BLDMEM((0xFF00 + r.BC.C), r.AF.A, 1, 8);
      case 0x3A: _LD(r.AF.A, cpu.read(r.HL--), 1, 8);
      case 0x32: _8BLDMEM(r.HL--, r.AF.A, 1, 8);
      case 0x2A: _LD(r.AF.A, cpu.read(r.HL++), 1, 8);
      case 0x22: _8BLDMEM(r.HL++, r.AF.A, 1, 8);
      case 0xE0: _8BLDMEM((0xFF00 + cpu.read(r.pc + 1)), r.AF.A, 2, 12);
      case 0xF0: _LD(r.AF.A, cpu.read(0xFF00 + cpu.read(r.pc + 1)), 2, 12);
      case 0x01: _LD(r.BC, cpu.readw(r.pc + 1), 3, 12);
      case 0x11: _LD(r.DE, cpu.readw(r.pc + 1), 3, 12);
      case 0x21: _LD(r.HL, cpu.readw(r.pc + 1), 3, 12);
      case 0x31: _LD(r.sp, cpu.readw(r.pc + 1), 3, 12);
      case 0xF9: _LD(r.sp, r.HL, 1, 8);
      case 0xF8: {
          FRESET(Z);
          FRESET(N);
          unsigned int op = r.sp;
          int rh = (int8_t)cpu.read(r.pc + 1);
          FASSIGN(H, _8BHCARRY(op, rh));
          FASSIGN(C, _8BCARRY(op, rh));
          _LD(r.HL, op + rh, 2, 12);
      }
      case 0x08: _8BLDMEM((cpu.readw(r.pc + 1)), r.sp, 3, 20);
          // conflit sur les 4 en dessous: verifier
      case 0xF5: _PUSH16(r.AF, 1, 16);
      case 0xC5: _PUSH16(r.BC, 1, 16);
      case 0xD5: _PUSH16(r.DE, 1, 16);
      case 0xE5: _PUSH16(r.HL, 1, 16);
      case 0xF1: _POP16(r.AF, 1, 12);
      case 0xC1: _POP16(r.BC, 1, 12);
      case 0xD1: _POP16(r.DE, 1, 12);
      case 0xE1: _POP16(r.HL, 1, 12);
      case 0x87: _ADD8(r.AF.A, r.AF.A, 1, 4);
      case 0x80: _ADD8(r.AF.A, r.BC.B, 1, 4);
      case 0x81: _ADD8(r.AF.A, r.BC.C, 1, 4);
      case 0x82: _ADD8(r.AF.A, r.DE.D, 1, 4);
      case 0x83: _ADD8(r.AF.A, r.DE.E, 1, 4);
      case 0x84: _ADD8(r.AF.A, r.HL.H, 1, 4);
      case 0x85: _ADD8(r.AF.A, r.HL.L, 1, 4);
      case 0x86: _ADD8(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0xC6: _ADD8(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x8F: _ADD8(r.AF.A, r.AF.A + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x88: _ADD8(r.AF.A, r.BC.B + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x89: _ADD8(r.AF.A, r.BC.C + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8A: _ADD8(r.AF.A, r.DE.D + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8B: _ADD8(r.AF.A, r.DE.E + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8C: _ADD8(r.AF.A, r.HL.H + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8D: _ADD8(r.AF.A, r.HL.L + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x8E: _ADD8(r.AF.A, cpu.read(r.HL) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 8);
      case 0xCE: _ADD8(r.AF.A, cpu.read(r.pc + 1) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 2, 8);
      case 0x97: _SUB8(r.AF.A, r.AF.A, 1, 4);
      case 0x90: _SUB8(r.AF.A, r.BC.B, 1, 4);
      case 0x91: _SUB8(r.AF.A, r.BC.C, 1, 4);
      case 0x92: _SUB8(r.AF.A, r.DE.D, 1, 4);
      case 0x93: _SUB8(r.AF.A, r.DE.E, 1, 4);
      case 0x94: _SUB8(r.AF.A, r.HL.H, 1, 4);
      case 0x95: _SUB8(r.AF.A, r.HL.L, 1, 4);
      case 0x96: _SUB8(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0xD6: _SUB8(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x9F: _SUB8(r.AF.A, r.AF.A + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x98: _SUB8(r.AF.A, r.BC.B + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x99: _SUB8(r.AF.A, r.BC.C + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9A: _SUB8(r.AF.A, r.DE.D + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9B: _SUB8(r.AF.A, r.DE.E + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9C: _SUB8(r.AF.A, r.HL.H + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9D: _SUB8(r.AF.A, r.HL.L + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 4);
      case 0x9E: _SUB8(r.AF.A, cpu.read(r.HL) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 1, 8);
      case 0xDE: _ADD8(r.AF.A, cpu.read(r.pc + 1) + !!(r.AF.F & (unsigned int)CPU::Flags::C), 2, 8);
      case 0xA7: _AND(r.AF.A, r.AF.A, 1, 4);
      case 0xA0: _AND(r.AF.A, r.BC.B, 1, 4);
      case 0xA1: _AND(r.AF.A, r.BC.C, 1, 4);
      case 0xA2: _AND(r.AF.A, r.DE.D, 1, 4);
      case 0xA3: _AND(r.AF.A, r.DE.E, 1, 4);
      case 0xA4: _AND(r.AF.A, r.HL.H, 1, 4);
      case 0xA5: _AND(r.AF.A, r.HL.L, 1, 4);
      case 0xA6: _AND(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0xE6: _AND(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0xB7: _OR(r.AF.A, r.AF.A, 1, 4);
      case 0xB0: _OR(r.AF.A, r.BC.B, 1, 4);
      case 0xB1: _OR(r.AF.A, r.BC.C, 1, 4);
      case 0xB2: _OR(r.AF.A, r.DE.D, 1, 4);
      case 0xB3: _OR(r.AF.A, r.DE.E, 1, 4);
      case 0xB4: _OR(r.AF.A, r.HL.H, 1, 4);
      case 0xB5: _OR(r.AF.A, r.HL.L, 1, 4);
      case 0xB6: _OR(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0xF6: _OR(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0xAF: _XOR(r.AF.A, r.AF.A, 1, 4);
      case 0xA8: _XOR(r.AF.A, r.BC.B, 1, 4);
      case 0xA9: _XOR(r.AF.A, r.BC.C, 1, 4);
      case 0xAA: _XOR(r.AF.A, r.DE.D, 1, 4);
      case 0xAB: _XOR(r.AF.A, r.DE.E, 1, 4);
      case 0xAC: _XOR(r.AF.A, r.HL.H, 1, 4);
      case 0xAD: _XOR(r.AF.A, r.HL.L, 1, 4);
      case 0xAE: _XOR(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0xEE: _XOR(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0xBF: _CP(r.AF.A, r.AF.A, 1, 4);
      case 0xB8: _CP(r.AF.A, r.BC.B, 1, 4);
      case 0xB9: _CP(r.AF.A, r.BC.C, 1, 4);
      case 0xBA: _CP(r.AF.A, r.DE.D, 1, 4);
      case 0xBB: _CP(r.AF.A, r.DE.E, 1, 4);
      case 0xBC: _CP(r.AF.A, r.HL.H, 1, 4);
      case 0xBD: _CP(r.AF.A, r.HL.L, 1, 4);
      case 0xBE: _CP(r.AF.A, cpu.read(r.HL), 1, 8);
      case 0xFE: _CP(r.AF.A, cpu.read(r.pc + 1), 2, 8);
      case 0x3C: _INC(r.AF.A, 1, 4);
      case 0x04: _INC(r.BC.B, 1, 4);
      case 0x0C: _INC(r.BC.C, 1, 4);
      case 0x14: _INC(r.DE.D, 1, 4);
      case 0x1C: _INC(r.DE.E, 1, 4);
      case 0x24: _INC(r.HL.H, 1, 4);
      case 0x2C: _INC(r.HL.L, 1, 4);
      case 0x34: _INCMEM((r.HL), 1, 12);
      case 0x3D: _DEC(r.AF.A, 1, 4);
      case 0x05: _DEC(r.BC.B, 1, 4);
      case 0x0D: _DEC(r.BC.C, 1, 4);
      case 0x15: _DEC(r.DE.D, 1, 4);
      case 0x1D: _DEC(r.DE.E, 1, 4);
      case 0x25: _DEC(r.HL.H, 1, 4);
      case 0x2D: _DEC(r.HL.L, 1, 4);
      case 0x35: _DECMEM((r.HL), 1, 12);
      case 0x09: _ADD16(r.HL, r.BC, 1, 8);
      case 0x19: _ADD16(r.HL, r.DE, 1, 8);
      case 0x29: _ADD16(r.HL, r.HL, 1, 8);
      case 0x39: _ADD16(r.HL, r.sp, 1, 8);
      case 0xE8: FRESET(Z); _ADD16(r.sp, (int)cpu.read(r.pc + 1), 2, 16); // reverifier si ADD ou ADDMEM ?
      case 0x03: _INCREG(r.BC, 1, 8);
      case 0x13: _INCREG(r.DE, 1, 8);
      case 0x23: _INCREG(r.HL, 1, 8);
      case 0x33: _INCREG(r.sp, 1, 8);
      case 0x0B: _DECREG(r.BC, 1, 8);
      case 0x1B: _DECREG(r.DE, 1, 8);
      case 0x2B: _DECREG(r.HL, 1, 8);
      case 0x3B: _DECREG(r.sp, 1, 8);
      case 0xCB: return bitOp(cpu, r);
      case 0x27: return DAA(cpu, r);
      case 0x2F: _CPL(r.AF.A, 1, 4);
      case 0x3F: _CCF();
      case 0x37: _SCF();
      case 0x00: case 0xE4: case 0xEC:
      case 0xF4: case 0xFC: case 0xFD:
      case 0xEB: case 0xDD: case 0xE3:
      case 0xED: case 0xDB: case 0xD3:
          _NOP();
      case 0x76: _HALT();
      case 0x10: _STOP();
      case 0xF3: _DI_();
      case 0xFB: _EI();
      case 0x07: _RLC(r.AF.A, 1, 4);
      case 0x17: _RL(r.AF.A, 1, 4);
      case 0x0F: _RRC(r.AF.A, 1, 4);
      case 0x1F: _RR(r.AF.A, 1, 4);
      case 0xC3: _JP();
      case 0xC2: _JPNZ();
      case 0xCA: _JPZ();
      case 0xD2: _JPNC();
      case 0xDA: _JPC();
      case 0xE9: _JPHL();
      case 0x18: _JR();
      case 0x20: _JRNZ();
      case 0x28: _JRZ();
      case 0x30: _JRNC();
      case 0x38: _JRC();
      case 0xCD: _CALL();
      case 0xC4: _CALLNZ();
      case 0xCC: _CALLZ();
      case 0xD4: _CALLNC();
      case 0xDC: _CALLC();
      case 0xC7: _RST(0x0);
      case 0xCF: _RST(0x8);
      case 0xD7: _RST(0x10);
      case 0xDF: _RST(0x18);
      case 0xE7: _RST(0x20);
      case 0xEF: _RST(0x28);
      case 0xF7: _RST(0x30);
      case 0xFF: _RST(0x38);
      case 0xC9: _RET();
      case 0xC0: _RETNZ();
      case 0xC8: _RETZ();
      case 0xD0: _RETNC();
      case 0xD8: _RETC();
      case 0xD9: _RETI();
      default:
        std::cerr << "Undefined opcode " << std::hex << op << ", treating as NOP" << std::endl;
        abort();
        return {4,1};
      }
    }

}
