#include "characteristic.h"

const BleUuid org_bluetooth_characteristic_blood_pressure_measurement(0x2A35);

class BloodPressureMeasurement: public GatewayBleCharacteristic
{
private:

public:
    BleUuid getUuid() {return org_bluetooth_characteristic_blood_pressure_measurement;};
};