cmake_minimum_required(VERSION 3.10)

#============================================
project(Alarm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(alarm_window WIN32 test_alarm2.cpp)
target_link_libraries(alarm_window PRIVATE user32 gdi32)

#============================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(udp_alarm_listener WIN32 client.cpp logger.cpp)
target_include_directories(udp_alarm_listener PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(udp_alarm_listener PRIVATE ws2_32)

add_custom_command(TARGET udp_alarm_listener POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/shutdown_udp_listener.ps1
        $<TARGET_FILE_DIR:udp_alarm_listener>/shutdown_udp_listener.ps1
)

#============================================
# TODO: for this to work properly with default install location, 
# we need to change log file path to appdata otherwise the log file will be created in the install location which is not writable by default
install(TARGETS udp_alarm_listener alarm_window
        RUNTIME DESTINATION bin)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/shutdown_udp_listener.ps1 DESTINATION bin)