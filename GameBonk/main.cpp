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
    cpu.prepareLaunch();

    GBonk::Debug::DebuggerHost host;
    GBonk::Debug::Debugger debug(host, cpu);

    host.open();

    debug.brk();
    //while (1)
    //{
        debug.run();
        // todo:mettre ca V dans dbg::run
    //    host.pumpEvents();
    //    Sleep(33);
    //}

    c.close();
}

void start()
{
    bool b = GBonk::Video::Driver::Init();

    engine();
    GBonk::Video::Driver::Shutdown();
    return;



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
