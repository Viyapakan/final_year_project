#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <Arduino.h>
#include "app_types.h"

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
String build_sensor_json(lora_packet_t *packet);

#endif // JSON_BUILDER_H