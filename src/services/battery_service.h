#pragma once

#include "Particle.h"

#define BLE_SIG_BATTERY_LEVEL_CHAR 0x2A19
#define BLE_SIG_BATTERY_LEVEL_STATE_CHAR 0x2A1B
#define BLE_SIG_BATTERY_POWER_STATE_CHAR 0x2A1A

class BatteryService 
{
private:
    BleCharacteristic _batteryLevel;
    uint8_t _level;
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    void (*_notifyNewData)(BleUuid, void*);
    void* _notifyContext;
public:
    BleService service;
    void onConnect();
    int getBatteryLevel();
    int forceBatteryUpdate();
    bool supportsNotify();
    void setNewValueCallback(void (*callback)(BleUuid, void*), void* context);

    BatteryService(BleService serv): _level(0), _notifyNewData(nullptr), service(serv) {}
    ~BatteryService() {}
};