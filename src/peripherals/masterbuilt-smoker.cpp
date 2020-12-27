#include "peripherals/masterbuilt-smoker.h"

const BleUuid notifyCharUuid("426f7567-6854-6563-2d57-65694c69fff4");
const BleUuid controlCharUuid("426f7567-6854-6563-2d57-65694c69fff3");

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
    BLE.startPairing(peer);
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
    Vector<BleCharacteristic> ch = peer.discoverCharacteristicsOfService(_dataService);
    peer.getCharacteristicByUUID(_dataService, _dataChar, notifyCharUuid);
    _dataChar.onDataReceived(onDataReceived, this);
    _dataChar.subscribe(true);
    peer.getCharacteristicByUUID(_dataService, _controlChar, controlCharUuid);
}

int MasterbuiltSmoker::passkeyInput(uint8_t* passkey)
{
    passkey = (uint8_t *)"000000";
    Log.info("Set pairing key to 000000");
    return 0;
}

 void MasterbuiltSmoker::setNewValueCallback(NewSmokerValue callback, void* context, bool onlyOnChange)
 {
     _callback = callback;
     _callbackContext = context;
     _callbackOnChange = onlyOnChange;
 }

 void MasterbuiltSmoker::loop()
 {

 }

bool MasterbuiltSmoker::setTempAndTime(uint16_t temp, uint16_t minutes)
{
    uint8_t buf[7] = {0xA1, 0x07, 0x05, (uint8_t)(minutes & 0xff), (uint8_t)(minutes >> 8), (uint8_t)(temp & 0xff), (uint8_t)(temp >> 8)};
    Log.info("Sending: %02X %02X %02X %02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
    return (_controlChar.setValue(buf, 7) == 7);
}

bool MasterbuiltSmoker::setSmokerTemp(uint16_t temp) 
{
    return setTempAndTime(temp, _remaining_minutes);
}

bool MasterbuiltSmoker::setSmokerTime(uint16_t time)
{
    return setTempAndTime(_set_temp, time);
}