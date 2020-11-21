#include "ble_device.h"

#include "services/cycling_speed_and_cadence.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

class CyclingSpeedAndCadence;

/**
 * The callback function format
 * 
 * monitor: instance of HeartRateMonitor
 * uuid: the UUID of the characteristic that sent a NOTIFY
 * context: the pointer that is passed when setNewValueCallback is called
 */
typedef void (*NewCyclingCallback)(CyclingSpeedAndCadence& monitor, BleUuid uuid, void* context);

class CyclingSpeedAndCadence: public BleDevice {
private:
    std::unique_ptr<CyclingSpeedAndCadenceService> cscService;
    std::unique_ptr<DeviceInformationService> disService;
    std::unique_ptr<BatteryService> battService;
    NewCyclingCallback _callback;
    void* _callbackContext;
    uint16_t _wheelmm, _speed, timed_event;
    uint32_t wheel_rev;
    static void _onNewValue(BleUuid, void*);
public:
    void onConnect();
    void loop() {};
    BleUuid getType() {return BleUuid(BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC);}

    /**
     * Register a callback that will be called when a new value is received. 
     * 
     * @param callback The callback function to be called
     * @param context An instance pointer, or NULL. Will be passed to the callback when called
     * 
     */
    void setNewValueCallback(NewCyclingCallback callback, void* context);

    // Configure wheel size in mm
    void setWheelSize(uint16_t mm) {_wheelmm = mm;}
    uint16_t getWheelSize() {return _wheelmm;}

    /**
     * Gets the calculated speed based on the last updated wheel rotation event, which is 
     * normally notified once per second. Calling this does not initiate a read on Bluetooth.
     * 
     * @return speed in meters per hour
     */
    uint16_t getSpeed() {return _speed;}

    /**
     * Returns the last received value of the wheel rotations, cadence, last wheel event and
     * last cadence event. This is updated when the sensor notifies of a new value, usually 
     * once per second. Calling this does not initiate a read on Bluetooth.
     */
    uint32_t getWheelRotations() {return (cscService) ? cscService->getWheelRotations() : 0;}
    uint32_t getCadence() {return (cscService) ? cscService->getCadence() : 0;}
    uint16_t getLastWheelEvent() {return (cscService) ? cscService->getLastWheelEvent() : 0;}
    uint16_t getLastCadenceEvent() {return (cscService) ? cscService->getLastCadenceEvent() : 0;}
    //SensorLocation getSensorLocation() {return (hrService) ? hrService->getSensorLocation() : SensorLocation::ERROR;}
    //int resetEnergyExpended() {return (hrService) ? hrService->resetEnergyExpended() : -1;}

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
    
    CyclingSpeedAndCadence(BleAddress addr): BleDevice{addr}, _callback(nullptr), _wheelmm(0), _speed(0), timed_event(0), wheel_rev(0) {};
    ~CyclingSpeedAndCadence() {};
};