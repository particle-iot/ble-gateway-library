#pragma once

#include "Particle.h"

#define BLE_SIG_HEART_RATE_MEASUREMENT_CHAR 0x2A37
#define BLE_SIG_BODY_SENSOR_LOCATION_CHAR 0x2A38
#define BLE_SIG_HEART_RATE_CONTROL_POINT_CHAR 0x2A39

class HeartRateMeasurement
{
private:
    enum HeartRateMeasurementFlags: uint8_t {
        NONE                                = 0,
        HEART_RATE_VALUE_UINT16             = 0x01,
        SENSOR_CONTACT_DETECTED             = 0x02,
        SENSOR_CONTACT_FEATURE_SUPPORTED    = 0x04,
        ENERGY_EXPENDED_PRESENT             = 0x08,
        RR_INTERVAL_PRESENT                 = 0x10
    };
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
        HeartRateMeasurement* ctx = (HeartRateMeasurement *)context;
        bool changed = false;
        size_t position = 0;
        if (len > 0) ctx->_flags = (HeartRateMeasurementFlags)data[position++];
        if ( !(ctx->_flags & HeartRateMeasurementFlags::SENSOR_CONTACT_FEATURE_SUPPORTED) ||
                ( ctx->_flags & HeartRateMeasurementFlags::SENSOR_CONTACT_FEATURE_SUPPORTED && 
                ctx->_flags & HeartRateMeasurementFlags::SENSOR_CONTACT_DETECTED )){
            if (ctx->_flags & HeartRateMeasurementFlags::HEART_RATE_VALUE_UINT16 && len > (position+1)) {
                if ( (data[position+1] << 8 | data[position]) != ctx->_hr) changed = true;
                ctx->_hr = data[position+1] << 8 | (data[position]);
                position += 2;
            } else if (len > position) {
                if ( data[position] != ctx->_hr ) changed = true;
                ctx->_hr = data[position++];
            }
        }
        if (len > position+1 && ctx->_flags & HeartRateMeasurementFlags::ENERGY_EXPENDED_PRESENT) {
            if ( (data[position+1] << 8 | data[position]) != ctx->_joules) changed = true;
            ctx->_joules = data[position+1] << 8 | (data[position]);
            position += 2;
        }
        if (ctx->_flags & HeartRateMeasurementFlags::RR_INTERVAL_PRESENT) {
            changed = true;
            ctx->_rrInterval.clear();
            while (len > position + 2) {
                ctx->_rrInterval.append(data[position+1] << 8 | data[position]);
                position += 2;
            }
        }
        if (ctx->_notifyNewData != nullptr && (!ctx->_notifyOnNewValueOnly || changed)) (ctx->_notifyNewData)(ctx->_characteristic.UUID(), ctx->_notifyContext);
    }
    HeartRateMeasurementFlags _flags;
    uint16_t _hr, _joules;
    BleCharacteristic _characteristic;
    Vector<uint16_t> _rrInterval;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
    bool _notifyOnNewValueOnly;
public:
    void onConnect() {
        _characteristic.onDataReceived(onDataReceived, this);
    };

    uint16_t getHeartRate() const {return _hr;}
    uint16_t getEnergyExpended() const {return _joules;}
    int enableNotification() {return _characteristic.subscribe(true);}
    void notifyCallback(void (*callback)(BleUuid, void*), void* context, bool onNewValueOnly = false) {
        _notifyNewData = callback;
        _notifyContext = context;
        _notifyOnNewValueOnly = onNewValueOnly;
        }

    HeartRateMeasurement(BleCharacteristic& ch): _flags(HeartRateMeasurementFlags::NONE),  _hr(0), _joules(0), _notifyNewData(nullptr) {_characteristic = ch;}
    ~HeartRateMeasurement() {};
};

class BodySensorLocation 
{
private:
    BleCharacteristic _characteristic;
public:
    enum SensorLocation: uint8_t {
        OTHER       = 0,
        CHEST       = 1,
        WRIST       = 2,
        FINGER      = 3,
        HAND        = 4,
        EAR_LOBE    = 5,
        FOOT        = 6,
        ERROR       = 0x80
    };
    SensorLocation read() {
        uint8_t buf;
        return (_characteristic.getValue(&buf, 1) == 1) ? (SensorLocation)buf : SensorLocation::ERROR; 
    }
    BodySensorLocation(BleCharacteristic& ch) {_characteristic = ch;}
    ~BodySensorLocation() {}
};

class HeartRateControlPoint 
{
private:
    BleCharacteristic _characteristic;
public:
    int resetEnergyExpended() {
        uint8_t buf = 1;
        return _characteristic.setValue(&buf, 1);
    }

    HeartRateControlPoint(BleCharacteristic& ch) {_characteristic = ch;}
    ~HeartRateControlPoint() {}
};