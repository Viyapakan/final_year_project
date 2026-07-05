#include "json_builder.h"
#include <ArduinoJson.h>
#include <time.h>

// -----------------------------------------------------------------------------
// Generalized JSON Builder
// -----------------------------------------------------------------------------
String build_sensor_json(lora_packet_t *packet) {
    // 512 bytes gives plenty of headroom for future, larger sensor payloads
    StaticJsonDocument<512> doc; 

    // 1. Attach Common Header Data
    doc["device_id"] = packet->device_id;
    doc["timestamp"] = time(nullptr); // Current NTP synced time
    
    // Create the nested object where specific sensor readings will go
    JsonObject data = doc.createNestedObject("data");

    // 2. Decode the payload based on Sensor Type
    switch(packet->sensor_type) {
        
        case SENSOR_TYPE_SOIL: {
            // Security check
            if (packet->payload_length != sizeof(soil_payload_t)) return ""; 
            
            doc["sensor_type"] = "SOIL_V1";
            soil_payload_t *soil = (soil_payload_t *)packet->payload;
            
            data["temperature_c"] = soil->temperature / 10.0;
            data["humidity_pct"] = soil->humidity / 10.0;
            data["ec_us_cm"] = soil->ec;
            break;
        }

        case SENSOR_TYPE_ENV: {
            // Security check: Ensure the payload is exactly the 4 bytes we expect
            if (packet->payload_length != sizeof(env_payload_t)) {
                Serial.println("[ERR] ENV Payload size mismatch!");
                return ""; 
            }
            
            doc["sensor_type"] = "ENV_V1";
            
            // Map the raw payload bytes to our new struct
            env_payload_t *env = (env_payload_t *)packet->payload;
            
            // Divide by 100.0 to restore the 2 decimal places from the STM32
            data["temperature_c"] = env->temperature / 100.0;
            data["humidity_pct"]  = env->humidity / 100.0;
            break;
        }
        
        default:
            // Unknown sensor type, reject the payload
            return ""; 
    }

    // 3. Serialize to String
    String output;
    serializeJson(doc, output);
    return output;
}


// -----------------------------------------------------------------------------
// [FIX] Buffer-based JSON Builder — Zero heap allocation version.
// Writes the serialized JSON directly into the caller's fixed-size char buffer.
// This is the version used in loop() to prevent long-term heap fragmentation
// that would eventually cause malloc() failures after hundreds of sensor cycles.
// -----------------------------------------------------------------------------
bool build_sensor_json_to_buf(lora_packet_t *packet, char *buf, size_t buf_len) {
    StaticJsonDocument<512> doc;

    // 1. Attach Common Header Data
    doc["device_id"]  = packet->device_id;
    doc["timestamp"]  = time(nullptr);

    JsonObject data = doc.createNestedObject("data");

    // 2. Decode the payload based on Sensor Type
    switch(packet->sensor_type) {

        case SENSOR_TYPE_SOIL: {
            if (packet->payload_length != sizeof(soil_payload_t)) return false;
            doc["sensor_type"] = "SOIL_V1";
            soil_payload_t *soil = (soil_payload_t *)packet->payload;
            data["temperature_c"] = soil->temperature / 10.0;
            data["humidity_pct"]  = soil->humidity    / 10.0;
            data["ec_us_cm"]      = soil->ec;
            break;
        }

        case SENSOR_TYPE_ENV: {
            if (packet->payload_length != sizeof(env_payload_t)) {
                return false;
            }
            doc["sensor_type"] = "ENV_V1";
            env_payload_t *env = (env_payload_t *)packet->payload;
            data["temperature_c"] = env->temperature / 100.0;
            data["humidity_pct"]  = env->humidity    / 100.0;
            break;
        }

        default:
            return false;
    }

    // 3. Serialize directly into the caller's fixed buffer — no heap involved.
    size_t written = serializeJson(doc, buf, buf_len);
    return (written > 0);
}