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
    enum Flags: uint8_t {
        NONE                                    = 0,
        TIME_OFFSET_PRESENT                     = 0x01,
        CONCENTRATION_TYPE_LOCATION_PRESENT     = 0x02,
        CONCENTRATION_UNITS_MOL                 = 0x04,
        ANNUNCIATION_PRESENT                    = 0x08,
        CONTEXT_INFO_FOLLOWS                    = 0x10
    };
    enum Type: uint8_t {
        TYPE_RESERVED_FUTURE_USE                = 0,
        CAPILLARY_WHOLE_BLOOD                   = 1,
        CAPILLARY_PLASMA                        = 2,
        VENOUS_WHOLE_BLOOD                      = 3,
        VENOUS_PLASMA                           = 4,
        ARTERIAL_WHOLE_BLOOD                    = 5,
        ARTERIAL_PLASMA                         = 6,
        UNDETERMINED_WHOLE_BLOOD                = 7,
        UNDETERMINED_PLASMA                     = 8,
        INTERSTITIAL_FLUID                      = 9,
        TYPE_CONTROL_SOLUTION                   = 10
    };
    enum Location: uint8_t {
        LOC_RESERVED_FUTURE_USE                 = 0,
        FINGER                                  = 1,
        ALTERNATE_SITE_TEST                     = 2,
        EARLOBE                                 = 3,
        LOC_CONTROL_SOLUTION                    = 4,
        LOCATION_NOT_AVAILABLE                  = 15
    };
    void onConnect();
    void notifyCallback(void (*callback)(BleUuid, void*), void* context);
    uint8_t units() const;
    uint16_t sequence() const;
    int getConcentration(float& gl) const;
    DateTimeChar::DateTime getTime() const;
    Type getType() const;
    Location getLocation() const;
    GlucoseMeasurement(BleCharacteristic& ch): _characteristic(ch), _flags(Flags::NONE) {}
    ~GlucoseMeasurement() {};
protected:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    BleCharacteristic _characteristic;
    Flags _flags;
    DateTimeChar::DateTime dateTime;
    Annunciation _annunciation;
    uint16_t _sequence_number, _concentration;
    uint8_t _type_location;
    int16_t _time_offset;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
};

class GlucoseMeasurementContext {
public:
    enum Flags: uint8_t {
        NONE                                = 0,
        CARBOHYDRATE_ID_PRESENT             = 0x01,
        MEAL_PRESENT                        = 0x02,
        TESTER_HEALTH_PRESENT               = 0x04,
        EXERCISE_DURATION_AND_INTENSITY     = 0x08,
        MEDICATION_ID_PRESENT               = 0x10,
        MEDICATION_VALUE_UNITS              = 0x20,
        HBA1C_PRESENT                       = 0x40,
        EXTENDED_FLAGS_PRESENT              = 0x80
    };
    enum ExtendedFlags: uint8_t {
        NONE_EXTENDED                       = 0
    };
    enum class CarbID: uint8_t {
        RESERVED_FUTURE_USE                 = 0,
        BREAKFAST                           = 1,
        LUNCH                               = 2,
        DINNER                              = 3,
        SNACK                               = 4,
        DRINK                               = 5,
        SUPPER                              = 6,
        BRUNCH                              = 7
    };
    enum class Meal: uint8_t {
        RESERVED_FUTURE_USE                 = 0,
        BEFORE_MEAL                         = 1,
        AFTER_MEAL                          = 2,
        FASTING                             = 3,
        CASUAL                              = 4,
        BEDTIME                             = 5
    };
    enum Tester: uint8_t {
        TESTER_RESERVED                     = 0,
        SELF                                = 1,
        HEALTH_CARE_PROFESSIONAL            = 2,
        LAB_TEST                            = 3,
        TESTER_VALUE_NOT_AVAILABLE          = 4
    };
    enum Health: uint8_t {
        HEALTH_RESERVED                     = 0,
        MINOR_HEALTH_ISSUES                 = 1,
        MAJOR_HEALTH_ISSUES                 = 2,
        DURING_MENSES                       = 3,
        UNDER_STRESS                        = 4,
        NO_HEALTH_ISSUES                    = 5,
        HEALTH_VALUE_NOT_AVAILABLE          = 6
    };
    enum class MedicationID: uint8_t {
        RESERVED_FUTURE_USE                 = 0,
        RAPID_ACTING_INSULIN                = 1,
        SHORT_ACTING_INSULIN                = 2,
        INTERMEDIATE_ACTING_INSULIN         = 3,
        LONG_ACTING_INSULIN                 = 4,
        PRE_MIXED_INSULIN                   = 5
    };
    void onConnect();
    void notifyCallback(void (*callback)(BleUuid, void*), void* context);
    GlucoseMeasurementContext(BleCharacteristic& ch): _characteristic(ch), _flags(Flags::NONE) {};
    ~GlucoseMeasurementContext() {};
protected:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    BleCharacteristic& _characteristic;
    Flags _flags;
    ExtendedFlags _extendedFlags;
    uint16_t _sequence_number, _carbKG, _exercise_duration, _medicationAmount, _hba1c;
    uint8_t _tester_health, _exercise_intensity;
    CarbID _carbID;
    Meal _meal;
    MedicationID _medicationID;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
};

class GlucoseFeatureChar {
private:
    BleCharacteristic& _characteristic;
public:
    //void onConnect() {_characteristic.getValue((uint8_t *)&_flags, 1);}

    GlucoseFeatureChar(BleCharacteristic& ch): _characteristic(ch) {};
    ~GlucoseFeatureChar() {};
};

class RecordAccessControlPoint {
private:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
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
    int sendCommand(OpCode opCode, Operator oper = Operator::NO_OPERATOR, FilterType filterType = FilterType::RESERVED_FUTURE_USE, uint8_t* operand = nullptr, uint8_t operand_size = 0) {
        uint8_t buf[3+operand_size];
        buf[0] = (uint8_t)opCode;
        buf[1] = (uint8_t)oper;
        switch (oper)
        {
        case Operator::ALL_RECORDS:
        case Operator::FIRST_RECORD:
        case Operator::LAST_RECORD:
        case Operator::NO_OPERATOR:
            return _characteristic.setValue(buf, 2);
            break;
        case Operator::GREATER_THAN_OR_EQUAL:
        case Operator::LESS_THAN_OR_EQUAL:
        case Operator::WITHIN_INCLUSIVE_RANGE:
            buf[2] = (uint8_t)filterType;
            memcpy(buf+3, operand, operand_size);
            return _characteristic.setValue(buf, sizeof(buf));
            break;
        default:
            break;
        }
        return -1;
    }
    RecordAccessControlPoint(BleCharacteristic& ch): _characteristic(ch) {};
    ~RecordAccessControlPoint() {};
};