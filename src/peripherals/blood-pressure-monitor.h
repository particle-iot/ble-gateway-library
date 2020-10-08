#include "ble_device.h"
#include "services/blood_pressure.h"


class BloodPressureMonitor: public BleDevice
{
private:
    std::unique_ptr<BloodPressureService> bpService;
public:
    void onConnect();
    void loop() {};
    BleUuid getType() final {return BleUuid(BLE_SIG_UUID_BLOOD_PRESSURE_SVC);}
    
    BloodPressureMonitor(BleAddress addr):  BleDevice{addr} {};
    ~BloodPressureMonitor() {};
};