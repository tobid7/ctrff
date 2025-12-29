cmake_minimum_required(VERSION 3.22)

project(ctrff)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

option(CTRFF_DESKTOP OFF "Buid for Desktop Platform")
option(CTRFF_3DS OFF "Build lib for 3ds")

if(${CTRFF_3DS})
    set(CTRFF_DESKTOP OFF CACHE BOOL)
    set(CTRFF_BUILD_GUI OFF CACHE BOOL)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(DEFINED ENV{DEVKITPRO})
            set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake" CACHE PATH "toolchain file")
        else()
            message(FATAL_ERROR "Please define DEVKITPRO to point to your SDK path!")
        endif()
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-psabi -O3")
endif()

add_library(ctrff STATIC
    source/helper.cpp
    source/lz11.cpp
    source/smdh.cpp
    source/binutil.cpp
    source/bcstm.cpp
    source/bcwav.cpp
    source/3dsx.cpp
)
target_include_directories(ctrff PUBLIC include)

if(${CTRFF_DESKTOP})
    add_subdirectory(vendor/palladium)
    add_executable(ctrff-cli tool/main.cpp)
    target_include_directories(ctrff-cli PUBLIC include vendor/stb vendor/cli-fancy/include)
    target_link_libraries(ctrff-cli PUBLIC ctrff palladium)
    add_executable(test tool/test.cpp)
    target_include_directories(test PUBLIC include vendor/stb vendor/cli-fancy/include)
    target_link_libraries(test PUBLIC ctrff)
endif()

install(TARGETS ctrff)
install(DIRECTORY include DESTINATION ".")
