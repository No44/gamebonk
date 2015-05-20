#ifndef GBONK_DEBUG_MEMORYDUMPER_HPP_
#define GBONK_DEBUG_MEMORYDUMPER_HPP_

#include <cstdint>
#include <string>

namespace GBonk
{

    namespace Debug
    {

        class MemoryDumper
        {
        public:
            MemoryDumper();

            unsigned int alignment;

            void setTrackedMemory(uint8_t* mem, unsigned int size);

            std::string dump() const;

        private:
            uint8_t*    mem_;
            unsigned int memsize_;
            
            
        };

    }


}

#endif
