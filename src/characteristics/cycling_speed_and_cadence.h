
#define BLE_SIG_CSC_MEASUREMENT_CHAR 0x2A5B

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
    };

    uint32_t getSpeed() {return _wheel;}
    uint32_t getCadence() {return _cadence;}
    uint16_t getLastWheelEvent() {return _last_wheel_event;}
    uint16_t getLastCadenceEvent() {return _last_cadence_event;}
    int enableNotification() {return _characteristic.subscribe(true);}
    void notifyCallback(void (*callback)(BleUuid, void*), void* context) {
        _notifyNewData = callback;
        _notifyContext = context;
        }
    
    CyclingSpeedAndCadenceChar(BleCharacteristic ch): _characteristic(ch), _notifyNewData(nullptr) {};
    ~CyclingSpeedAndCadenceChar() {};
};