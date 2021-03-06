#include "video/TileTable.hpp"

namespace GBonk
{

    namespace Video
    {

        int TileTable::tileId(int x, int y) const
        {
            uint32_t offset = y * Cols + x;

            return addr_[offset];
        }

    }

}
