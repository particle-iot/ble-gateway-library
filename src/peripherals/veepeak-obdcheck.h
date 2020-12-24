#pragma once

#include "ble_device.h"
#include "services/device_information_service.h"

#define VEEPEAK_SERVICE "49535343-fe7d-4ae5-8fa9-9fafd205e455"

class VeepeakObd;


class VeepeakObd: public BleDevice, public Stream
{
private:
    BleCharacteristic _notifyChar, _writeChar;
    BleService _dataService;
    std::unique_ptr<DeviceInformationService> disService;
    Vector<uint8_t> _input_vector, _output_vector;
    static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
public:
    void onConnect();
    BleUuid getType() override {return BleUuid(VEEPEAK_SERVICE);}
    static std::shared_ptr<BleDevice> bleDevicePtr(const BleScanResult* scanResult) {
        return std::make_shared<VeepeakObd>(scanResult->address());
    }

    int available() override {return _input_vector.size();};
    size_t write(uint8_t c) override;
    int read() override { return (_input_vector.size() > 0) ? (int)_input_vector.takeFirst() : -1; };
    int peek() override { return (_input_vector.size() > 0) ? (int)_input_vector.first() : -1; };
    void flush() override { _input_vector.clear(); };

    VeepeakObd(BleAddress addr): BleDevice{addr}, _input_vector(), _output_vector() {_input_vector.reserve(1024);_output_vector.reserve(128);};

    ~VeepeakObd() {};
};