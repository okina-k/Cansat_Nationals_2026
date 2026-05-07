add_library(adxl345
    interface/driver_adxl345_interface.c
    src/driver_adxl345.c
)

target_include_directories(adxl345
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/interface
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(adxl345
    PUBLIC
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_gpio
)