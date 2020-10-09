#include "ble_device.h"

#include "services/heart_rate_service.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

class HeartRateMonitor;

/**
 * The callback function format
 * 
 * monitor: instance of HeartRateMonitor
 * uuid: the UUID of the characteristic that sent a NOTIFY
 * context: the pointer that is passed when setNewValueCallback is called
 */
typedef void (*NewHeartRateCallback)(HeartRateMonitor& monitor, BleUuid uuid, void* context);

class HeartRateMonitor: public BleDevice {
private:
    std::unique_ptr<HeartRateService> hrService;
    std::unique_ptr<DeviceInformationService> disService;
    std::unique_ptr<BatteryService> battService;
    NewHeartRateCallback _callback;
    void* _callbackContext;
    static void _onNewValue(BleUuid, void*);
public:
    void onConnect();
    void loop() {};
    BleUuid getType() {return BleUuid(BLE_SIG_UUID_HEART_RATE_SVC);}

    /**
     * Register a callback that will be called when a new value is received. 
     * 
     * @param callback The callback function to be called
     * @param context An instance pointer, or NULL. Will be passed to the callback when called
     * 
     */
    void setNewValueCallback(NewHeartRateCallback callback, void* context);

    /**
     * Returns the last received value of the heart rate. This is updated when the heart
     * rate monitor notifies of a new value, usually once per second. Calling this does not
     * initiate a read on Bluetooth, since this characteristic is not readable, only notify.
     * 
     * Returns 0 if there's an error.
     */
    uint16_t getHeartRate() {return (hrService) ? hrService->getHeartRate() : 0;}
    /**
     * Returns the last received value of energy expended. This is updated when the heart
     * rate monitor notifies of a new value, usually once every 10 seconds. Calling this does not
     * initiate a read on Bluetooth, since this characteristic is not readable, only notify.
     */
    uint16_t getEnergyExpended() {return (hrService) ? hrService->getEnergyExpended() : 0;}
    BodySensorLocation::SensorLocation getSensorLocation() {return (hrService) ? hrService->getSensorLocation() : BodySensorLocation::SensorLocation::ERROR;}
    /**
     * Resets the energy expended counter back to zero.
     * 
     * Returns negative value if there's an error.
     */
    int resetEnergyExpended() {return (hrService) ? hrService->resetEnergyExpended() : -1;}

    /**
     * The following are used to access the values of the Device Information Service.
     * 
     * When called, they will start a BLE transaction to retrieve the value.
     * 
     * @param buf The buffer that will be filled the with the value
     * @param len The size of the buffer
     * 
     * @return The length of the value
     * @return Negative if there's an error
     */
    int getManufacturerName(uint8_t* buf, size_t len) { return (disService) ? disService->getManufacturerName(buf, len) : -1;}
    int getModelNumber(uint8_t* buf, size_t len) { return (disService) ? disService->getModelNumber(buf, len) : -1;}
    int getSerialNumber(uint8_t* buf, size_t len) { return (disService) ? disService->getSerialNumber(buf, len) : -1;}
    int getHardwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getHardwareRevision(buf, len) : -1;}
    int getFirmwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getFirmwareRevision(buf, len) : -1;}
    int getSoftwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getSoftwareRevision(buf, len) : -1;}

    /**
     * Returns the last updated battery level, which is usually notified when the value changes, or can
     * be manually updated.
     * 
     * @return the battery percentage from 0 to 100
     * @return negative if there's an error
     */
    int getBatteryLevel() {return (battService) ? battService->getBatteryLevel() : -1;}
    /**
     * Check if notification of the battery level is supported.
     */
    bool batterySupportsNotify() {return (battService) ? battService->supportsNotify() : false;}
    /**
     * Manually update the battery level. This will start a BLE transaction with the heart rate monitor.
     * 
     * @return the battery percentage from 0 to 100
     * @return negative if there's an error
     */
    int forceBatteryUpdate() {return (battService) ? battService->forceBatteryUpdate() : -1;}
    
    HeartRateMonitor(BleAddress addr): BleDevice{addr}, _callback(nullptr) {};
    ~HeartRateMonitor() {};
};