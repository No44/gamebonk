#ifndef GBONK_VIDEO_DRIVER_HPP
#define GBONK_VIDEO_DRIVER_HPP

#include "video/Sprite.hpp"

namespace GBonk
{

    namespace Video
    {

        class VideoSystem;

        class Driver
        {
        public:

            void draw(const Sprite&);

        private:
        };

    }
    
}


#endif
