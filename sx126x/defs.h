#ifndef sx126x_defs
#define sx126x_defs

#include "hardware/spi.h"
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    spi_inst_t* spi;
    uint cs_pin;
    uint busy_pin;
    uint reset_pin;
} sx126x_pico_context_t;

#ifdef __cplusplus
}
#endif

#endif
