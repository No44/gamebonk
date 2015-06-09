#include <cstring>
#include <exception>
#include <string>
#include <iostream>


#include <SDL2/SDL.h>

#include "video/VideoSystem.hpp"
#include "video/Driver.hpp"

namespace GBonk
{

    namespace Video
    {

        class Driver_p
        {
        public:
            Driver_p();
            bool pumpEvents();
            void openWindow();
            void render();
            void draw(const Sprite&);

            friend bool Driver::Init();
            friend void Driver::Shutdown();

        private:
            bool init();
            void shutdown();

            SDL_Surface* screenSurface_;
            SDL_Texture* screenTex_;
            SDL_Window* win_;
            SDL_Renderer* rend_;
        };

        Driver_p* Driver::p_ = new Driver_p;
        
        void Driver::openWindow()
        {
            p_->openWindow();
        }

        void Driver::draw(const Sprite& s)
        {
            p_->draw(s);
        }

        void Driver::render()
        {
            p_->render();
        }

        bool Driver::pumpEvents()
        {
            return p_->pumpEvents();
        }

        bool Driver::Init()
        {
            Driver::p_ = new Driver_p;
            if (!p_->init())
            {
                Shutdown();
                return false;
            }
            return true;
        }

        void Driver::Shutdown()
        {
            Driver::p_->shutdown();
            delete Driver::p_;
        }




        Driver_p::Driver_p()
            : screenSurface_(nullptr),
            screenTex_(nullptr),
            win_(nullptr),
            rend_(nullptr)
        {
        }


        bool Driver_p::init()
        {
            if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            {
                std::cerr << "SDL_Init: failed" << std::endl;
                return false;
            }
            screenSurface_ = SDL_CreateRGBSurface(0, VideoSystem::ScreenWidth, VideoSystem::ScreenHeight, 32, 0, 0, 0, 0);
            /*
            for (int i = 0; i < screenSurface_->h; ++i)
            {
                int line = i * screenSurface_->w;
                for (int j = 0; j < screenSurface_->w; ++j)
                    ((uint32_t*)screenSurface_->pixels)[i + j] = 0xFFFFFFFF;
            }
            */
            std::memset(screenSurface_->pixels, 0xFF, screenSurface_->h * screenSurface_->w * 4);

            return true;
        }

        void Driver_p::shutdown()
        {
            SDL_FreeSurface(screenSurface_);
            screenSurface_ = nullptr;
            SDL_DestroyTexture(screenTex_);
            screenTex_ = nullptr;
            SDL_DestroyRenderer(rend_);
            rend_ = nullptr;
            SDL_DestroyWindow(win_);
            win_ = nullptr;
            SDL_Quit();
        }

        void Driver_p::openWindow()
        {
            win_ = SDL_CreateWindow("GameBonk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VideoSystem::ScreenWidth, VideoSystem::ScreenHeight, SDL_WINDOW_SHOWN);
            if (!win_)
                throw std::runtime_error("SDL_CreateWindow: error");
            rend_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED);
            if (!rend_)
                throw std::runtime_error("SDL_CreateRenderer: error");
            screenTex_ = SDL_CreateTextureFromSurface(rend_, screenSurface_);
        }

        void Driver_p::render()
        {
            SDL_RenderClear(rend_);
            SDL_UpdateTexture(screenTex_, nullptr, screenSurface_->pixels, screenSurface_->pitch);
            SDL_RenderCopy(rend_, screenTex_, nullptr, nullptr);
            SDL_RenderPresent(rend_);
        }

        bool Driver_p::pumpEvents()
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT
                    || e.type == SDL_KEYDOWN)
                    return true;
            }
            return false;
        }

        void Driver_p::draw(const Sprite& s)
        {
            uint32_t* pixels = (uint32_t*)screenSurface_->pixels;

            for (int j = 0; j < s.height(); ++j)
            {
                for (int i = 0; i < s.width(); ++i)
                {
                    int x = s.x + i;
                    int y = s.y + j;
                    pixels[y * VideoSystem::ScreenWidth + x] = s.at(i, j);
                }
            }
        }
    }


}
