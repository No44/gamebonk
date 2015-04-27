#include "video/TileTable.hpp"

namespace GBonk
{

    namespace Video
    {

        unsigned int TileTable::tileId(int x, int y)
        {
            uint32_t offset = y * Cols + x;

            return addr_[offset];
        }

    }


}