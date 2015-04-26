#include "Cartridge.hpp"

int main(int, char**)
{
    GBonk::Cartridge c;

    c.openFile("games/tetris.gb");
    c.displayInfo();
    c.close();

    return EXIT_SUCCESS;
}