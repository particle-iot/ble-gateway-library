#include "ble_device.h"

#include "services/heart_rate_service.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

class HeartRateMonitor;

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

    void setNewValueCallback(NewHeartRateCallback callback, void* context);

    // Data from Heart Rate Service
    uint16_t getHeartRate() {return (hrService) ? hrService->getHeartRate() : 0;}
    uint16_t getEnergyExpended() {return (hrService) ? hrService->getEnergyExpended() : 0;}
    SensorLocation getSensorLocation() {return (hrService) ? hrService->getSensorLocation() : SensorLocation::ERROR;}
    int resetEnergyExpended() {return (hrService) ? hrService->resetEnergyExpended() : -1;}

    // Data from Device Information Service
    int getManufacturerName(uint8_t* buf, size_t len) { return (disService) ? disService->getManufacturerName(buf, len) : -1;}
    int getModelNumber(uint8_t* buf, size_t len) { return (disService) ? disService->getModelNumber(buf, len) : -1;}
    int getSerialNumber(uint8_t* buf, size_t len) { return (disService) ? disService->getSerialNumber(buf, len) : -1;}
    int getHardwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getHardwareRevision(buf, len) : -1;}
    int getFirmwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getFirmwareRevision(buf, len) : -1;}
    int getSoftwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getSoftwareRevision(buf, len) : -1;}

    // Data from Battery Service
    int getBatteryLevel() {return (battService) ? battService->getBatteryLevel() : -1;}
    bool batterySupportsNotify() {return (battService) ? battService->supportsNotify() : false;}
    
    HeartRateMonitor(BleAddress addr): BleDevice{addr}, _callback(nullptr) {};
    ~HeartRateMonitor() {};
};