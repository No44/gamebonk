#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>

#include "AMemoryBankController.hpp"
#include "MemoryBankController1.hpp"
#include "Cartridge.hpp"

namespace GBonk
{

    AMemoryBankController::AMemoryBankController(const Cartridge& c)
        : type_(c.MBCType()),
        cartridgeROM_(c.ROM()),
        externalRAM_(new uint8_t[c.RAMSize()])
    {
        std::memset(externalRAM_, 0, c.RAMSize());
    }

    AMemoryBankController::~AMemoryBankController()
    {
        delete[] externalRAM_;
        externalRAM_ = nullptr;
        cartridgeROM_ = nullptr;
    }

    std::string AMBC::typeToString(Type t)
    {
        return std::to_string(uint32_t(t));
    }

    AMBC* AMBC::makeMBC(const Cartridge& c)
    {
        // todo: autres MBC
      switch (c.MBCType())
      {
      case AMBC::Type_ROM_ONLY:
      case AMBC::Type_ROM_MBC1:
      case AMBC::Type_ROM_MBC1_RAM:
      case AMBC::Type_ROM_MBC1_RAM_BATT:
          return new MBC1(c);
      default:
        throw std::runtime_error("Not implemented");
      }
        return nullptr;
    }

}
