#include "ROMReader.hpp"
#include "CPU.hpp"

namespace GBonk
{

    ROMReader::ROMReader(CPU& c, unsigned int memloc)
        : cpu_(c),
        memloc_(memloc)
    {
    }

    ROMReader::ROMReader(const ROMReader& r)
        : cpu_(r.cpu_),
        memloc_(r.memloc_)
    {
    }

    uint8_t ROMReader::operator*() const
    {
        return cpu_.read(memloc_);
    }

    uint8_t ROMReader::operator[](int idx) const
    {
        return cpu_.read(memloc_ + idx);
    }

    ROMReader ROMReader::operator+(int v) const
    {
        ROMReader r(*this);

        r.memloc_ += v;
        return r;
    }

    ROMReader ROMReader::operator-(int v) const
    {
        return operator+(-v);
    }
}
