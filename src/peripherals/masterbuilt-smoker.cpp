#include "peripherals/masterbuilt-smoker.h"

const BleUuid notifyCharUuid("426f7567-6854-6563-2d57-65694c69fff4");
const BleUuid controlCharUuid("426f7567-6854-6563-2d57-65694c69fff3");
const BleUuid startwriteCharUuid("426f7567-6854-6563-2d57-65694c69fff1");

void MasterbuiltSmoker::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
    MasterbuiltSmoker* ctx = (MasterbuiltSmoker *)context;
    bool changed = false;
    if (len > 14) {
        if ( ( (data[5] << 8 | data[4]) != ctx->_smoker_temp) || ( (data[7] << 8 | data[6]) != ctx->_probe_temp)  ) changed = true;
        ctx->_powerOn = true;
        ctx->_smoker_temp = data[5] << 8 | data[4];
        ctx->_probe_temp = data[7] << 8 | data[6];
        ctx->_remaining_minutes = data[9] << 8 | data[8];
        ctx->_set_minutes = data[12] << 8 | data[11];
        ctx->_set_temp = data[14] << 8 | data[13];
    } else if (len > 3) {
        changed = ctx->_powerOn;
        if ( data[0] == 0xb2 && data[1] == 0 && data[2] == 0 && data[3] == 0) ctx->_powerOn = false;
        changed = changed != ctx->_powerOn;
    }
    if ( ctx->_callback != nullptr && ( !ctx->_callbackOnChange || changed )) ctx->_callback(*ctx, ctx->_callbackContext);
}

void MasterbuiltSmoker::onConnect()
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
            case BLE_SIG_UUID_BATTERY_SVC:
                battService = std::make_unique<BatteryService>(serv, peer);
                battService->onConnect();
                break; 
            default:
                break;
            }
        }
    }
    peer.getServiceByUUID(_dataService, BleUuid(MASTERBUILT_SMOKER_SERVICE));
    peer.discoverCharacteristicsOfService(_dataService);
    BleCharacteristic writeChar;
    // The smoker requires that this value be written first to maintain connection
    peer.getCharacteristicByUUID(_dataService, writeChar, startwriteCharUuid);
    uint8_t buf[10] = {0x0e, 0x08, 0x0e, 0x1c, 0x9b, 0x02, 0x7e, 0x09, 0xba, 0x3b};
    writeChar.setValue(buf, 10);
    peer.getCharacteristicByUUID(_dataService, _dataChar, notifyCharUuid);
    _dataChar.onDataReceived(onDataReceived, this);
    _dataChar.subscribe(true);
    peer.getCharacteristicByUUID(_dataService, _controlChar, controlCharUuid);
}

void MasterbuiltSmoker::passkeyInput()
{
    // Not always required, but if it is, the passkey is 000000
    BLE.setPairingPasskey(peer, (uint8_t *)"000000");
}

 void MasterbuiltSmoker::setNewValueCallback(NewSmokerValue callback, void* context, bool onlyOnChange)
 {
     _callback = callback;
     _callbackContext = context;
     _callbackOnChange = onlyOnChange;
 }

bool MasterbuiltSmoker::setTempAndTime(uint16_t temp, uint16_t minutes)
{
    uint8_t buf[7] = {0xA1, 0x07, 0x05, (uint8_t)(minutes & 0xff), (uint8_t)(minutes >> 8), (uint8_t)(temp & 0xff), (uint8_t)(temp >> 8)};
    return (_controlChar.setValue(buf, 7) == 7);
}