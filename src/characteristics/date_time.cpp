#include "date_time.h"

void DateTimeChar::dataReceivedToDateTime(const uint8_t *data, DateTime& dateTime){
        dateTime.year = data[1] << 8 | (data[0]);
        dateTime.month = data[2];
        dateTime.day = data[3];
        dateTime.hour = data[4];
        dateTime.minute = data[5];
        dateTime.second = data[6];
    };

DateTimeChar::DateTime operator+(const DateTimeChar::DateTime& base, const int16_t offset) {
    DateTimeChar::DateTime time;
    time.second = base.second;
    time.minute = (base.minute + offset) % 60;
    time.hour = (base.hour + (base.minute+offset)/60) % 24;
    time.day = (base.day + (base.hour + (base.minute+offset)/60)/24) % 31;
    time.month = (base.month + (base.day + (base.hour + (base.minute+offset)/60)/24)/31) % 12;
    time.year = (base.year + (base.month + (base.day + (base.hour + (base.minute+offset)/60)/24)/31)/12);
    return time;
}
