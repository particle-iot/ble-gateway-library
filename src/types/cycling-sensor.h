#include "ble_device.h"

#include "services/cycling_speed_and_cadence.h"
//#include "services/device_information_service.h"
//#include "services/battery_service.h"

class CyclingSpeedAndCadence;

typedef void (*NewCyclingCallback)(CyclingSpeedAndCadence& monitor, BleUuid uuid, void* context);

class CyclingSpeedAndCadence: public BleDevice {
private:
    std::unique_ptr<CyclingSpeedAndCadenceService> cscService;
    //std::unique_ptr<DeviceInformationService> disService;
    //std::unique_ptr<BatteryService> battService;
    NewCyclingCallback _callback;
    void* _callbackContext;
    static void _onNewValue(BleUuid, void*);
public:
    void onConnect();
    void loop() {};
    BleUuid getType() {return BleUuid(BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC);}

    void setNewValueCallback(NewCyclingCallback callback, void* context);

    // Data from Cycling Speed and Cadence Service
    uint32_t getSpeed() {return (cscService) ? cscService->getSpeed() : 0;}
    uint32_t getCadence() {return (cscService) ? cscService->getCadence() : 0;}
    uint16_t getLastWheelEvent() {return (cscService) ? cscService->getLastWheelEvent() : 0;}
    uint16_t getLastCadenceEvent() {return (cscService) ? cscService->getLastCadenceEvent() : 0;}
    //SensorLocation getSensorLocation() {return (hrService) ? hrService->getSensorLocation() : SensorLocation::ERROR;}
    //int resetEnergyExpended() {return (hrService) ? hrService->resetEnergyExpended() : -1;}

    // Data from Device Information Service
    //int getManufacturerName(uint8_t* buf, size_t len) { return (disService) ? disService->getManufacturerName(buf, len) : -1;}
    //int getModelNumber(uint8_t* buf, size_t len) { return (disService) ? disService->getModelNumber(buf, len) : -1;}
    //int getSerialNumber(uint8_t* buf, size_t len) { return (disService) ? disService->getSerialNumber(buf, len) : -1;}
    //int getHardwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getHardwareRevision(buf, len) : -1;}
    //int getFirmwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getFirmwareRevision(buf, len) : -1;}
    //int getSoftwareRevision(uint8_t* buf, size_t len) { return (disService) ? disService->getSoftwareRevision(buf, len) : -1;}

    // Data from Battery Service
    //int getBatteryLevel() {return (battService) ? battService->getBatteryLevel() : -1;}
    //bool batterySupportsNotify() {return (battService) ? battService->supportsNotify() : false;}
    
    CyclingSpeedAndCadence(BleAddress addr): BleDevice{addr}, _callback(nullptr) {};
    ~CyclingSpeedAndCadence() {};
};