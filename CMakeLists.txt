cmake_minimum_required(VERSION 2.8)

project(GameBonk)

add_executable(GameBonk GameBonk/inc/MemoryBankController1.hpp
  GameBonk/inc/video/Driver.hpp
  GameBonk/inc/video/TilePatternTable.hpp
  GameBonk/inc/video/VideoSystem.hpp
  GameBonk/inc/video/TileTable.hpp
  GameBonk/inc/MMU.hpp
  GameBonk/inc/AMemoryBankController.hpp
  GameBonk/inc/CPU.hpp
  GameBonk/inc/Cartridge.hpp
  GameBonk/games/pokemon_red.gb
  GameBonk/games/tetris.gb
  GameBonk/src/CPU.cpp
  GameBonk/src/MemoryBankController1.cpp
  GameBonk/src/MMU.cpp
  GameBonk/src/Cartridge.cpp
  GameBonk/src/VideoSystem.cpp
  GameBonk/src/AMemoryBankController.cpp
  GameBonk/main.cpp)
