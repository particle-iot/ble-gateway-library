#include "ble_device.h"
#include "services/blood_pressure_service.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

class BloodPressureMonitor;

/**
 * The callback function format
 * 
 * monitor: instance of BloodPressureMonitor
 * uuid: the UUID of the characteristic that sent a NOTIFY/INDICATE
 * context: the pointer that is passed when setNewValueCallback is called
 */
typedef void (*NewBloodPressureCallback)(BloodPressureMonitor& monitor, BleUuid uuid, void* context);

class BloodPressureMonitor: public BleDevice
{
private:
    std::unique_ptr<BloodPressureService> _bpService;
    std::unique_ptr<DeviceInformationService> _disService;
    std::unique_ptr<BatteryService> _battService;
    NewBloodPressureCallback _callback;
    void* _callbackContext;
    static void _onNewValue(BleUuid, void*);
public:
    void onConnect();
    void loop() {};
    BleUuid getType() final {return BleUuid(BLE_SIG_UUID_BLOOD_PRESSURE_SVC);}
    /**
     *  Access data as last reported by the device 
     */
    int getSystolic(float& bp) const {return (_bpService) ? _bpService->getSystolic(bp) : -1;}
    int getDiastolic(float& bp) const {return (_bpService) ? _bpService->getDiastolic(bp) : -1;}
    int getMeanArterial(float& bp) const {return (_bpService) ? _bpService->getMeanArterial(bp) : -1;}
    int getIntermediateCuffPressure(float& bp) const {return (_bpService) ? _bpService->getIntermediateCuffPressure(bp) : -1;}

    /**
     * Register a callback that will be called when a new value is received. 
     * 
     * @param callback The callback function to be called
     * @param context An instance pointer, or NULL. Will be passed to the callback when called
     * 
     */
    void setNewValueCallback(NewBloodPressureCallback callback, void* context);
    
    BloodPressureMonitor(BleAddress addr):  BleDevice{addr} {};
    ~BloodPressureMonitor() {};
};