#include "peripherals/veepeak-obdcheck.h"

#define DATA_SERVICE_UUID 0xFFF0
#define NOTIFY_CHAR 0xFFF1
#define WRITE_CHAR 0xFFF2

const BleUuid dataCharUuid(0xFFF0);

void VeepeakObd::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
    VeepeakObd* ctx = (VeepeakObd *)context;
    if ( (ctx->_input_vector.capacity() - ctx->_input_vector.size()) > 0) {
        ctx->_input_vector.append(data, std::min((int)len, ctx->_input_vector.capacity() - ctx->_input_vector.size()));
    }
}

size_t VeepeakObd::write(uint8_t c)
{
    if (_output_vector.capacity() - _output_vector.size() > 0) {
        _output_vector.append(c);
    } else {
        return -1;
    }
    if (c == '\r' || (_output_vector.capacity() == _output_vector.size()) ) {
        int num = _writeChar.setValue(_output_vector.data(), _output_vector.size());
        if (num > 0) {
            _output_vector.removeAt(0, num);
        }
    }
    return 1;
}

void VeepeakObd::onConnect()
{
    peer.discoverAllServices();
    for (auto& serv: peer.services()) {
        if (serv.UUID().type() == BleUuidType::SHORT) {
            switch (serv.UUID().shorted())
            {
            case BLE_SIG_UUID_GENERIC_ACCESS_SVC:
                _gapService = std::make_unique<GapGenericService>(serv, peer);
                _gapService->onConnect();
                break;
            case BLE_SIG_UUID_DEVICE_INFORMATION_SVC:
                disService = std::make_unique<DeviceInformationService>(serv, peer);
                disService->onConnect();
                break;
            default:
                break;
            }
        }
    }
    peer.getServiceByUUID(_dataService, BleUuid(DATA_SERVICE_UUID));
    peer.discoverCharacteristicsOfService(_dataService);
    peer.getCharacteristicByUUID(_dataService, _notifyChar, BleUuid(NOTIFY_CHAR));
    _notifyChar.onDataReceived(onDataReceived, this);
    _notifyChar.subscribe(true);
    peer.getCharacteristicByUUID(_dataService, _writeChar, BleUuid(WRITE_CHAR));
}