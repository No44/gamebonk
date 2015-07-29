#ifndef GBONK_UTILITY_HPP
#define GBONK_UTILITY_HPP

#include <utility>
#include <algorithm>

namespace GBonk
{

    namespace Utility
    {

        template<typename T>
        inline T min(T arg)
        {
            return arg;
        }

        template<typename T, typename... Args>
        inline T min(T first, Args... args)
        {
            return std::min(first, min(args...));
        }


    }

}

#endif
