#ifndef GBONK_VIDEO_TILEPATTERNTABLE_HPP
#define GBONK_VIDEO_TILEPATTERNTABLE_HPP

#include <cstdint>

namespace GBonk
{

    namespace Video
    {

        class TilePatternTable
        {
        public:

            void setAddr(const uint8_t* a) { addr_ = a; }

        private:
            const uint8_t* addr_;
        };


    }

}

#endif