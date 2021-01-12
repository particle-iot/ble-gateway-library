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
    };
    
    void onConnect() override;
    void loop() override;
    bool pendingData() override;
    BleUuid getType() final {return BleUuid(BLE_SIG_UUID_GLUCOSE_SVC);}
    static std::shared_ptr<BleDevice> bleDevicePtr(const BleScanResult* scanResult) {
        return std::make_shared<GlucoseMonitor>(scanResult->address());
    }
    /**
     *  Access data as last reported by the device 
     */
    int getNumberStoredRecords(uint16_t timeout_ms = 3000);
    Vector<Measurement>& getMeasurements(uint16_t timeout_ms = 3000);
    void flushBuffer();
    Vector<Measurement>& getBufferedMeasurements();
    int getBatteryLevel();
    /**
     * Register a callback that will be called when a new value is received. 
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
};