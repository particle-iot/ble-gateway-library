#pragma once
#include "cmath"

static int ieee11073_20601a_sfloat(uint16_t value, float& data) {
    if (value >= 0x07FE && value <= 0x0802) {
        data = (float)value;
        return -2;
    }
    uint16_t mantissa = value & 0x0FFF;
    uint8_t exponent = value >> 12;
    float magnitude = std::pow(10.0, (exponent >= 0x08) ? -(0x10-exponent): exponent);
    mantissa = (mantissa >= 0x0800) ? -(0x1000 - mantissa) : mantissa;
    data = mantissa * magnitude;
    return 0;
}