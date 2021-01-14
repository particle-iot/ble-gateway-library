#include "ble_device.h"
#include "services/glucose_service.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

class GlucoseMonitor;

/**
 * The callback function format
 * 
 * monitor: instance of GlucoseMonitor
 * uuid: the UUID of the characteristic that sent a NOTIFY/INDICATE
 * context: the pointer that is passed when setNewValueCallback is called
 */
typedef void (*NewGlucoseCallback)(GlucoseMonitor& monitor, BleUuid uuid, void* context);

class GlucoseMonitor: public BleDevice
{
public:
    struct Measurement
    {
        float concentration;
        bool mol_per_l;
        uint16_t sequence;
        DateTimeChar::DateTime time;
        GlucoseMeasurement::Type type;
        GlucoseMeasurement::Location location;
    };
    
    void onConnect() override;
    void loop() override;
    bool pendingData() override;
    BleUuid getType() final {return BleUuid(BLE_SIG_UUID_GLUCOSE_SVC);}
    static std::shared_ptr<BleDevice> bleDevicePtr(const BleScanResult* scanResult) {
        return std::make_shared<GlucoseMonitor>(scanResult->address());
    }
    /**
     * Blocking call to request and return the number of stored records. Since it
     * blocks until it receives the response, there is no callback.
     * 
     * @return Number of stored records. Negative if there's an error (like timeout)
     */
    int getNumberStoredRecords(uint16_t timeout_ms = 3000);
    /**
     * Blocking call to request and return Glucose measurements from the device.
     * Since it blocks until receives the response, there is no callback.
     * 
     * Note that if there's an error, the return value will be the existing vector
     * of measurements, which will be empty unless a previous request to get the
     * measurements was successful and flushBuffer() has not been called.
     * 
     * IMPORTANT: make sure to call flushBuffer() after gathering the data,
     * otherwise it won't reconnect to the same device.
     */
    Vector<Measurement>& getMeasurements(uint16_t timeout_ms = 3000);
    Vector<Measurement>& getMeasurements(RecordAccessControlPoint::Operator oper, uint16_t min, uint16_t timeout_ms = 3000);
    void flushBuffer();
    /**
     * Returns the glucose measurements that were previously received. No new request
     * goes out to the device.
     */
    Vector<Measurement>& getBufferedMeasurements();
    /**
     * @return last battery value sent by device
     */
    int getBatteryLevel();
    /**
     * Register a callback that will be called when a new value is received. Only the
     * Battery Service will send callbacks.
     * 
     * @param callback The callback function to be called
     * @param context An instance pointer, or NULL. Will be passed to the callback when called
     * 
     */
    void setNewValueCallback(NewGlucoseCallback callback, void* context);
    
    GlucoseMonitor(BleAddress addr);
    ~GlucoseMonitor();
private:
    std::unique_ptr<GlucoseService> _gService;
    std::unique_ptr<DeviceInformationService> _disService;
    std::unique_ptr<BatteryService> _battService;
    NewGlucoseCallback _callback;
    Vector<Measurement> glValues;
    os_semaphore_t _blockSemaphore;
    void* _callbackContext;
    static void _onNewValue(BleUuid, void*);
    int requestMeasurements(uint16_t timeout_ms = 3000, 
            RecordAccessControlPoint::Operator oper = RecordAccessControlPoint::Operator::NO_OPERATOR, 
            RecordAccessControlPoint::FilterType filterType = RecordAccessControlPoint::FilterType::RESERVED_FUTURE_USE,
            uint8_t* operand = nullptr, uint8_t operand_size = 0);
};