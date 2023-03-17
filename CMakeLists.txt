cmake_minimum_required(VERSION 3.22)

# this must be first to set the correct toolchain
include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
include(pico_extras_import.cmake)


# this is the board we are using, we need to set this to use the correct board header and bootloader
# the board file is: src/Hardware/Board/xerxes_rp2040.h
set(PICO_BOARD "xerxes_rp2040")
set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/Hardware/Board")


set(PROJECT_NAME sensor-rp2040)
project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_CXX_STANDARD 23)


# check if ${CLKDIV} is set and set it to 4 if not - necessary for some boards
if(NOT DEFINED CLKDIV)
	set(CLKDIV 4)
endif()
add_compile_definitions(__CLKDIV=${CLKDIV})

# check if ${SENSOR_TYPE} is set
if(NOT DEFINED SENSOR_TYPE)
	message(FATAL_ERROR "SENSOR_TYPE not set, use -DSENSOR_TYPE=...\n\
	Possible options for SENSOR_TYPE: SCL3300, SCL3300a, SCL3400, AI, 4DI4DO, ABP")
endif()
# this is the sensor type we are using, we need to set this to use the correct sensor header
if(${SENSOR_TYPE} STREQUAL "SCL3300")
	add_compile_definitions(__SHIELD_SCL3300)
	add_compile_definitions(__SENSOR_CLASS=SCL3300)
elseif(${SENSOR_TYPE} STREQUAL "SCL3300a")
	add_compile_definitions(__SHIELD_SCL3300)
	add_compile_definitions(__SENSOR_CLASS=SCL3300a)
elseif(${SENSOR_TYPE} STREQUAL "SCL3400")
	add_compile_definitions(__SHIELD_SCL3400)
	add_compile_definitions(__SENSOR_CLASS=SCL3400)
elseif(${SENSOR_TYPE} STREQUAL "AI")
	add_compile_definitions(__SHIELD_AI)
	add_compile_definitions(__SENSOR_CLASS=AnalogInput)
elseif(${SENSOR_TYPE} STREQUAL "AnalogInput")
	add_compile_definitions(__SHIELD_AI)
	add_compile_definitions(__SENSOR_CLASS=AnalogInput)
elseif(${SENSOR_TYPE} STREQUAL "4DI4DO")
	add_compile_definitions(__SHIELD_4DI4DO)
	add_compile_definitions(__SENSOR_CLASS=_4DI4DO)
elseif(${SENSOR_TYPE} STREQUAL "ABP")
	add_compile_definitions(__SHIELD_ABP)
	add_compile_definitions(__SENSOR_CLASS=ABP)
elseif(${SENSOR_TYPE} STREQUAL "hx711")
	add_compile_definitions(__SHIELD_HX711)
	add_compile_definitions(__SENSOR_CLASS=HX711)
else()
	message(FATAL_ERROR "SENSOR_TYPE '" ${SENSOR_TYPE} "' is incorrect, use -DSENSOR_TYPE=...\n\
	Possible options for SENSOR_TYPE: SCL3300, SCL3300a, SCL3400, AI, 4DI4DO, ABP")
endif()
message(WARNING "\nSENSOR_TYPE set to ${SENSOR_TYPE}."
	"\nCLKDIV set to ${CLKDIV}, increase this "
	"value if device is not responding after reboot.")


pico_sdk_init()


 
# find xerxes-protocol library
set(xerxes-protocol_DIR "lib/xerxes-protocol-cpp")
find_package(xerxes-protocol REQUIRED)
add_library(xerxes-protocol STATIC ${xerxes-protocol_SOURCES})

	
include_directories(
	"src"
	${xerxes-protocol_DIR}
)

add_executable(
	${PROJECT_NAME}
	src/Communication/Callbacks.cpp
	src/Hardware/ClockUtils.cpp
	src/Hardware/Sleep.cpp
	src/Hardware/InitUtils.cpp
	src/Hardware/UserFlash.cpp
	src/Communication/RS485.cpp
	src/Core/Slave.cpp
	src/Core/Register.cpp
	src/Sensors/Peripheral.cpp
	src/Sensors/Sensor.cpp
	src/Sensors/Generic/AnalogInput.cpp
	src/Sensors/Generic/DigitalInputOutput.cpp
	src/Sensors/Generic/4DI4DO.cpp
	src/Sensors/Murata/SCL3X00.cpp
	src/Sensors/Murata/SCL3300.cpp
	src/Sensors/Murata/SCL3300a.cpp
	src/Sensors/Murata/SCL3400.cpp
	src/Sensors/Honeywell/ABP.cpp
	src/Sensors/Generic/hx711.cpp

	src/main.cpp
)

target_link_libraries(
	${PROJECT_NAME} 
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
	xerxes-protocol
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${SENSOR_TYPE})

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
# pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})

# copy .UF2 binary to the RP2040
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/${SENSOR_TYPE}.uf2
	DESTINATION /media/$ENV{USER}/RPI-RP2/${SENSOR_TYPE}.uf2
)