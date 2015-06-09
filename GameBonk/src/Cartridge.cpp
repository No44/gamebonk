#if defined _WIN32 || defined _WIN64
#define NOMINMAX
#include <Windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#endif


#include <iostream>
#include <exception>
#include <cassert>
#include <algorithm>

#include "AMemoryBankController.hpp"
#include "MMU.hpp"
#include "Cartridge.hpp"

namespace GBonk
{

    Cartridge::Cartridge()
        : content_(nullptr)
    {
    }

    Cartridge::~Cartridge()
    {
        close();
    }

#if defined _WIN32 || defined _WIN64
    void Cartridge::openFile(const std::string& file)
    {
        HANDLE fileHandle =
            CreateFile(file.c_str(), GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (fileHandle == INVALID_HANDLE_VALUE)
            throw std::runtime_error(file);

        unsigned int contentSize = GetFileSize(fileHandle, nullptr);
        content_ = new uint8_t[contentSize];
        std::memset(content_, 0, contentSize);
        DWORD bytesRead = 0;
        ReadFile(fileHandle, content_, contentSize, &bytesRead, nullptr);
        CloseHandle(fileHandle);

        load_();
    }
#else
    void Cartridge::openFile(const std::string &game)
    {
      int fd = open(game.c_str(), O_RDONLY);
      if (fd == -1)
        throw std::runtime_error(strerror(errno));

      struct stat buf;
      int st = fstat(fd, &buf);
      if (st == -1)
      {
        ::close(fd);
        throw std::runtime_error(strerror(errno));
      }

      unsigned int contentSize = buf.st_size;
      content_ = new uint8_t[contentSize];
      std::memset(content_, 0, contentSize);
      ssize_t bytesRead = ::read(fd, content_, contentSize);
      ::close(fd);

      load_();
    }

#endif

    void Cartridge::loadContent(const uint8_t* c, uint32_t s)
    {
        close();
        content_ = new uint8_t[s];
        std::memcpy(content_, c, s);
        load_();
    }

    void Cartridge::close()
    {
        delete[] content_;
        content_ = nullptr;
    }

    static const uint8_t nintendo_art[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73,
        0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F,
        0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD,
        0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    static const uint32_t ROM_sizes[] = {
        32 * (1 << 10),
        64 * (1 << 10),
        128 * (1 << 10),
        256 * (1 << 10),
        512 * (1 << 10)
    };
    static const uint32_t RAM_sizes[] = {
        0,
        2 * (1 << 10),
        8 * (1 << 10),
        32 * (1 << 10),
    };

    uint16_t Cartridge::checksum()
    {
        uint32_t total = 0;
        for (unsigned int i = 0; i < 0x14E; ++i)
            total += content_[i];
        for (unsigned int i = 0x14F + 1; i < romSize_; ++i)
            total += content_[i];
        return total & 0xFFFF;
    }

    void Cartridge::load_()
    {
        uint32_t startAddr =
            *(uint32_t*)(content_ + 0x100);
        startAddress_ = ((startAddr & 0xFF000000) >> 16)
            | ((startAddr & 0xFF0000) >> 16);

        const void* cart_art = content_ + 0x104;
        assert(std::memcmp(nintendo_art, cart_art, sizeof(nintendo_art)) == 0);

        name_ = (char*)(content_ + 0x134);
        type_ = content_[0x147];
        //romSize_ = ROM_sizes[content_[0x148]];
        romSize_ = (32 * (1 << 10)) << content_[0x148];
        ramSize_ = RAM_sizes[content_[0x149]];
        manufacturer_ = static_cast<Manufacturer>(content_[0x151] << 8 | content_[0x150]);
        version_ = content_[0x14C];

        uint16_t ch = checksum();
        uint16_t cartch = content_[0x14E] << 8 | content_[0x14F];
        assert(ch == cartch);
    }

    std::string Cartridge::LangToString(Language l)
    {
        switch (l)
        {
        case Lang_Japanese:
            return "Japanese";
        case Lang_English:
            return "English";
        default:
            break;
        };
        return "undefined";
    }
    std::string Cartridge::ManufacturerToString(Manufacturer m)
    {
        switch (m)
        {
        case Man_Nintendo:
            return "Nintendo";
        case Man_Accolade:
            return "Accolade";
        case Man_Konami:
            return "Konami";
        default:
            break;
        };
        return "undefined";
    }

    void Cartridge::displayInfo() const
    {
        assert(content_);
        std::cout <<
            "Game " << name_ << ":" << std::endl
            << "\tLanguage:\t" << LangToString(language_) << std::endl
            << "\tManufacturer:\t" << ManufacturerToString(manufacturer_) << std::endl
            << "\tVersion:\t" << version_ << std::endl
            << "\tType:\t" << AMBC::typeToString(AMBC::Type(type_)) << std::endl
            << "\tRom size:\t" << romSize_ << std::endl
            << "\tRam size:\t" << ramSize_ << std::endl;
    }
}
