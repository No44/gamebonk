#ifndef DBONK_OBJECT_HPP_
#define DBONK_OBJECT_HPP_

#include "Assembly.hpp"

namespace GBonk{
    class Cartridge;
}

namespace DBonk
{

    class Object
    {
    public:

        void load(const GBonk::Cartridge&);

        const Assembly& assembly() const { return ass_; }

    private:
        Assembly ass_;
    };

}


#endif
