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
            // For future expansion! 
            doc["sensor_type"] = "ENV_V1";
            // env_payload_t *env = (env_payload_t *)packet->payload;
            // data["co2"] = env->co2;
            // data["voc"] = env->voc;
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