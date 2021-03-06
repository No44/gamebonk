#ifndef GBONK_VIDEO_TILETABLE_HPP
#define GBONK_VIDEO_TILETABLE_HPP

#include <cstdint>

namespace GBonk
{

    namespace Video
    {

        class TileTable
        {
        public:

            void setAddr(const uint8_t* a) { addr_ = a; }
            int tileId(int x, int y) const;

        private:
            const uint8_t* addr_;
            static const uint32_t Rows = 32;
            static const uint32_t Cols = 32;
        };

    }


}


#endif
