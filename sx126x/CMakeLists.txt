add_library(sx126x
    src/sx126x.c
    src/sx126x_hal.c
    src/sx126x_driver_version.c
    src/sx126x_lr_fhss.c
    src/lr_fhss_mac.c
)

target_include_directories(sx126x
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(sx126x
    PUBLIC
    pico_stdlib
    hardware_spi
    hardware_gpio
)