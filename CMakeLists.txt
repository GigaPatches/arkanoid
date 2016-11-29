cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(arkanoid VERSION 0.1 LANGUAGES CXX)

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED on)

find_package(SDL2)

add_executable(arkanoid src/main.cpp)
target_include_directories(arkanoid PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(arkanoid SDL2::SDL2main)

include(InstallRequiredSystemLibraries)
install(TARGETS arkanoid DESTINATION arkanoid)
if(SDL2_DYNAMIC)
    install(FILES ${SDL2_RUNTIME_LIBRARY} DESTINATION arkanoid)
endif()