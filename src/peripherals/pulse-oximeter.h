#pragma once

#include "ble_device.h"

#define JUMPER_PULSEOX_SERVICE "cdeacb80-5235-4c07-8846-93a37ee6b86d"

class PulseOxAlert;
class PulseOx;

enum class PulseOxAlertType {
    SPO_LOW,
    HR_LOW_HIGH,
    NONE
};

typedef void (*PulseOxAlertCallback)(PulseOx& device, PulseOxAlertType type);

class PulseOxAlert
{
public:
    uint8_t min, max;
    uint16_t period;
    bool cleared;
    int last_published;
    PulseOxAlertType type;
    PulseOxAlertCallback callback;
    PulseOxAlert(): callback(nullptr) {};
};

class PulseOx: public BleDevice
{
private:
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
    uint8_t _elapsed;
    BleCharacteristic dataChar;
    BleService dataService;
    uint8_t _hr, _spo;
    Vector<PulseOxAlert> _alerts;

public:
    void onConnect();
    void loop();
    BleUuid getType() override {return BleUuid(JUMPER_PULSEOX_SERVICE);}

    uint8_t getSpo() { return _spo;};
    uint8_t getHr()  { return _hr;};

    int setAlert(PulseOxAlertType type, PulseOxAlertCallback callback, uint16_t period, uint8_t low = 0, uint8_t high = 0);

    PulseOx(BleAddress addr): BleDevice{addr}, _elapsed(System.uptime()), _hr(0), _spo(0) {};
    PulseOx(BleDevice& dev) {PulseOx(dev.address);};

    ~PulseOx() {};
};