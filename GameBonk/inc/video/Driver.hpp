#ifndef GBONK_VIDEO_DRIVER_HPP
#define GBONK_VIDEO_DRIVER_HPP

#include <memory>
#include <functional>

#include "video/Sprite.hpp"

namespace GBonk
{

    enum class GBKeys : unsigned int;

    namespace Video
    {

        class VideoSystem;
        class Driver_p;

        class Driver
        {
        public:

            typedef std::function<void(GBKeys)> InputCb;

            bool pumpEvents();
            void openWindow();
            void render();
            void draw(const Sprite&);
            void setWindowTitle(const std::string& title);

            static bool Init();
            static void Shutdown();

            static void SetOnKeyDownCb(InputCb cb);
            static void SetOnKeyUpCb(InputCb cb);

        private:
            static Driver_p* p_;
        };

        

    }
    
}


#endif
