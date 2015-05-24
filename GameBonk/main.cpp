#include "video/Driver.hpp"
#include "Cartridge.hpp"
#include "CPU.hpp"
#include "debug/DebuggerHost.hpp"
#include "Debug/Debugger.hpp"

void engine()
{
    GBonk::CPU cpu;
    GBonk::Cartridge c;

    c.openFile("games/tetris.gb");
    c.displayInfo();

    cpu.load(c);


    GBonk::Debug::DebuggerHost host;
    GBonk::Debug::Debugger debug(host, cpu);

    host.debugger(debug);
    host.open();

    while (1)
        host.pumpEvents();

    c.close();
}

void start()
{
    engine();
    return;

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
