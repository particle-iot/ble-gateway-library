#pragma once

#include "Particle.h"
#include "cmath"

#define BLE_SIG_BLOOD_PRESSURE_MEASUREMENT_CHAR 0x2A35
#define BLE_SIG_INTERMEDIATE_CUFF_PRESSURE_CHAR 0x2A36
#define BLE_SIG_BLOOD_PRESSURE_FEATURE_CHAR 0x2A49

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

class BloodPressureMeasurement
{
protected:
    enum BloodPressureMeasurementFlags: uint8_t {
        NONE                = 0,
        UNIT_IS_KPA         = 0x01,
        TIMESTAMP_PRESENT   = 0x02,
        PULSE_PRESENT       = 0x04,
        USERID_PRESENT      = 0x08,
        STATUS_PRESENT      = 0x10
    };
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
        BloodPressureMeasurement* ctx = (BloodPressureMeasurement *)context;
        size_t position = 0;
        if (len > 0) ctx->_flags = (BloodPressureMeasurementFlags)data[position++];
        if (len > (position + 5)) {
            ctx->_systolic = data[position+1] << 8 | (data[position]);
            ctx->_diastolic = data[position+3] << 8 | (data[position+2]);
            ctx->_map = data[position+5] << 8 | (data[position+4]);
            position += 6;
        }
        if (ctx->_flags & BloodPressureMeasurementFlags::TIMESTAMP_PRESENT && len > (position + 6)) {
            ctx->_ts_year = data[position+1] << 8 | (data[position]);
            ctx->_ts_month = data[position+2];
            ctx->_ts_day = data[position+3];
            ctx->_ts_hour = data[position+4];
            ctx->_ts_min = data[position+5];
            ctx->_ts_sec = data[position+6];
            position += 7;
        }
        if (ctx->_flags & BloodPressureMeasurementFlags::PULSE_PRESENT && len > (position + 1)) {
            ctx->_pulse = data[position+1] << 8 | (data[position]);
            position += 2;
        }
        if (ctx->_flags & BloodPressureMeasurementFlags::USERID_PRESENT && len > position) {
            ctx->_userid = data[position++];
        }
        if (ctx->_flags & BloodPressureMeasurementFlags::STATUS_PRESENT && len > (position + 1)) {
            ctx->_measurement_status = data[position+1] << 8 | data[position];
        }
        if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(ctx->_characteristic.UUID(), ctx->_notifyContext);
    }
    BloodPressureMeasurementFlags _flags;
    BleCharacteristic& _characteristic;
    uint16_t _systolic, _diastolic, _map, _ts_year, _pulse, _measurement_status;
    uint8_t _ts_month, _ts_day, _ts_hour, _ts_min, _ts_sec, _userid;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
public:
    void onConnect() {
        _characteristic.onDataReceived(onDataReceived, this);
    };
    int getSystolic(float& bp) const {return ieee11073_20601a_sfloat(_systolic, bp);}
    int getDiastolic(float& bp) const {return ieee11073_20601a_sfloat(_diastolic, bp);}
    int getMeanArterial(float& bp) const {return ieee11073_20601a_sfloat(_map, bp);}
    int enableNotification() {return _characteristic.subscribe(true);}
    void notifyCallback(void (*callback)(BleUuid, void*), void* context) {
        _notifyNewData = callback;
        _notifyContext = context;
    }
    BloodPressureMeasurement(BleCharacteristic& ch): _flags(BloodPressureMeasurementFlags::NONE), _characteristic(ch) {}
    ~BloodPressureMeasurement() {};
};

class IntermediateCuffPressureChar: public BloodPressureMeasurement {
public:
    int getCuffPressure(float& bp) const {return this->getSystolic(bp);}
    IntermediateCuffPressureChar(BleCharacteristic& ch): BloodPressureMeasurement(ch) {};
};

class BloodPressureFeatureChar {
private:
    enum BloodPressureFeatureFlags: uint8_t {
        BODY_MOVEMENT_DETECTION_SUPPORT         = 0x01,
        CUFF_FIT_DETECTION_SUPPORT              = 0x02,
        IRREGULAR_PULSE_DETECTION_SUPPORT       = 0x04,
        PULSE_RATE_RANGE_DETECTION_SUPPORT      = 0x08,
        MEASUREMENT_POSITION_DETECTION_SUPPORT  = 0x10,
        MULTIPLE_BOND_SUPPORT                   = 0x20
    };
    BleCharacteristic& _characteristic;
    BloodPressureFeatureFlags _flags;
public:
    void onConnect() {_characteristic.getValue((uint8_t *)&_flags, 1);}

    bool isBodyMovementDetectionSupported() const {return _flags & BloodPressureFeatureFlags::BODY_MOVEMENT_DETECTION_SUPPORT;}
    bool isCuffFitDetectionSupported() const {return _flags & BloodPressureFeatureFlags::CUFF_FIT_DETECTION_SUPPORT;}
    bool isIrregularPulseDetectionSupported() const {return _flags & BloodPressureFeatureFlags::IRREGULAR_PULSE_DETECTION_SUPPORT;}
    bool isPulseRateRangeDetectionSupported() const {return _flags & BloodPressureFeatureFlags::PULSE_RATE_RANGE_DETECTION_SUPPORT;}
    bool isMeasurementPositionDetectionSupported() const {return _flags & BloodPressureFeatureFlags::MEASUREMENT_POSITION_DETECTION_SUPPORT;}
    bool isMultipleBondSupported() const {return _flags & BloodPressureFeatureFlags::MULTIPLE_BOND_SUPPORT;}
   
    BloodPressureFeatureChar(BleCharacteristic& ch): _characteristic(ch) {};
    ~BloodPressureFeatureChar() {};
};