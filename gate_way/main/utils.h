#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "pin_config.h"

// Function prototypes returning bool for error handling/debugging
bool led_on();
bool led_off();
bool led_blink(uint16_t how_many_blinks, uint32_t in_which_gap_ms);

#endif // UTILS_H