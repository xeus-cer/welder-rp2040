cmake_minimum_required(VERSION 3.22)

include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
include(pico_extras_import.cmake)

set(PROJECT_NAME sensor-rp2040)

# this is the board we are using, we need to set this to use the correct board header and bootloader
# the board file is: src/Hardware/Board/xerxes_rp2040.h
set(PICO_BOARD "xerxes_rp2040")
set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/Hardware/Board")

# check if ${SENSOR_TYPE} is set
if(NOT DEFINED SENSOR_TYPE)
	message(FATAL_ERROR "SENSOR_TYPE not set, use -DSENSOR_TYPE=...\n\
	Possible options for SENSOR_TYPE: SCL3300, SCL3400, AI, 4DI4DO, ABP")
endif()

# this is the sensor type we are using, we need to set this to use the correct sensor header
if(${SENSOR_TYPE} STREQUAL "SCL3300")
	add_compile_definitions(SHIELD_SCL3300)
	add_compile_definitions(SENSOR_CLASS=SCL3300)
elseif(${SENSOR_TYPE} STREQUAL "SCL3400")
	add_compile_definitions(SHIELD_SCL3400)
	add_compile_definitions(SENSOR_CLASS=SCL3400)
elseif(${SENSOR_TYPE} STREQUAL "AI")
	add_compile_definitions(SHIELD_AI)
	add_compile_definitions(SENSOR_CLASS=AnalogInput)
elseif(${SENSOR_TYPE} STREQUAL "4DI4DO")
	add_compile_definitions(SHIELD_4DI4DO)
	add_compile_definitions(SENSOR_CLASS=_4DI4DO)
elseif(${SENSOR_TYPE} STREQUAL "ABP")
	add_compile_definitions(SHIELD_ABP)
	add_compile_definitions(SENSOR_CLASS=ABP)
else()
	message(FATAL_ERROR "SENSOR_TYPE '" ${SENSOR_TYPE}"' is incorrect, use -DSENSOR_TYPE=...\n\
	Possible options for SENSOR_TYPE: SCL3300, SCL3400, AI, 4DI4DO, ABP")
endif()


project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()
	
include_directories(
	"src"
)


add_executable(
	${PROJECT_NAME}

	src/Communication/Callbacks.cpp
	src/Hardware/ClockUtils.cpp
	src/Hardware/Sleep.cpp
	src/Hardware/InitUtils.cpp
	src/Hardware/UserFlash.cpp
	src/Communication/Message.cpp
	src/Communication/Packet.cpp
	src/Communication/Protocol.cpp
	src/Communication/RS485.cpp
	src/Core/Slave.cpp
	src/Core/Register.cpp
	src/Communication/Network.cpp
	src/Sensors/Peripheral.cpp
	src/Sensors/Sensor.cpp
	src/Sensors/Generic/AnalogInput.cpp
	src/Sensors/Generic/DigitalInputOutput.cpp
	src/Sensors/Generic/4DI4DO.cpp
	src/Sensors/Murata/SCL3X00.cpp
	src/Sensors/Murata/SCL3300.cpp
	src/Sensors/Murata/SCL3400.cpp
	src/Sensors/Honeywell/ABP.cpp

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
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
# pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})

# copy .UF2 binary to the RP2040
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.uf2
	DESTINATION /media/$ENV{USER}/RPI-RP2/${PROJECT_NAME}.uf2
)