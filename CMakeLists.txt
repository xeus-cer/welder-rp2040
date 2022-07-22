cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(picotest C CXX ASM)
# set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

include_directories("lib")

add_executable(picotest
	src/main.cpp
	)


add_subdirectory(lib/xerxes-protocol-cpp)

target_link_libraries(picotest pico_stdlib pico_multicore hardware_gpio hardware_adc hardware_uart hardware_pwm xerxes)

pico_enable_stdio_usb(picotest 1)
pico_enable_stdio_uart(picotest 1)

pico_add_extra_outputs(picotest)