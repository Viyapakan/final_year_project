#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

// -----------------------------------------------------------------------------
// MQTT Broker Credentials (From EMQX Dashboard)
// -----------------------------------------------------------------------------
#define MQTT_BROKER    "i3952631.ala.asia-southeast1.emqxsl.com"
#define MQTT_PORT      8883
#define MQTT_CLIENT_ID "ESP32_Gateway_01" // Must be unique per device
#define MQTT_USER      "fyp_user"
#define MQTT_PASSWORD  "Cjed5Tdva52JbA4"

// -----------------------------------------------------------------------------
// MQTT Timing Configuration
// -----------------------------------------------------------------------------
constexpr uint32_t MQTT_RETRY_INTERVAL_MS = 5000;

// -----------------------------------------------------------------------------
// EMQX CA Certificate (Root CA for TLS)
// -----------------------------------------------------------------------------
// Downloaded from the EMQX console. This ensures against Man-in-the-Middle attacks.
const char* EMQX_CA_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRnXubJIVReMwDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n" \
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n" \
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n" \
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJ1y/DvsCCElcMAC1\n" \
"pzVEuRhO+2z9Q5V3e7gE9tY5T4l3s+c7V7w/h7P2nU9p5J5z8Z4O1oT5z6t4n5+x\n" \
"3v9/e/T7x+1+x8v/9/1+x8v/9/1+x8v/9/1+x8v/9/1+x8v/9/1+x8v/9/1+x8v/\n" \
"This is a truncated example of the Let's Encrypt Root CA.\n" \
"Please replace this block with the actual CA text from your download.\n" \
"-----END CERTIFICATE-----\n";

#endif // MQTT_CONFIG_H