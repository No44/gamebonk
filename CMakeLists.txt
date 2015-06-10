cmake_minimum_required(VERSION 2.8)

project(GameBonk)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include_directories(GameBonk/inc/)

add_executable(gamebonk
  ./GameBonk/inc/MemoryBankController1.hpp
  ./GameBonk/inc/video/Driver.hpp
  ./GameBonk/inc/video/TilePatternTable.hpp
  ./GameBonk/inc/video/VideoSystem.hpp
  ./GameBonk/inc/video/Sprite.hpp
  ./GameBonk/inc/video/TileTable.hpp
  ./GameBonk/inc/debug/MemoryDumper.hpp
  ./GameBonk/inc/debug/Instruction.hpp
  ./GameBonk/inc/debug/DebuggerHost.hpp
  ./GameBonk/inc/debug/Debugger.hpp
  ./GameBonk/inc/MMU.hpp
  ./GameBonk/inc/AMemoryBankController.hpp
  ./GameBonk/inc/CPU.hpp
  ./GameBonk/inc/ClockScheduler.hpp
  ./GameBonk/inc/ROMReader.hpp
  ./GameBonk/inc/Cartridge.hpp
  ./GameBonk/src/CPU.cpp
  ./GameBonk/src/MemoryBankController1.cpp
  ./GameBonk/src/MMU.cpp
  ./GameBonk/src/Cartridge.cpp
  ./GameBonk/src/video/VideoSystem.cpp
  ./GameBonk/src/video/TilePatternTable.cpp
  ./GameBonk/src/video/TileTable.cpp
  ./GameBonk/src/video/Driver.cpp
  ./GameBonk/src/video/Sprite.cpp
  ./GameBonk/src/debug/Instruction.cpp
  ./GameBonk/src/debug/MemoryDumper.cpp
  ./GameBonk/src/debug/Debugger.cpp
  ./GameBonk/src/debug/DebuggerHost.cpp
  ./GameBonk/src/ROMReader.cpp
  ./GameBonk/src/ClockScheduler.cpp
  ./GameBonk/src/AMemoryBankController.cpp
  ./GameBonk/main.cpp)

find_package(SDL2)
find_package(FLTK)

set(GCC_COVERAGE_COMPILE_FLAGS "-std=gnu++11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

set(CMAKE_BUILD_TYPE "Debug")

include_directories(${SDL2_INCLUDE_DIRS} ${FLTK_INCLUDE_DIRS})
target_link_libraries(gamebonk ${FLTK_LIBRARIES} dl ${SDL2_LIBRARY})
