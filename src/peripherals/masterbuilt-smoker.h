#pragma once

#include "ble_device.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

#define MASTERBUILT_SMOKER_SERVICE "426f7567-6854-6563-2d57-65694c69fff0"

class MasterbuiltSmoker;

typedef void (*NewSmokerValue)(MasterbuiltSmoker& smoker, void* context);

class MasterbuiltSmoker: public BleDevice
{
private:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    BleCharacteristic _dataChar, _controlChar;
    BleService _dataService;
    std::unique_ptr<DeviceInformationService> disService;
    std::unique_ptr<BatteryService> battService;
    uint16_t _probe_temp, _smoker_temp, _set_temp, _remaining_minutes, _set_minutes;
    bool _powerOn, _callbackOnChange;
    NewSmokerValue _callback;
    void* _callbackContext;
public:
    void onConnect();
    BleUuid getType() override {return BleUuid(MASTERBUILT_SMOKER_SERVICE);}
    static std::shared_ptr<BleDevice> bleDevicePtr(const BleScanResult* scanResult) {
        return std::make_shared<MasterbuiltSmoker>(scanResult->address());
    }

    void passkeyInput();
    /**
     * Get the last reported probe temperature
     */
    uint16_t getProbeTemp() {return _probe_temp;};
    /**
     * Get the last reported smoker temperature
     */
    uint16_t getSmokerTemp() {return _smoker_temp;};
    uint16_t getSetTemp() {return _set_temp;};
    uint16_t getRemainingTime() {return _remaining_minutes;};
    uint16_t getSetTime() {return _set_minutes;};
    bool setTempAndTime(uint16_t temp, uint16_t minutes);
    bool setSmokerTemp(uint16_t temp) { return setTempAndTime(temp, _remaining_minutes); };
    bool setSmokerTime(uint16_t minutes) { return setTempAndTime(_set_temp, minutes); };
    bool isPowerOn() { return _powerOn;};

    /**
     * Set a function to be called whenever there's a new value.
     * Note that a new value might be sent once per second.
     * 
     * @param callback function to call
     * @param context context pointer or pass NULL
     * @param onlyOnChange only trigger callback if the temperature value of either the smoker or the meat probe has changed
     */
    void setNewValueCallback(NewSmokerValue callback, void* context, bool onlyOnChange = false);
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

    MasterbuiltSmoker(BleAddress addr): BleDevice{addr}, _smoker_temp(225), _callback(nullptr) {};

    ~MasterbuiltSmoker() {};
};