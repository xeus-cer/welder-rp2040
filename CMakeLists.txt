cmake_minimum_required(VERSION 3.22)

# this must be first to set the correct toolchain
include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
include(pico_extras_import.cmake)

set(PROJECT_NAME sensor-rp2040)
project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)


set(DEVICE_TYPE_HINT "Possible options for SENSOR_TYPE: SCL3300, SCL3300a, "
"SCL3400, AI, 4DI4DO, ABP, hx711, Encoder, Cutter, EnviroLS")

# this is the board we are using, we need to set this to use the correct board header and bootloader
# the board file is: src/Hardware/Board/xerxes_rp2040.h
set(PICO_BOARD "xerxes_rp2040")
set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/Hardware/Board")

# parse version and build number from git
execute_process(
    COMMAND git describe --long
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(__VERSION="${VERSION}")

if(NOT DEFINED DEVICE_ADDRESS)
    set(DEVICE_ADDRESS 0)
endif()

message("DEVICE_ADDRESS set to ${DEVICE_ADDRESS}")
add_compile_definitions(__DEVICE_ADDRESS=${DEVICE_ADDRESS})

# check if ${CLKDIV} is set and set it to 4 if not - necessary for some boards
if(NOT DEFINED CLKDIV)
	set(CLKDIV 4)
endif()
add_compile_definitions(__CLKDIV=${CLKDIV})

if(NOT DEFINED LOG_LEVEL)
	set(LOG_LEVEL 1)
endif()
add_compile_definitions(_LOG_LEVEL=${LOG_LEVEL})
if(${LOG_LEVEL} EQUAL 1)
    message("LOG_LEVEL set to ${LOG_LEVEL} = ERROR")
elseif(${LOG_LEVEL} EQUAL 2)
    message("LOG_LEVEL set to ${LOG_LEVEL} = WARNING")
elseif(${LOG_LEVEL} EQUAL 3)
    message("LOG_LEVEL set to ${LOG_LEVEL} = INFO")
elseif(${LOG_LEVEL} EQUAL 4)
    message("LOG_LEVEL set to ${LOG_LEVEL} = DEBUG")
elseif(${LOG_LEVEL} EQUAL 5)
    message("LOG_LEVEL set to ${LOG_LEVEL} = TRACE")
endif()

# check if ${DEVICE_TYPE} is set
if(NOT DEFINED DEVICE_TYPE)
	message(FATAL_ERROR "DEVICE_TYPE not set, use -DSENSOR_TYPE=...\n${DEVICE_TYPE_HINT}")
endif()

# this is the sensor type we are using, we need to set this to use the correct sensor header
if(${DEVICE_TYPE} STREQUAL "SCL3300")
	add_compile_definitions(__SHIELD_SCL3300)
	add_compile_definitions(__DEVICE_CLASS=SCL3300)
elseif(${DEVICE_TYPE} STREQUAL "SCL3300a")
	add_compile_definitions(__SHIELD_SCL3300)
	add_compile_definitions(__DEVICE_CLASS=SCL3300a)
elseif(${DEVICE_TYPE} STREQUAL "SCL3400")
	add_compile_definitions(__SHIELD_SCL3400)
	add_compile_definitions(__DEVICE_CLASS=SCL3400)
elseif(${DEVICE_TYPE} STREQUAL "AI")
	add_compile_definitions(__SHIELD_AI)
	add_compile_definitions(__DEVICE_CLASS=AnalogInput)
elseif(${DEVICE_TYPE} STREQUAL "AnalogInput")
	add_compile_definitions(__SHIELD_AI)
	add_compile_definitions(__DEVICE_CLASS=AnalogInput)
elseif(${DEVICE_TYPE} STREQUAL "4DI4DO")
	add_compile_definitions(__SHIELD_4DI4DO)
	add_compile_definitions(__DEVICE_CLASS=_4DI4DO)
elseif(${DEVICE_TYPE} STREQUAL "ABP")
	add_compile_definitions(__SHIELD_ABP)
	add_compile_definitions(__DEVICE_CLASS=ABP)
elseif(${DEVICE_TYPE} STREQUAL "hx711")
	add_compile_definitions(__SHIELD_HX711)
	add_compile_definitions(__DEVICE_CLASS=HX711)
elseif(${DEVICE_TYPE} STREQUAL "Encoder")
    add_compile_definitions(__SHIELD_ENCODER)
    add_compile_definitions(__DEVICE_CLASS=Encoder)
elseif(${DEVICE_TYPE} STREQUAL "Cutter")
    add_compile_definitions(__SHIELD_CUTTER)
    add_compile_definitions(__TIGHTLOOP)
    add_compile_definitions(__DEVICE_CLASS=Cutter)
elseif(${DEVICE_TYPE} STREQUAL "temp")
    add_compile_definitions(__SHIELD_TEMP)
    add_compile_definitions(__DEVICE_CLASS=DS18B20)
elseif(${DEVICE_TYPE} STREQUAL "EnviroLS")
    add_compile_definitions(__SHIELD_ENVIROLS)
    add_compile_definitions(__DEVICE_CLASS=LightSound)
else()
	message(FATAL_ERROR "DEVICE_TYPE '${DEVICE_TYPE}' is incorrect, use -DDEVICE_TYPE=...\n${DEVICE_TYPE_HINT}")
endif()

# print warning to clearly show which device type is used and which CLKDIV is set
message(WARNING "\nDEVICE_TYPE set to '${DEVICE_TYPE}'."
	"\nCLKDIV set to ${CLKDIV}, increase this "
	"value if device is not responding after reboot.")

pico_sdk_init()
 
# find xerxes-protocol library
set(xerxes-protocol_DIR "lib/xerxes-protocol-cpp")
find_package(xerxes-protocol REQUIRED)
add_library(xerxes-protocol STATIC ${xerxes-protocol_SOURCES})

include_directories(
	"src"
	${xerxes-protocol_INCLUDE_DIRS}
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
	src/Sensors/Generic/DIO/DigitalInputOutput.cpp
	src/Sensors/Generic/DIO/4DI4DO.cpp
	src/Sensors/Murata/SCL3X00.cpp
	src/Sensors/Murata/SCL3300.cpp
	src/Sensors/Murata/SCL3300a.cpp
	src/Sensors/Murata/SCL3400.cpp
	src/Sensors/Honeywell/ABP.cpp
	src/Sensors/Generic/hx711.cpp
    src/Sensors/Generic/DIO/Encoder.cpp
    src/Sensors/Generic/DIO/Cutter.cpp
    src/Sensors/Generic/ds18b20/ds18b20.cpp
    src/Sensors/Generic/Enviro/LightSound.cpp

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

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${DEVICE_TYPE})

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
# pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})

# copy .UF2 binary to the RP2040
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_TYPE}.uf2
	DESTINATION /media/$ENV{USER}/RPI-RP2/${DEVICE_TYPE}.uf2
)