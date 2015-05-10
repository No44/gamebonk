#include <iostream>
#include <fstream>

#include "Assembly.hpp"
#include "Cartridge.hpp"
#include "Object.hpp"

int main(int ac, char **av)
{
    std::cerr << av[0] << " " << av[1] << std::endl;

    std::string input = av[1];
    std::string output = input.substr(0, input.rfind('.')) + ".s";

    GBonk::Cartridge c;
    DBonk::Object o;

    c.openFile(input);
    o.load(c);

    const DBonk::Assembly& ass = o.assembly();

    std::string txt = ass.toString();

    std::ofstream out(output, std::ofstream::out | std::ofstream::trunc);
    out << txt;
    out.close();

    return EXIT_SUCCESS;
}
