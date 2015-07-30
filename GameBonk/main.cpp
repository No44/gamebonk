#include "video/Driver.hpp"
#include "Cartridge.hpp"
#include "CPU.hpp"
#include "debug/DebuggerHost.hpp"
#include "debug/Debugger.hpp"

void engine(const char* game)
{
    GBonk::CPU cpu;
    GBonk::Cartridge c;

    c.openFile(game);
    c.displayInfo();

    cpu.load(c);
    cpu.prepareLaunch();

    GBonk::Debug::DebuggerHost host;
    GBonk::Debug::Debugger debug(host, cpu);

    host.open();

    debug.brk();

    debug.run();


    c.close();
}

void start(const char* game = nullptr)
{
    bool b = GBonk::Video::Driver::Init();
    const char* cartridge = game ? game : "games/tetris.gb";

#ifdef GBONK_DEBUG
    engine();
    GBonk::Video::Driver::Shutdown();
    return;
#endif


    GBonk::CPU cpu;
    GBonk::Cartridge c;
    GBonk::Video::Driver d;

    c.openFile(cartridge);
    c.displayInfo();

    cpu.load(c);
    d.openWindow();
    cpu.run();

    c.close();

    GBonk::Video::Driver::Shutdown();
}

#if defined WIN32 || defined WIN64

#include <Windows.h>

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    start();
    return EXIT_SUCCESS;
}

#endif

int main(int, char** av)
{
    start(av[1]);
    return EXIT_SUCCESS;
}
