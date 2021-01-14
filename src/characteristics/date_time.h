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
    static void dataReceivedToDateTime(const uint8_t *data, DateTime& dateTime);
    DateTimeChar(BleCharacteristic& ch): _characteristic(ch) {}
    ~DateTimeChar() {};
};

DateTimeChar::DateTime operator+(const DateTimeChar::DateTime& base, const int16_t offset);
