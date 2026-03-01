cmake_minimum_required(VERSION 3.22)

project(ctrff VERSION 0.2.0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

option(CTRFF_DESKTOP OFF "Buid for Desktop Platform")
option(CTRFF_3DS OFF "Build lib for 3ds")
option(CTRFF_SHARED OFF)

if(CTRFF_3DS)
    set(CTRFF_DESKTOP OFF CACHE BOOL)
    set(CTRFF_BUILD_GUI OFF CACHE BOOL)
    set(CTRFF_SHARED OFF CACHE BOOL)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(DEFINED ENV{DEVKITPRO})
            set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake" CACHE PATH "toolchain file")
        else()
            message(FATAL_ERROR "Please define DEVKITPRO to point to your SDK path!")
        endif()
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-psabi -O3")
endif()

set(CTRFF_SOURCE
    source/3dsx.cpp
    source/bclim.cpp
    source/bcstm.cpp
    source/bcwav.cpp
    source/binutil.cpp
    source/helper.cpp
    source/lz11.cpp
    source/pica.cpp
    source/smdh.cpp
)

if(CTRFF_SHARED)
    add_library(ctrff SHARED ${CTRFF_SOURCE})
    target_compile_definitions(ctrff PUBLIC CTRFF_SHARED)
    target_compile_definitions(ctrff PRIVATE CTRFF_BUILD_SHARED)
else()
    add_library(ctrff STATIC ${CTRFF_SOURCE})
endif()

target_include_directories(ctrff
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
add_library(ctrff::ctrff ALIAS ctrff)

if(CTRFF_DESKTOP)
    add_subdirectory(tools)
endif()

install(TARGETS ctrff
    EXPORT ctrffTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT ctrffTargets
    FILE ctrffTargets.cmake
    NAMESPACE ctrff::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctrff
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/ctrffConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ctrffConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctrff
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ctrffConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ctrffConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ctrffConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctrff
)
