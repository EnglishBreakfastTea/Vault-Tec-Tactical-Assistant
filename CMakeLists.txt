cmake_minimum_required(VERSION 3.6)
project(VaultTecTacticalAssistant)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -masm=intel -static-libgcc -static-libstdc++")

set(DLL_SOURCE_FILES dllmain.cpp mainLoop.cpp mainLoop.h FOClient.h messageQueue.h State.h State.cpp ComplexAction.h
        HexAttack.cpp HexAttack.h FOClient.cpp LeftMouseHook.h LeftMouseHook.cpp InterfaceOverlay.cpp
        InterfaceOverlay.h Hotkeys.cpp Hotkeys.h)
set(UI_SOURCE_FILES uiMain.cpp messageQueue.h)
set(INJECTOR_SOURCE_FILES main.cpp)

set(BOOST_ROOT "C:/boost_1_61_0")
find_package(Boost 1.61.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(VaultTecTacticalAssistant SHARED ${DLL_SOURCE_FILES})
add_executable(Injector ${INJECTOR_SOURCE_FILES})
add_executable(VaultTecTacticalAssistantUI ${UI_SOURCE_FILES})
