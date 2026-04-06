cmake_minimum_required(VERSION 3.20)
project(lol_proximity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(LibDataChannel CONFIG REQUIRED)

add_executable(lol_proximity
    main.cpp
    minimap_reader.cpp
    AudioManager.cpp
)

target_link_libraries(lol_proximity PRIVATE
    Ws2_32
    Ole32
    ${OpenCV_LIBS}
    LibDataChannel::LibDataChannel
)

target_include_directories(lol_proximity PRIVATE ${OpenCV_INCLUDE_DIRS})

target_compile_definitions(lol_proximity PRIVATE 
    WIN32_LEAN_AND_MEAN 
    NOMINMAX 
)
