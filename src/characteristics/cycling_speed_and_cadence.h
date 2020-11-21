
#define BLE_SIG_CSC_MEASUREMENT_CHAR 0x2A5B
#define BLE_SIG_CSC_FEATURE_CHAR 0x2A5C
#define BLE_SIG_SC_CONTROL_POINT 0x2A55

enum CyclingSpeedAndCadenceFlags: uint8_t {
    WHEEL_REVOLUTION_DATA_PRESENT       = 0x01,
    CRANK_REVOLUTION_DATA_PRESENT       = 0x02
};

class CyclingSpeedAndCadenceChar {
private:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
        CyclingSpeedAndCadenceChar* ctx = (CyclingSpeedAndCadenceChar *)context;
        size_t position = 0;
        if (len > 0) ctx->_flags = (CyclingSpeedAndCadenceFlags)data[position++];
        if ( (len > (position + 5)) && (ctx->_flags & CyclingSpeedAndCadenceFlags::WHEEL_REVOLUTION_DATA_PRESENT) ) {
            ctx->_wheel = data[position+3] << 24 | data[position+2] << 16 | data[position+1] << 8 | (data[position]);
            ctx->_last_wheel_event = data[position+5] << 8 | (data[position+4]);
            position += 6;
        }
        if ( (len > (position + 5)) && (ctx->_flags & CyclingSpeedAndCadenceFlags::CRANK_REVOLUTION_DATA_PRESENT) ) {
            ctx->_cadence = data[position+3] << 24 | data[position+2] << 16 | data[position+1] << 8 | (data[position]);
            ctx->_last_cadence_event = data[position+5] << 8 | (data[position+4]);
            position += 6;
        }
        if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(BleUuid(BLE_SIG_CSC_MEASUREMENT_CHAR), ctx->_notifyContext);
    }
    CyclingSpeedAndCadenceFlags _flags;
    uint32_t _wheel, _cadence;
    uint16_t _last_wheel_event, _last_cadence_event;
    BleCharacteristic _characteristic;
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;

public:
    void onConnect() {
        _characteristic.onDataReceived(onDataReceived, this);
        _characteristic.subscribe(true);
    };

    uint32_t getWheelRotations() const {return _wheel;}
    uint32_t getCadence() const {return _cadence;}
    uint16_t getLastWheelEvent() const {return _last_wheel_event;}
    uint16_t getLastCadenceEvent() const {return _last_cadence_event;}
    void notifyCallback(void (*callback)(BleUuid, void*), void* context) {
        _notifyNewData = callback;
        _notifyContext = context;
        }
    
    CyclingSpeedAndCadenceChar(BleCharacteristic& ch): _characteristic(ch), _notifyNewData(nullptr) {};
    ~CyclingSpeedAndCadenceChar() {};
};

class CSCFeatureChar {
private:
    enum CSCFeatureFlags: uint8_t {
        WHEEL_REVOLUTION_SUPPORTED          = 0x01,
        CRANK_REVOLUTION_SUPPORTED          = 0x02,
        MULTIPLE_SENSOR_LOCATION_SUPPORTED  = 0x04
    };
    BleCharacteristic _characteristic;
    CSCFeatureFlags _flags;
public:
    void onConnect() {_characteristic.getValue((uint8_t *)&_flags, 1);}

    bool isWheelRevolutionSupported() {return _flags & CSCFeatureFlags::WHEEL_REVOLUTION_SUPPORTED;}
    bool isCrankRevolutionSupported() {return _flags & CSCFeatureFlags::CRANK_REVOLUTION_SUPPORTED;}
    bool isMultipleSensorLocationSupported() {return _flags & CSCFeatureFlags::MULTIPLE_SENSOR_LOCATION_SUPPORTED;}

    CSCFeatureChar(BleCharacteristic& ch): _characteristic(ch) {};
    ~CSCFeatureChar() {};
};

class SensorLocation 
{
private:
    BleCharacteristic _characteristic;
public:
    enum Location: uint8_t {
        OTHER           = 0,
        TOP_OF_SHOE     = 1,
        IN_SHOE         = 2,
        HIP             = 3,
        FRONT_WHEEL     = 4,
        LEFT_CRANK      = 5,
        RIGHT_CRANK     = 6,
        LEFT_PEDAL      = 7,
        RIGHT_PEDAL     = 8,
        FRONT_HUB       = 9,
        REAR_DROPOUT    = 10,
        CHAINSTAY       = 11,
        REAR_WHEEL      = 12,
        REAR_HUB        = 13,
        CHEST           = 14,
        SPIDER          = 15,
        CHAIN_RING      = 16,
        ERROR           = 255
    };
    Location read() {
        uint8_t buf;
        return (_characteristic.getValue(&buf, 1) == 1) ? (Location)buf : Location::ERROR; 
    }
    SensorLocation(BleCharacteristic& ch) {_characteristic = ch;}
    ~SensorLocation() {}
};

class SCControlPoint 
{
private:
    BleCharacteristic _characteristic;
public:
    enum Response: uint8_t {
        SUCCESS                 = 1,
        OP_CODE_NOT_SUPPORTED   = 2,
        INVALID_PARAMETER       = 3,
        OPERATION_FAILED        = 4
    };
    Response setCumulativeValue(uint32_t value) {return (Response)_characteristic.setValue((uint8_t *)&value, 4);}

    SCControlPoint(BleCharacteristic& ch) {_characteristic = ch;}
    ~SCControlPoint() {}
};