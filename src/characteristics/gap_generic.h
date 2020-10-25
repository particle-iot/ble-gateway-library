#pragma once

#include "Particle.h"

#define BLE_SIG_DEVICE_NAME_CHAR 0x2A00
#define BLE_SIG_APPEARANCE_CHAR 0x2A01
#define BLE_SIG_PERIPHERAL_PREFERRED_CONNECTION_PARAMS_CHAR 0x2A04

class DeviceNameCharacteristic
{
private:
    BleCharacteristic _characteristic;
public:
    int setValue(const char* str) {
        return _characteristic.setValue(str);
    };
    int getValue(uint8_t* buf, size_t len) {
        return _characteristic.getValue(buf, len);
    }
    DeviceNameCharacteristic(BleCharacteristic& ch): _characteristic(ch) {};
    ~DeviceNameCharacteristic() {};
};

class Appearance
{
private:
    BleCharacteristic _characteristic;
public:
    enum Value: uint16_t {
        NONE                = 0,
        PHONE               = 64,
        COMPUTER            = 128,
        WATCH               = 192,
        WATCH_SPORTS        = 193,
        CLOCK               = 256,
        DISPLAY             = 320,
        REMOTE_CONTROL      = 384,
        EYE_GLASSES         = 448,
        TAG                 = 512,
        KEYRING             = 576,
        MEDIA_PLAYER        = 640,
        BARCODE_SCANNER     = 704,
        THERMOMETER         = 768,
        THERMOMETER_EAR     = 769,
        HEART_RATE_SENSOR   = 832,
        HEART_RATE_BELT     = 833,
        BLOOD_PRESSURE      = 896,
        BLOOD_PRESSURE_ARM  = 897,
        BLOOD_PRESSURE_WRIST = 898,
        HID_GENERIC         = 960,
        HID_KEYBOARD        = 961,
        HID_MOUSE           = 962,
        HID_JOYSTIC         = 963,
        HID_GAMEPAD         = 964,
        HID_DIGITIZER_TABLET = 965,
        HID_CARD_READER     = 966,
        HID_DIGITAL_PEN     = 967,
        HID_BARCODE_SCANNER = 968,
        GLUCOSE_METER       = 1024,
        RUNNING_WALKING_SENSOR = 1088,
        RUNNING_WALKING_SENSOR_IN_SHOE = 1089,
        RUNNING_WALKING_SENSOR_ON_SHOE = 1090,
        RUNNING_WALKING_SENSOR_ON_SHIP = 1091,
        CYCLING_GENERIC     = 1152,
        CYCLING_COMPUTER    = 1153,
        CYCLING_SPEED       = 1154,
        CYCLING_CADENCE     = 1155,
        CYCLING_POWER       = 1156,
        CYCLING_SPEED_AND_CADENCE = 1157,
        PULSE_OXIMETER      = 3136,
        PULSE_OXIMETER_FINGERTIP = 3137,
        PULSE_OXIMETER_WRIST = 3138,
        WEIGHT_SCALE        = 3200,
        OUTDOOR_SPORTS_GENERIC = 5184,
        OUTDOOR_SPORTS_LOCATION = 5185,
        OUTDOOR_SPORTS_LOCATION_AND_NAVIGATION = 5186,
        OUTDOOR_SPORTS_LOCATION_POD = 5187,
        OUTDOOR_SPORTS_LOCATION_AND_NAVIGATION_POD = 5188,
        ENVIRONMENTAL_SENSOR = 5696
    };
    int setValue(Value app) {
        uint8_t buf[2];
        buf[0] = (uint8_t)app;
        buf[1] = (uint8_t)(app >> 8);
        return _characteristic.setValue(buf, 2);
    }
    Value getValue() {
        uint16_t val;
        return (_characteristic.getValue((uint16_t *)&val) == 2) ? (Value)(val) : Value::NONE;
    } 
    Appearance(BleCharacteristic& ch): _characteristic(ch) {};
    ~Appearance() {};
};

class PeripheralPreferredConnectionParams
{
private:
    BleCharacteristic _characteristic;
public:
    PeripheralPreferredConnectionParams(BleCharacteristic& ch): _characteristic(ch) {};
    ~PeripheralPreferredConnectionParams() {};
};