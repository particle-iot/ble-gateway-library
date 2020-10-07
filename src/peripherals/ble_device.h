#pragma once

#include "Particle.h"


class BleDevice
{
private:
public:
    BleAddress address;
    BlePeerDevice peer;

    virtual BleUuid getType() = 0;

    virtual void loop() {};
    virtual void onConnect() = 0;

    BleDevice(BleAddress addr) :
         address(addr) {};
    
    BleDevice() {};
};