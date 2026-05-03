# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(CanSat C CXX ASM)

# Initialise the Raspberry Pi Pico SD
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

#
add_executable(CanSat 

    CanSat_2.cpp 
    modules/spi.cpp 
    modules/i2c.cpp 
    modules/TX.cpp 
    modules/sensors.cpp 
    modules/rtc.cpp
    modules/io.cpp
    modules/conversions.cpp
    modules/buzzer.cpp
    hw_config.c
    )
    
#[[
add_executable(CanSat 
    modules/conversions.cpp
    RX_2.cpp
    )
]]

pico_set_program_name(CanSat "CanSat")
pico_set_program_version(CanSat "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(CanSat 1)
pico_enable_stdio_usb(CanSat 1)

add_subdirectory(sx126x)
add_subdirectory(adxl345)
add_subdirectory(BMP280)
add_subdirectory(ds3231)

#add_subdirectory(fatfs)
add_subdirectory(no-OS-FatFS-SD-SDIO-SPI-RPi-Pico-main/src)

# Add the standard library to the build
target_link_libraries(CanSat
sx126x
adxl345
BMP280
ds3231
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico

        pico_stdlib)

# Add the standard include files to the build
target_include_directories(CanSat PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/include
        ${CMAKE_CURRENT_LIST_DIR}/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico-main/src
)

# Add any user requested libraries
target_link_libraries(CanSat 
        hardware_spi
        hardware_i2c
        hardware_pwm
        )

pico_add_extra_outputs(CanSat)

target_sources(CanSat PRIVATE
    hw_config.c
)

