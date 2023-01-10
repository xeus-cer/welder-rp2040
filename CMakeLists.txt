cmake_minimum_required(VERSION 3.22)

include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(picotest C CXX ASM)
# set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()
	
include_directories(
	"include"
)

# Add -O0 to remove optimizations when using gcc
IF(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)



add_compile_definitions(TYPE_PRESSURE)

add_executable(picotest
	src/main.cpp
)

target_link_libraries(
	picotest 
	pico_stdlib 
	pico_multicore
	hardware_rtc
	hardware_gpio
	hardware_adc
	hardware_uart
	hardware_pwm
	hardware_sleep
	hardware_flash
	hardware_sync
	hardware_spi
	)

	
pico_enable_stdio_usb(picotest 1)
pico_enable_stdio_uart(picotest 0)
# pico_enable_stdio_uart(picotest 1)

pico_add_extra_outputs(picotest)

# copy .UF2 binary to the RP2040
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.uf2
	DESTINATION /media/$ENV{USER}/RPI-RP2/${PROJECT_NAME}.uf2
)