#pragma once

#include "Particle.h"
#include "characteristics/gap_generic.h"

class GapGenericService 
{
private:
    std::unique_ptr<DeviceNameCharacteristic> _deviceName;
    std::unique_ptr<Appearance> _appearance;
    std::unique_ptr<PeripheralPreferredConnectionParams> _peripheralConnParams;
    BlePeerDevice _peer;
public:
    int getDeviceName(uint8_t* buf, size_t len) {return (_deviceName) ? _deviceName->getValue(buf, len) : -1;}
    int setDeviceName(const char* str) {return (_deviceName) ? _deviceName->setValue(str) : -1;}
    Appearance::Value getAppearance() {return (_appearance) ? _appearance->getValue() : Appearance::Value::NONE;}
    int setAppearance(Appearance::Value appearance) {return (_appearance) ? _appearance->setValue(appearance) : -1;}
    BleService service;
    void onConnect();

    GapGenericService(BleService& serv, BlePeerDevice& peer): _peer(peer), service(serv) {};
};