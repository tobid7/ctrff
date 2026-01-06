cmake_minimum_required(VERSION 3.22)

project(ctrff-cli)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(ctrff-cli source/main.cpp)
target_include_directories(ctrff-cli PUBLIC include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/cli-fancy/include
)
target_link_libraries(ctrff-cli PUBLIC ctrff palladium)

add_executable(test source/test.cpp)
target_include_directories(test PUBLIC include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/cli-fancy/include
)
target_link_libraries(test PUBLIC ctrff)

install(TARGETS ctrff-cli)
