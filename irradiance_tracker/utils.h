#ifndef UTILS_H
#define UTILS_H

const int LED_PIN = 2;

inline void led_on() {
    digitalWrite(LED_PIN, HIGH);
}

inline void led_off() {
    digitalWrite(LED_PIN, LOW);
}

inline void led_toggle(int toggle_times, int delay_ms) {
    for (int i = 0; i < toggle_times * 2; i++) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(delay_ms);
    }
}

#endif