// Example usage for ble-devices library by Mariano Goluboff.

#include "ble-devices.h"

// Initialize objects from the lib
BleDeviceGateway bledevices;

void setup() {
    // Call functions on initialized library objects that require hardware
    bledevices.begin();
}

void loop() {
    // Use the library's initialized objects and functions
    bledevices.process();
}
