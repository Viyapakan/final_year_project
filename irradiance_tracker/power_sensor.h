#ifndef POWER_SENSOR_H
#define POWER_SENSOR_H

#include <Wire.h>
#include <INA226.h>

// Your library expects the address (0x40) right here
INA226 ina(0x40);

bool init_power_sensor() {
    Serial.print("Initializing INA226...");
    
    Wire.begin(); // Ensure I2C bus is started
    if (!ina.begin()) {
        Serial.println(" ERROR: INA226 not found!");
        return false;
    }

    // This library uses 'setMaxCurrentShunt' instead of 'calibrate'
    // Parameters: Max Amps (10), Shunt Ohms (0.1)
    ina.setMaxCurrentShunt(10, 0.1);

    Serial.println(" Success! INA226 Ready.");
    return true;
}

void get_power_data(float &voltage, float &current) {
    // Your library uses 'getBusVoltage' and 'getCurrent'
    voltage = ina.getBusVoltage();
    current = ina.getCurrent();
}

#endif