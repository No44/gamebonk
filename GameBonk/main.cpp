#include "video/Driver.hpp"
#include "Cartridge.hpp"
#include "CPU.hpp"

void start()
{
    bool b = GBonk::Video::Driver::Init();

    GBonk::CPU cpu;
    GBonk::Cartridge c;

    c.openFile("games/tetris.gb");
    c.displayInfo();

    cpu.load(c);
    cpu.run();

    c.close();

    GBonk::Video::Driver::Shutdown();
}


int WinMain(int, char **)
{
    start();
    return EXIT_SUCCESS;
}

int main(int, char**)
{
    start();
    return EXIT_SUCCESS;
}
