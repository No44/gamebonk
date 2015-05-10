#ifndef DBONK_INSTRUCTION_HPP_
#define DBONK_INSTRUCTION_HPP_

#include <vector>
#include <string>

#include <cstdint>

namespace DBonk
{

    class Instruction
    {
    public:
        Instruction(const uint8_t* addr, unsigned int addr_offset);
        Instruction(Instruction&&);
        Instruction(const Instruction&);

        const std::string& mnemonic() const { return mnemo_; }
        const std::vector<unsigned int>& bytecode() const { return bytecode_; }
        unsigned int addr() const { return addr_; }
        unsigned int len() const { return len_; }
        const std::vector<std::string>& parameters() const { return params_; }

        std::string toString() const;

    private:
        void decodeBitInstruction_(const uint8_t* addr);
        void decodeGbInstruction_(const uint8_t* addr);
        void decodeInstruction_(const uint8_t* addr);
        unsigned int bc(unsigned int v) { bytecode_.push_back(v); return v; }
        unsigned int bc2(unsigned int v) { bytecode_.push_back(v & 0xFF); bytecode_.push_back((v>>8) & 0xFF);  return v; }

        std::string mnemo_;
        std::vector<unsigned int> bytecode_;
        unsigned int addr_;
        unsigned int len_;
        std::vector<std::string> params_;
    };

}

#endif
