#ifndef GBONK_CARTRIDGE_HPP
#define GBONK_CARTRIDGE_HPP

#include <cstdint>
#include <string>

namespace GBonk
{

    class Cartridge
    {
    public:

        enum Language
        {
            Lang_Japanese = 0,
            Lang_English = 1,
        };
        enum Manufacturer
        {
            Man_Nintendo = 0x3301,
            Man_Accolade = 0x7901,
            Man_Konami = 0xA400
        };
        Cartridge();
        ~Cartridge();
        
        void openFile(const std::string& game);
        void loadContent(const uint8_t*, uint32_t size);
        void close();

        void displayInfo() const;
        uint32_t RAMSize() const { return ramSize_; }
        uint32_t ROMSize() const { return romSize_; }
        uint32_t startAddress() const { return startAddress_; }
        const uint8_t* ROM() const { return content_; }

        uint16_t checksum();

        static std::string LangToString(Language);
        static std::string ManufacturerToString(Manufacturer);

    private:
        friend class CPU;
        void load_();

        uint32_t startAddress_;
        std::string name_;
        Language language_;
        Manufacturer manufacturer_;
        uint32_t type_;
        uint32_t version_;
        uint32_t romSize_;
        uint32_t ramSize_;
        uint8_t *content_;
    };

}


#endif
