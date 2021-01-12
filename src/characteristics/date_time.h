#pragma once

#include "Particle.h"

#define BLE_SIG_DATE_TIME_CHAR 0x2A08

class DateTimeChar
{
protected:
    BleCharacteristic _characteristic;

public:
    struct DateTime {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    };
    static void dataReceivedToDateTime(const uint8_t *data, DateTime dateTime) {
        dateTime.year = data[1] << 8 | (data[0]);
        dateTime.month = data[2];
        dateTime.day = data[3];
        dateTime.hour = data[4];
        dateTime.minute = data[5];
        dateTime.second = data[6];
    };
    DateTimeChar(BleCharacteristic& ch): _characteristic(ch) {}
    ~DateTimeChar() {};
};
