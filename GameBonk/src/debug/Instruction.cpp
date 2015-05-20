#include <iostream>
#include <iomanip>
#include <sstream>

#include "debug/Instruction.hpp"

namespace GBonk
{

    namespace Debug {

        static inline std::string stringify(unsigned int v)
        {
            std::stringstream builder;

            builder << "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << v;
            return builder.str();
        }

        static inline std::string stringifyw(unsigned int v)
        {
            std::stringstream builder;

            builder << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << v;
            return builder.str();
        }

        Instruction::Instruction(const uint8_t* addr, unsigned int off)
            : mnemo_(""),
            addr_(off),
            len_(0),
            params_()
        {
            bytecode_.reserve(3);
            bytecode_.push_back(*addr);
            decodeInstruction_(addr);
        }

        Instruction::Instruction(const Instruction& i)
            : mnemo_(i.mnemo_),
            bytecode_(i.bytecode_),
            addr_(i.addr_),
            len_(i.len_),
            params_(i.params_)
        {

        }

        Instruction::Instruction(Instruction&& i)
            : mnemo_(i.mnemo_),
            bytecode_(i.bytecode_),
            addr_(i.addr_),
            len_(i.len_),
            params_(i.params_)
        {
            i.mnemo_ = std::string();
            i.bytecode_ = std::vector<unsigned int>();
            i.addr_ = 0;
            i.len_ = 0;
            i.params_ = std::vector<std::string>();
        }

        std::string Instruction::toString() const
        {
            std::stringstream builder;
            int bcount = 0;
            for (auto& b : bytecode_)
            {
                builder << " " << stringify(b);
                bcount++;
            }
            while (bcount++ < 3)
                builder << "     ";

            builder << " " << mnemo_;

            for (auto& p : params_)
                builder << " " << p;

            switch (bytecode_[0])
            {
            case 0x18:
            case 0x20:
            case 0x28:
            case 0x30:
            case 0x38:
                //std::cerr << "adding " << std::hex << addr_ << std::dec << "," << len_ << "," << (int)int8_t(bytecode_[1] & 0xFF) << " " << std::hex << bytecode_[1] << std::endl;
                //std::getchar();
            {
                int add = (int8_t)bytecode_[1] + (int)addr_ + (int)len_;
                builder << " ; 0x" << std::hex << add;
                break;
            }
            default:
                break;
            }

            return builder.str();
        }

#define _LD(P1, P2, LEN) { mnemo_ = "LD"; params_.push_back(P1); params_.push_back(P2); len_ = LEN; break; }
#define _LDD(P1, P2) { mnemo_ = "LDD"; params_.push_back(P1); params_.push_back(P2); len_ = 1; break;}
#define _LDI(P1, P2) {mnemo_ = "LDI"; params_.push_back(P1); params_.push_back(P2); len_ = 1; break;}
#define _LDH(P1, P2) { mnemo_ = "LDH"; params_.push_back(P1); params_.push_back(P2); len_ = 2; break;}
#define _LDHL() {mnemo_ = "LDHL"; params_.push_back("SP"); params_.push_back(stringify(bc(addr[1]))); len_ = 2; break;}
#define _PUSH16(P1, LEN) {mnemo_ = "PUSH"; params_.push_back(P1); len_ = LEN; break;}
#define _POP16(P1) { mnemo_ = "POP"; params_.push_back(P1); len_ = 1; break;}
#define _DAA() { mnemo_ = "DAA"; len_ = 1; break; }
#define _ADD(P1, P2, LEN) { mnemo_ = "ADD"; params_.push_back(P1); params_.push_back(P2); len_ = LEN; break;}
#define _ADC(P1, P2, LEN) { mnemo_ = "ADC"; params_.push_back(P1); params_.push_back(P2); len_ = LEN; break;}
#define _SUB8(P1, P2, LEN) { mnemo_ = "SUB"; params_.push_back(P1); params_.push_back(P2); len_ = LEN; break;}
#define _SBC(P1, P2, LEN) { mnemo_ = "SBC"; params_.push_back(P1); params_.push_back(P2); len_ = LEN; break;}
#define _AND(P1, P2, LEN) {mnemo_ = "AND"; params_.push_back(P1); params_.push_back(P2); len_ = LEN;break;}
#define _OR(P1, P2, LEN) {mnemo_ = "OR"; params_.push_back(P1); params_.push_back(P2); len_ = LEN;break;}
#define _XOR(P1, P2, LEN) {mnemo_ = "XOR"; params_.push_back(P1); params_.push_back(P2); len_ = LEN;break;}
#define _CP(P1, P2, LEN) {mnemo_ = "CP"; params_.push_back(P1); params_.push_back(P2); len_ = LEN;break;}
#define _INC(P1) {mnemo_ = "INC"; params_.push_back(P1); len_ = 1;break;}
#define _DEC(P1) {mnemo_ = "DEC"; params_.push_back(P1); len_ = 1;break;}
#define _SWAP(P1) {mnemo_ = "SWAP"; params_.push_back(P1); len_ = 2; break;}
#define _CPL() {mnemo_ = "CPL"; len_ = 1;break;}
#define _CCF() {mnemo_ = "CCF"; len_ = 1;break;}
#define _SCF() {mnemo_ = "SCF"; len_ = 1;break;}
#define _NOP() {mnemo_ = "NOP"; len_ = 1;break;}
#define _HALT() {mnemo_ = "HALT"; len_ = 1;break;}
#define _STOP() {mnemo_ = "STOP"; len_ = 2;break;}
#define _DI_() {mnemo_ = "DI"; len_ = 1; break;}
#define _EI() {mnemo_ = "EI"; len_ = 1; break;}
#define _RLCA() {mnemo_ = "RLCA"; len_ = 1;break;}
#define _RLA() {mnemo_ = "RLA"; len_ = 1;break;}
#define _RRCA() {mnemo_ = "RRCA"; len_ = 1; break;}
#define _RRA() {mnemo_ = "RRA"; len_ = 1; break;}
#define _RLC(P1) { mnemo_ = "RLC"; params_.push_back(P1); len_ = 2; break;}
#define _RL(P1) { mnemo_ = "RL"; params_.push_back(P1); len_ = 2; break; }
#define _RRC(P1) {mnemo_ = "RRC"; params_.push_back(P1); len_ = 2; break;}
#define _RR(P1) {mnemo_ = "RR"; params_.push_back(P1); len_ = 2; break;}
#define _SLA(P1) {mnemo_ = "SLA"; params_.push_back(P1); len_ = 2; break;}
#define _SRA(P1) {mnemo_ = "SRA"; params_.push_back(P1); len_ = 2; break;}
#define _SRL(P1) {mnemo_ = "SRL"; params_.push_back(P1); len_ = 2; break;}
#define _BIT(P1, P2) {mnemo_ = "BIT"; params_.push_back(P1); params_.push_back(P2); len_ = 2; break;}
#define _SET(P1, P2) {mnemo_ = "SET"; params_.push_back(P1); params_.push_back(P2); len_ = 2; break;}
#define _RES(P1, P2) {mnemo_ = "RES"; params_.push_back(P1); params_.push_back(P2); len_ = 2; break;}
#define _JP() {mnemo_ = "JP"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _JPNZ() {mnemo_ = "JPNZ"; params_.push_back(stringifyw(bc2(readw(addr + 1))));  len_ = 3; break;}
#define _JPZ() {mnemo_ = "JPZ"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _JPNC() {mnemo_ = "JPNC"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _JPC() {mnemo_ = "JPC"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _JPHL() {mnemo_ = "JP"; params_.push_back("(HL)"); len_ = 1; break;}
#define _JR() {mnemo_ = "JR"; params_.push_back(stringify(bc(addr[1]))); len_ = 2; break;} 
#define _JRNZ() {mnemo_ = "JRNZ"; params_.push_back(stringify(bc(addr[1]))); len_ = 2; break;}
#define _JRZ() {mnemo_ = "JRZ"; params_.push_back(stringify(bc(addr[1]))); len_ = 2; break;}
#define _JRNC() {mnemo_ = "JRNC"; params_.push_back(stringify(bc(addr[1]))); len_ = 2; break;}
#define _JRC() {mnemo_ = "JRC"; params_.push_back(stringify(bc(addr[1]))); len_ = 2; break;}
#define _CALL() {mnemo_ = "CALL"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _CALLNZ() {mnemo_ = "CALLNZ"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _CALLZ() {mnemo_ = "CALLZ"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _CALLNC() {mnemo_ = "CALLNC"; params_.push_back(stringifyw(bc2(readw(addr + 1)))); len_ = 3; break;}
#define _CALLC() {mnemo_ = "CALLC"; params_.push_back(stringifyw(bc2(readw(addr + 1))));  len_ = 3; break;}
#define _RST(P1) {mnemo_ = "RST"; params_.push_back(P1); len_ = 1; break;}
#define _RET() {mnemo_ = "RET"; len_ = 1; break;}
#define _RETNZ() {mnemo_ = "RETNZ"; len_ = 1; break;}
#define _RETZ() {mnemo_ = "RETZ"; len_ = 1; break;}
#define _RETNC() {mnemo_ = "RETNC"; len_ = 1; break;}
#define _RETC() {mnemo_ = "RETC"; len_ = 1; break;}
#define _RETI() {mnemo_ = "RETI"; len_ = 1; break;}

        std::string deref(const std::string& in)
        {
            return std::string("(") + in + std::string(")");
        }

        unsigned int readw(const uint8_t* addr)
        {
            return addr[0] | addr[1] << 8;
        }

        void Instruction::decodeBitInstruction_(const uint8_t* addr)
        {
            bytecode_.push_back(addr[0]);
            switch (addr[0])
            {
            case 0x37: _SWAP("A");
            case 0x30: _SWAP("B");
            case 0x31: _SWAP("C");
            case 0x32: _SWAP("D");
            case 0x33: _SWAP("E");
            case 0x34: _SWAP("H");
            case 0x35: _SWAP("L");
            case 0x36: _SWAP("(HL)");
            case 0x07: _RLC("A");
            case 0x00: _RLC("B");
            case 0x01: _RLC("C");
            case 0x02: _RLC("D");
            case 0x03: _RLC("E");
            case 0x04: _RLC("H");
            case 0x05: _RLC("L");
            case 0x06: _RLC("(HL)");
            case 0x17: _RL("A");
            case 0x10: _RL("B");
            case 0x11: _RL("C");
            case 0x12: _RL("D");
            case 0x13: _RL("E");
            case 0x14: _RL("H");
            case 0x15: _RL("L");
            case 0x16: _RL("(HL)");
            case 0x0F: _RRC("A");
            case 0x08: _RRC("B");
            case 0x09: _RRC("C");
            case 0x0A: _RRC("D");
            case 0x0B: _RRC("E");
            case 0x0C: _RRC("H");
            case 0x0D: _RRC("L");
            case 0x0E: _RRC("(HL)");
            case 0x1F: _RR("A");
            case 0x18: _RR("B");
            case 0x19: _RR("C");
            case 0x1A: _RR("D");
            case 0x1B: _RR("E");
            case 0x1C: _RR("H");
            case 0x1D: _RR("L");
            case 0x1E: _RR("(HL)");
            case 0x27: _SLA("A");
            case 0x20: _SLA("B");
            case 0x21: _SLA("C");
            case 0x22: _SLA("D");
            case 0x23: _SLA("E");
            case 0x24: _SLA("H");
            case 0x25: _SLA("L");
            case 0x26: _SLA("(HL)");
            case 0x2F: _SRA("A");
            case 0x28: _SRA("B");
            case 0x29: _SRA("C");
            case 0x2A: _SRA("D");
            case 0x2B: _SRA("E");
            case 0x2C: _SRA("H");
            case 0x2D: _SRA("L");
            case 0x2E: _SRA("(HL)");
            case 0x3F: _SRL("A");
            case 0x38: _SRL("B");
            case 0x39: _SRL("C");
            case 0x3A: _SRL("D");
            case 0x3B: _SRL("E");
            case 0x3C: _SRL("H");
            case 0x3D: _SRL("L");
            case 0x3E: _SRL("(HL)");
            case 0x47: _BIT("0", "A");
            case 0x40: _BIT("0", "B");
            case 0x41: _BIT("0", "C");
            case 0x42: _BIT("0", "D");
            case 0x43: _BIT("0", "E");
            case 0x44: _BIT("0", "H");
            case 0x45: _BIT("0", "L");
            case 0x46: _BIT("0", "(HL)");
            case 0x4F: _BIT("1", "A");
            case 0x48: _BIT("1", "B");
            case 0x49: _BIT("1", "C");
            case 0x4A: _BIT("1", "D");
            case 0x4B: _BIT("1", "E");
            case 0x4C: _BIT("1", "H");
            case 0x4D: _BIT("1", "L");
            case 0x4E: _BIT("1", "(HL)");
            case 0x57: _BIT("2", "A");
            case 0x50: _BIT("2", "B");
            case 0x51: _BIT("2", "C");
            case 0x52: _BIT("2", "D");
            case 0x53: _BIT("2", "E");
            case 0x54: _BIT("2", "H");
            case 0x55: _BIT("2", "L");
            case 0x56: _BIT("2", "(HL)");
            case 0x5F: _BIT("3", "A");
            case 0x58: _BIT("3", "B");
            case 0x59: _BIT("3", "C");
            case 0x5A: _BIT("3", "D");
            case 0x5B: _BIT("3", "E");
            case 0x5C: _BIT("3", "H");
            case 0x5D: _BIT("3", "L");
            case 0x5E: _BIT("3", "(HL)");
            case 0x67: _BIT("4", "A");
            case 0x60: _BIT("4", "B");
            case 0x61: _BIT("4", "C");
            case 0x62: _BIT("4", "D");
            case 0x63: _BIT("4", "E");
            case 0x64: _BIT("4", "H");
            case 0x65: _BIT("4", "L");
            case 0x66: _BIT("4", "(HL)");
            case 0x6F: _BIT("5", "A");
            case 0x68: _BIT("5", "B");
            case 0x69: _BIT("5", "C");
            case 0x6A: _BIT("5", "D");
            case 0x6B: _BIT("5", "E");
            case 0x6C: _BIT("5", "H");
            case 0x6D: _BIT("5", "L");
            case 0x6E: _BIT("5", "(HL)");
            case 0x77: _BIT("6", "A");
            case 0x70: _BIT("6", "B");
            case 0x71: _BIT("6", "C");
            case 0x72: _BIT("6", "D");
            case 0x73: _BIT("6", "E");
            case 0x74: _BIT("6", "H");
            case 0x75: _BIT("6", "L");
            case 0x76: _BIT("6", "(HL)");
            case 0x7F: _BIT("7", "A");
            case 0x78: _BIT("7", "B");
            case 0x79: _BIT("7", "C");
            case 0x7A: _BIT("7", "D");
            case 0x7B: _BIT("7", "E");
            case 0x7C: _BIT("7", "H");
            case 0x7D: _BIT("7", "L");
            case 0xDE: _BIT("3", "(HL)");
            case 0x7E: _BIT("7", "(HL)");
            case 0xC7: _SET("0", "A");
            case 0xC0: _SET("0", "B");
            case 0xC1: _SET("0", "C");
            case 0xC2: _SET("0", "D");
            case 0xC3: _SET("0", "E");
            case 0xC4: _SET("0", "H");
            case 0xC5: _SET("0", "L");
            case 0xC6: _SET("0", "(HL)");
            case 0xCC: _SET("1", "H");
            case 0xD0: _SET("2", "B");
            case 0xD8: _SET("3", "B");
            case 0xF0: _SET("6", "B");
            case 0xF8: _SET("7", "B");
            case 0xFE: _SET("7", "(HL)");
            case 0x87: _RES("0", "A");
            case 0x80: _RES("0", "B");
            case 0x81: _RES("0", "C");
            case 0x82: _RES("0", "D");
            case 0x83: _RES("0", "E");
            case 0x84: _RES("0", "H");
            case 0x85: _RES("0", "L");
            case 0x86: _RES("0", "(HL)");
            case 0x9E: _RES("3", "(HL)");
            case 0xBE: _RES("7", "(HL)");
            }
        }

        void Instruction::decodeGbInstruction_(const uint8_t* addr)
        {
            bytecode_.push_back(addr[0]);
            switch (addr[0])
            {
            case 0x00: _STOP();
            }
        }

        void Instruction::decodeInstruction_(const uint8_t* addr)
        {
            switch (bytecode_[0])
            {
            case 0x06: _LD("B", stringify(bc(addr[1])), 2);
            case 0x0E: _LD("C", stringify(bc(addr[1])), 2);
            case 0x16: _LD("D", stringify(bc(addr[1])), 2);
            case 0x1E: _LD("E", stringify(bc(addr[1])), 2);
            case 0x26: _LD("H", stringify(bc(addr[1])), 2);
            case 0x2E: _LD("L", stringify(bc(addr[1])), 2);
            case 0x7F: _LD("A", "A", 1);
            case 0x78: _LD("A", "B", 1);
            case 0x79: _LD("A", "C", 1);
            case 0x7A: _LD("A", "D", 1);
            case 0x7B: _LD("A", "E", 1);
            case 0x7C: _LD("A", "H", 1);
            case 0x7D: _LD("A", "L", 1);
            case 0x7E: _LD("A", "(HL)", 1);
            case 0x40: _LD("B", "B", 1);
            case 0x41: _LD("B", "C", 1);
            case 0x42: _LD("B", "D", 1);
            case 0x43: _LD("B", "E", 1);
            case 0x44: _LD("B", "H", 1);
            case 0x45: _LD("B", "L", 1);
            case 0x46: _LD("B", "(HL)", 1);
            case 0x48: _LD("C", "B", 1);
            case 0x49: _LD("C", "C", 1);
            case 0x4A: _LD("C", "D", 1);
            case 0x4B: _LD("C", "E", 1);
            case 0x4C: _LD("C", "H", 1);
            case 0x4D: _LD("C", "L", 1);
            case 0x4E: _LD("C", "(HL)", 1);
            case 0x50: _LD("D", "B", 1);
            case 0x51: _LD("D", "C", 1);
            case 0x52: _LD("D", "D", 1);
            case 0x53: _LD("D", "E", 1);
            case 0x54: _LD("D", "H", 1);
            case 0x55: _LD("D", "L", 1);
            case 0x56: _LD("D", "(HL)", 1);
            case 0x58: _LD("E", "B", 1);
            case 0x59: _LD("E", "C", 1);
            case 0x5A: _LD("E", "D", 1);
            case 0x5B: _LD("E", "E", 1);
            case 0x5C: _LD("E", "H", 1);
            case 0x5D: _LD("E", "L", 1);
            case 0x5E: _LD("E", "(HL)", 1);
            case 0x60: _LD("H", "B", 1);
            case 0x61: _LD("H", "C", 1);
            case 0x62: _LD("H", "D", 1);
            case 0x63: _LD("H", "E", 1);
            case 0x64: _LD("H", "H", 1);
            case 0x65: _LD("H", "L", 1);
            case 0x66: _LD("H", "(HL)", 1);
            case 0x68: _LD("L", "B", 1);
            case 0x69: _LD("L", "C", 1);
            case 0x6A: _LD("L", "D", 1);
            case 0x6B: _LD("L", "E", 1);
            case 0x6C: _LD("L", "H", 1);
            case 0x6D: _LD("L", "L", 1);
            case 0x6E: _LD("L", "(HL)", 1);
            case 0x70: _LD("(HL)", "B", 1);
            case 0x71: _LD("(HL)", "C", 1);
            case 0x72: _LD("(HL)", "D", 1);
            case 0x73: _LD("(HL)", "E", 1);
            case 0x74: _LD("(HL)", "H", 1);
            case 0x75: _LD("(HL)", "L", 1);
            case 0x36: _LD("(HL)", stringify(bc(addr[1])), 2);
            case 0x0A: _LD("A", "(BC)", 1);
            case 0x1A: _LD("A", "(DE)", 1);
            case 0xFA: _LD("A", deref(stringifyw(bc2(readw(addr + 1)))), 3);
            case 0x3E: _LD("A", stringify(bc(addr[1])), 2);
            case 0x47: _LD("B", "A", 1);
            case 0x4F: _LD("C", "A", 1);
            case 0x57: _LD("D", "A", 1);
            case 0x5F: _LD("E", "A", 1);
            case 0x67: _LD("H", "A", 1);
            case 0x6F: _LD("L", "A", 1);
            case 0x02: _LD("(BC)", "A", 1);
            case 0x12: _LD("(DE)", "A", 1);
            case 0x77: _LD("(HL)", "A", 1);
            case 0xEA: _LD(deref(stringifyw(bc2(readw(addr + 1)))), "A", 3);
            case 0xF2: _LD("A", deref(stringifyw(0xFF00) + " + C"), 1);
            case 0xE2: _LD(deref(stringifyw(0xFF00) + " + C"), "A", 1);
            case 0x3A: _LDD("A", "(HL)");
            case 0x32: _LDD("(HL)", "A");
            case 0x2A: _LDI("A", "(HL)");
            case 0x22: _LDI("(HL)", "A");
            case 0xE0: _LDH(deref(stringify(bc(addr[1]))), "A");
            case 0xF0: _LDH("A", deref(stringify(bc(addr[1]))));
            case 0x01: _LD("BC", stringifyw(bc2(readw(addr + 1))), 3);
            case 0x11: _LD("DE", stringifyw(bc2(readw(addr + 1))), 3);
            case 0x21: _LD("HL", stringifyw(bc2(readw(addr + 1))), 3);
            case 0x31: _LD("SP", stringifyw(bc2(readw(addr + 1))), 3);
            case 0xF9: _LD("SP", "HL", 1);
            case 0xF8: _LDHL();
            case 0x08: _LD(deref(stringifyw(bc2(readw(addr + 1)))), "SP", 3);
                // conflit sur les 4 en dessous: verifier
            case 0xF5: _PUSH16("AF", 1);
            case 0xC5: _PUSH16("BC", 1);
            case 0xD5: _PUSH16("DE", 1);
            case 0xE5: _PUSH16("HL", 1);
            case 0xF1: _POP16("AF");
            case 0xC1: _POP16("BC");
            case 0xD1: _POP16("DE");
            case 0xE1: _POP16("HL");
            case 0x87: _ADD("A", "A", 1);
            case 0x80: _ADD("A", "B", 1);
            case 0x81: _ADD("A", "C", 1);
            case 0x82: _ADD("A", "D", 1);
            case 0x83: _ADD("A", "E", 1);
            case 0x84: _ADD("A", "H", 1);
            case 0x85: _ADD("A", "L", 1);
            case 0x86: _ADD("A", "(HL)", 1);
            case 0xC6: _ADD("A", stringify(bc(addr[1])), 2);
            case 0x8F: _ADC("A", "A", 1);
            case 0x88: _ADC("A", "B", 1);
            case 0x89: _ADC("A", "C", 1);
            case 0x8A: _ADC("A", "D", 1);
            case 0x8B: _ADC("A", "E", 1);
            case 0x8C: _ADC("A", "H", 1);
            case 0x8D: _ADC("A", "L", 1);
            case 0x8E: _ADC("A", "(HL)", 1);
            case 0xCE: _ADC("A", stringify(bc(addr[1])), 2);
            case 0x97: _SUB8("A", "A", 1);
            case 0x90: _SUB8("A", "B", 1);
            case 0x91: _SUB8("A", "C", 1);
            case 0x92: _SUB8("A", "D", 1);
            case 0x93: _SUB8("A", "E", 1);
            case 0x94: _SUB8("A", "H", 1);
            case 0x95: _SUB8("A", "L", 1);
            case 0x96: _SUB8("A", "(HL)", 1);
            case 0xD6: _SUB8("A", stringify(bc(addr[1])), 2);
            case 0x9F: _SBC("A", "A", 1);
            case 0x98: _SBC("A", "B", 1);
            case 0x99: _SBC("A", "C", 1);
            case 0x9A: _SBC("A", "D", 1);
            case 0x9B: _SBC("A", "E", 1);
            case 0x9C: _SBC("A", "H", 1);
            case 0x9D: _SBC("A", "L", 1);
            case 0x9E: _SBC("A", "(HL)", 1);
            case 0xDE: _SBC("A", stringify(bc(addr[1])), 2);
            case 0xA7: _AND("A", "A", 1);
            case 0xA0: _AND("A", "B", 1);
            case 0xA1: _AND("A", "C", 1);
            case 0xA2: _AND("A", "D", 1);
            case 0xA3: _AND("A", "E", 1);
            case 0xA4: _AND("A", "H", 1);
            case 0xA5: _AND("A", "L", 1);
            case 0xA6: _AND("A", "(HL)", 1);
            case 0xE6: _AND("A", stringify(bc(addr[1])), 2);
            case 0xB7: _OR("A", "A", 1);
            case 0xB0: _OR("A", "B", 1);
            case 0xB1: _OR("A", "C", 1);
            case 0xB2: _OR("A", "D", 1);
            case 0xB3: _OR("A", "E", 1);
            case 0xB4: _OR("A", "H", 1);
            case 0xB5: _OR("A", "L", 1);
            case 0xB6: _OR("A", "(HL)", 1);
            case 0xF6: _OR("A", stringify(bc(addr[1])), 2);
            case 0xAF: _XOR("A", "A", 1);
            case 0xA8: _XOR("A", "B", 1);
            case 0xA9: _XOR("A", "C", 1);
            case 0xAA: _XOR("A", "D", 1);
            case 0xAB: _XOR("A", "E", 1);
            case 0xAC: _XOR("A", "H", 1);
            case 0xAD: _XOR("A", "L", 1);
            case 0xAE: _XOR("A", "(HL)", 1);
            case 0xEE: _XOR("A", stringify(bc(addr[1])), 2);
            case 0xBF: _CP("A", "A", 1);
            case 0xB8: _CP("A", "B", 1);
            case 0xB9: _CP("A", "C", 1);
            case 0xBA: _CP("A", "D", 1);
            case 0xBB: _CP("A", "E", 1);
            case 0xBC: _CP("A", "H", 1);
            case 0xBD: _CP("A", "L", 1);
            case 0xBE: _CP("A", "(HL)", 1);
            case 0xFE: _CP("A", stringify(bc(addr[1])), 2);
            case 0x3C: _INC("A");
            case 0x04: _INC("B");
            case 0x0C: _INC("C");
            case 0x14: _INC("D");
            case 0x1C: _INC("E");
            case 0x24: _INC("H");
            case 0x2C: _INC("L");
            case 0x34: _INC("(HL)");
            case 0x3D: _DEC("A");
            case 0x05: _DEC("B");
            case 0x0D: _DEC("C");
            case 0x15: _DEC("D");
            case 0x1D: _DEC("E");
            case 0x25: _DEC("H");
            case 0x2D: _DEC("L");
            case 0x35: _DEC("(HL)");
            case 0x09: _ADD("HL", "BC", 1);
            case 0x19: _ADD("HL", "DE", 1);
            case 0x29: _ADD("HL", "HL", 1);
            case 0x39: _ADD("HL", "SP", 1);
            case 0xE8: _ADD("SP", stringify(bc(addr[1])), 2); // reverifier si ADD ou ADDMEM ?
            case 0x03: _INC("BC");
            case 0x13: _INC("DE");
            case 0x23: _INC("HL");
            case 0x33: _INC("SP");
            case 0x0B: _DEC("BC");
            case 0x1B: _DEC("DE");
            case 0x2B: _DEC("HL");
            case 0x3B: _DEC("SP");
            case 0xCB: decodeBitInstruction_(addr + 1); break;
            case 0x27: _DAA();
            case 0x2F: _CPL();
            case 0x3F: _CCF();
            case 0x37: _SCF();
            case 0x00: case 0xE3: case 0xE4:
            case 0xF4: case 0xFC: case 0xFD:
            case 0xEB: case 0xDD: case 0xEC:
            case 0xED: case 0xDB: case 0xD3:
                _NOP();
            case 0x76: _HALT();
            case 0x10: _STOP();
            case 0xF3: _DI_();
            case 0xFB: _EI();
            case 0x07: _RLCA();
            case 0x17: _RLA();
            case 0x0F: _RRCA();
            case 0x1F: _RRA();
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
            case 0xC7: _RST(stringify(0x0));
            case 0xCF: _RST(stringify(0x8));
            case 0xD7: _RST(stringify(0x10));
            case 0xDF: _RST(stringify(0x18));
            case 0xE7: _RST(stringify(0x20));
            case 0xEF: _RST(stringify(0x28));
            case 0xF7: _RST(stringify(0x30));
            case 0xFF: _RST(stringify(0x38));
            case 0xC9: _RET();
            case 0xC0: _RETNZ();
            case 0xC8: _RETZ();
            case 0xD0: _RETNC();
            case 0xD8: _RETC();
            case 0xD9: _RETI();

            }
        }
    }
}
