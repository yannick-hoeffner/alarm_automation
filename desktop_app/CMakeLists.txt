cmake_minimum_required(VERSION 3.10)

#============================================
project(Alarm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(alarm WIN32 test_alarm2.cpp)
target_link_libraries(alarm PRIVATE user32 gdi32)

#============================================
project(udp_client LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(udp_listener client.cpp)
target_link_libraries(udp_listener PRIVATE ws2_32)