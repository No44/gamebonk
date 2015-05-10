#ifndef GBONK_VIDEO_DRIVER_HPP
#define GBONK_VIDEO_DRIVER_HPP

#include <memory>

#include "video/Sprite.hpp"

namespace GBonk
{

    namespace Video
    {

        class VideoSystem;
        class Driver_p;

        class Driver
        {
        public:

            bool pumpEvents();
            void openWindow();
            void render();
            void draw(const Sprite&);

            static bool Init();
            static void Shutdown();

        private:
            static Driver_p* p_;
        };

    }
    
}


#endif
