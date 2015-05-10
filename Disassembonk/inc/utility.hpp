#ifndef DBONK_UTILITY_HPP
#define DBONK_UTILITY_HPP

#include <string>
#include <sstream>
#include <iomanip>

namespace DBonk
{

    inline std::string stringify(unsigned int v)
    {
        std::stringstream builder;

        builder << "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << v;
        return builder.str();
    }

    inline std::string stringifyw(unsigned int v)
    {
        std::stringstream builder;

        builder << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << v;
        return builder.str();
    }

}

#endif
