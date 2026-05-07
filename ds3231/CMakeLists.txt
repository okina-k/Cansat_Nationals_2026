add_library(ds3231
    interface/driver_ds3231_interface.c
    src/driver_ds3231.c
)

target_include_directories(ds3231
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/interface
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(ds3231
    PUBLIC
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_gpio
)