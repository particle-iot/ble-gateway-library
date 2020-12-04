#pragma once

#include "ble_device.h"
#include "services/device_information_service.h"
#include "services/battery_service.h"

#define MASTERBUILT_SMOKER_SERVICE "426f7567-6854-6563-2d57-65694c69fff0"

class MasterbuiltSmoker;

typedef void (*NewSmokerValue)(MasterbuiltSmoker& smoker, BleUuid uuid, void* context);
typedef void (*ChangedSmokerValue)(MasterbuiltSmoker& smoker, void* context);

class MasterbuiltSmoker: public BleDevice
{
private:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    BleCharacteristic _dataChar, _controlChar;
    BleService _dataService;
    std::unique_ptr<DeviceInformationService> disService;
    std::unique_ptr<BatteryService> battService;
    uint16_t _probe_temp, _smoker_temp, _set_temp, _remaining_minutes, _set_minutes;
    NewSmokerValue _callback;
    ChangedSmokerValue _changedCallback;
    void* _callbackContext;
    void* _changedCallbackContext;
public:
    void onConnect();
    void loop();
    BleUuid getType() override {return BleUuid(MASTERBUILT_SMOKER_SERVICE);}

    int passkeyInput(uint8_t* passkey);
    uint16_t getProbeTemp() {return _probe_temp;};
    uint16_t getSmokerTemp() {return _smoker_temp;};
    bool setSmokerTemp(uint16_t temp);

    void setNewValueCallback(NewSmokerValue callback, void* context);
    void setChangedValueCallback(ChangedSmokerValue callback, void* context);

    MasterbuiltSmoker(BleAddress addr): BleDevice{addr}, _callback(nullptr), _changedCallback(nullptr) {};

    ~MasterbuiltSmoker() {};
};