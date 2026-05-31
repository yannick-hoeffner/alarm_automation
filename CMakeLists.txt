cmake_minimum_required(VERSION 3.10)
project(Alarm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(alarm WIN32 test_alarm2.cpp)
target_link_libraries(alarm PRIVATE user32 gdi32)