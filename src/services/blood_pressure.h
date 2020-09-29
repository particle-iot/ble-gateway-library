#pragma once

#include "Particle.h"

const BleUuid ORG_BLUETOOTH_SERVICE_BLOOD_PRESSURE(0x1810);

class BloodPressureService : public BleService
{
private:
public:
    static const BleUuid* getUuid() {return &ORG_BLUETOOTH_SERVICE_BLOOD_PRESSURE;};

    BloodPressureService() {};
    BloodPressureService(BleService& service) {BloodPressureService();};
};