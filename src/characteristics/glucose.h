#pragma once

#include "Particle.h"
#include "date_time.h"

#define BLE_SIG_GLUCOSE_MEASUREMENT_CHAR 0x2A18
#define BLE_SIG_GLUCOSE_MEASUREMENT_CONTEXT_CHAR 0x2A34
#define BLE_SIG_RECORD_ACCESS_CONTROL_CHAR 0x2A52
#define BLE_SIG_GLUCOSE_FEATURE_CHAR 0x2A51

class GlucoseMeasurement
{
public:
    enum class Annunciation: uint16_t {
        NO_ANNUNCIATION                         = 0x00,
        BATTERY_LOW_AT_MEASUREMENT              = 0x01,
        SENSOR_MALFUNCTION                      = 0x02,
        SAMPLE_SIZE_INSUFFICIENT                = 0x04,
        STRIP_INSERTION_ERROR                   = 0x08,
        STRIP_TYPE_INCORRECT                    = 0x10,
        SENSOR_RESULT_HIGHER_THAN_CAN_PROCESS   = 0x20,
        SENSOR_RESULT_LOWER_THAN_CAN_PROCESS    = 0x40,
        SENSOR_TEMPERATURE_TOO_HIGH             = 0x80,
        SENSOR_TEMPERATURE_TOO_LOW              = 0x100,
        INTERRUPTED_PULLED_TOO_SOON             = 0x200,
        GENERAL_DEVICE_FAULT                    = 0x400,
        TIME_FAULT_MAY_BE_INACCURATE            = 0x800
    };
    enum GlucoseMeasurementFlags: uint8_t {
        NONE                                    = 0,
        TIME_OFFSET_PRESENT                     = 0x01,
        CONCENTRATION_TYPE_LOCATION_PRESENT     = 0x02,
        CONCENTRATION_UNITS_MOL                 = 0x04,
        ANNUNCIATION_PRESENT                    = 0x08,
        CONTEXT_INFO_FOLLOWS                    = 0x10
    };
    void onConnect();
    void notifyCallback(void (*callback)(BleUuid, void*), void* context);
    uint8_t units() const;
    uint16_t sequence() const;
    int getConcentration(float& gl) const;
    GlucoseMeasurement(BleCharacteristic& ch): _characteristic(ch), _flags(GlucoseMeasurementFlags::NONE) {}
    ~GlucoseMeasurement() {};
protected:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    BleCharacteristic _characteristic;
    GlucoseMeasurementFlags _flags;
    DateTimeChar::DateTime dateTime;
    Annunciation _annunciation;
    uint16_t _sequence_number, _concentration;
    uint8_t _type_location;
    int16_t _time_offset;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
};

class GlucoseFeatureChar {
private:
    enum GlucoseFeatureFlags: uint8_t {
        BODY_MOVEMENT_DETECTION_SUPPORT         = 0x01,
        CUFF_FIT_DETECTION_SUPPORT              = 0x02,
        IRREGULAR_PULSE_DETECTION_SUPPORT       = 0x04,
        PULSE_RATE_RANGE_DETECTION_SUPPORT      = 0x08,
        MEASUREMENT_POSITION_DETECTION_SUPPORT  = 0x10,
        MULTIPLE_BOND_SUPPORT                   = 0x20
    };
    BleCharacteristic& _characteristic;
    GlucoseFeatureFlags _flags;
public:
    void onConnect() {_characteristic.getValue((uint8_t *)&_flags, 1);}

    bool isBodyMovementDetectionSupported() const {return _flags & GlucoseFeatureFlags::BODY_MOVEMENT_DETECTION_SUPPORT;}
    bool isCuffFitDetectionSupported() const {return _flags & GlucoseFeatureFlags::CUFF_FIT_DETECTION_SUPPORT;}
    bool isIrregularPulseDetectionSupported() const {return _flags & GlucoseFeatureFlags::IRREGULAR_PULSE_DETECTION_SUPPORT;}
    bool isPulseRateRangeDetectionSupported() const {return _flags & GlucoseFeatureFlags::PULSE_RATE_RANGE_DETECTION_SUPPORT;}
    bool isMeasurementPositionDetectionSupported() const {return _flags & GlucoseFeatureFlags::MEASUREMENT_POSITION_DETECTION_SUPPORT;}
    bool isMultipleBondSupported() const {return _flags & GlucoseFeatureFlags::MULTIPLE_BOND_SUPPORT;}
   
    GlucoseFeatureChar(BleCharacteristic& ch): _characteristic(ch) {};
    ~GlucoseFeatureChar() {};
};

class RecordAccessControlPoint {
private:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
        Log.info("RACP callback");
        RecordAccessControlPoint* ctx = (RecordAccessControlPoint *)context;
        ctx->responseCode = (len > 2) ? (OpCode)data[0] : OpCode::RESERVED_FUTURE_USE;
        if (ctx->responseCode == OpCode::RESPONSE_NUMBER_OF_RECORDS) {
            ctx->numberOfRecords = data[1] << 8 | data[2];
        }
        else if ( ctx->responseCode == OpCode::RESPONSE_CODE ) {
            ctx->requestCode = (OpCode)data[1];
            ctx->responseCodeValue = (ResponseCodeValues)data[2];
        }

        if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(ctx->_characteristic.UUID(), ctx->_notifyContext);
    }
    BleCharacteristic _characteristic;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
public:
    enum class OpCode: uint8_t {
        RESERVED_FUTURE_USE         = 0,
        REPORT_STORED_RECORDS       = 1,
        DELETE_STORED_RECORDS       = 2,
        ABORT_OPERATION             = 3,
        REPORT_NUMBER_OF_RECORDS    = 4,
        RESPONSE_NUMBER_OF_RECORDS  = 5,
        RESPONSE_CODE               = 6
    };
    enum class Operator: uint8_t {
        NO_OPERATOR                 = 0,
        ALL_RECORDS                 = 1,
        LESS_THAN_OR_EQUAL          = 2,
        GREATER_THAN_OR_EQUAL       = 3,
        WITHIN_INCLUSIVE_RANGE      = 4,
        FIRST_RECORD                = 5,
        LAST_RECORD                 = 6
    };
    enum class FilterType: uint8_t {
        RESERVED_FUTURE_USE         = 0,
        SEQUENCE_NUMBER             = 1,
        USER_FACING_TIME            = 2
    };
    enum class ResponseCodeValues: uint8_t {
        RESERVED_FUTURE_USE         = 0,
        SUCCESS                     = 1,
        OP_CODE_NOT_SUPPORTED       = 2,
        INVALID_OPERATOR            = 3,
        OPERATOR_NOT_SUPPORTED      = 4,
        INVALID_OPERAND             = 5,
        NO_RECORDS_FOUND            = 6,
        ABORT_UNSUCCESSFUL          = 7,
        PROCEDURE_NOT_COMPLETE      = 8,
        OPERAND_NOT_SUPPORTED       = 9
    };
    OpCode responseCode, requestCode;
    ResponseCodeValues responseCodeValue;
    uint16_t numberOfRecords;
    void onConnect() {
        _characteristic.onDataReceived(onDataReceived, this);
    }
    void notifyCallback(void (*callback)(BleUuid, void*), void* context) {
        _notifyNewData = callback;
        _notifyContext = context;
        _characteristic.subscribe(true);
    }
    int sendCommand(OpCode opCode, Operator oper = Operator::NO_OPERATOR, FilterType filterType = FilterType::RESERVED_FUTURE_USE, uint16_t operand = 0) {
        uint8_t buf[20];
        buf[0] = (uint8_t)opCode;
        switch (opCode)
        {
        case OpCode::REPORT_NUMBER_OF_RECORDS:
            switch (oper)
            {
            case Operator::ALL_RECORDS:
                buf[1] = (uint8_t)oper;
                return _characteristic.setValue(buf, 2);
                break;
            default:
                break;
            }
            break;
        case OpCode::REPORT_STORED_RECORDS:
            switch (oper)
            {
            case Operator::ALL_RECORDS:
                buf[1] = (uint8_t)oper;
                return _characteristic.setValue(buf, 2);
                break;
            
            default:
                break;
            }
        default:
            break;
        }
        return -1;
    }
    RecordAccessControlPoint(BleCharacteristic& ch): _characteristic(ch) {};
    ~RecordAccessControlPoint() {};
};