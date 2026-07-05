#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include "app_types.h"
#include <Arduino.h>


// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
// Original: returns an Arduino String (kept for backward compatibility)
String build_sensor_json(lora_packet_t *packet);

// [FIX] New: writes directly into a pre-allocated buffer — zero heap
// allocations. Returns true on success, false if the packet type is unknown or
// sizes mismatch. Use this in loop() to eliminate long-term heap fragmentation.
bool build_sensor_json_to_buf(lora_packet_t *packet, char *buf, size_t buf_len);

#endif // JSON_BUILDER_H