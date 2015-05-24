#ifndef _GBONK_ROMREADER_HPP_
#define _GBONK_ROMREADER_HPP_

#include <cstdint>

namespace GBonk
{

    class CPU;

    class ROMReader
    {
    public:
        ROMReader(CPU&, unsigned int memloc);
        ROMReader(const ROMReader&);

        unsigned int addr() const { return memloc_; }
        uint8_t operator*() const;
        uint8_t operator[](int idx) const;
        ROMReader operator+(int v) const;
        ROMReader operator-(int v) const;

    private:
        CPU& cpu_;
        unsigned int memloc_;
    };


}

#endif
