#include <sstream>
#include <iomanip>

#include "debug/MemoryDumper.hpp"

namespace GBonk
{

    namespace Debug
    {

        MemoryDumper::MemoryDumper()
            : alignment(16),
            mem_(nullptr),
            memsize_(0)
        {
        }

        void MemoryDumper::setTrackedMemory(uint8_t* mem, unsigned int size)
        {
            mem_ = mem;
            memsize_ = size;
        }

        static inline char printable(int c)
        {
            return '.';
        }

        std::string MemoryDumper::dump() const
        {
            std::stringstream builder;
            std::string strrep;
            unsigned int idx = 0;

            strrep.reserve(alignment + 1);
            while (idx < memsize_)
            {
                unsigned int lidx = 0;
                builder << "[0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << idx << "] ";
                while (lidx < alignment && idx + lidx < memsize_)
                {
                    unsigned int i = idx + lidx;
                    builder << ' ' << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << mem_[i];
                    strrep.push_back(printable(mem_[i]));
                    lidx++;
                }
                builder << " |" << strrep << "|\n";
                strrep.clear();
                idx += alignment;
            }


            return builder.str();
        }

    }

}
