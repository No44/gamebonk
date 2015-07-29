#include "video/Driver.hpp"
#include "Cartridge.hpp"
#include "CPU.hpp"
#include "debug/DebuggerHost.hpp"
#include "debug/Debugger.hpp"

void engine()
{
    GBonk::CPU cpu;
    GBonk::Cartridge c;

    c.openFile("games/tennis.gb");
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

void start()
{
    bool b = GBonk::Video::Driver::Init();

#ifdef GBONK_DEBUG
    engine();
    GBonk::Video::Driver::Shutdown();
    return;
#endif


    GBonk::CPU cpu;
    GBonk::Cartridge c;
    GBonk::Video::Driver d;

    c.openFile("games/tetris.gb");
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

int main(int, char**)
{
    start();
    return EXIT_SUCCESS;
}
