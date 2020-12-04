#include "peripherals/masterbuilt-smoker.h"

const BleUuid notifyCharUuid("426f7567-6854-6563-2d57-65694c69fff4");
const BleUuid controlCharUuid("426f7567-6854-6563-2d57-65694c69fff3");

void MasterbuiltSmoker::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
    MasterbuiltSmoker* ctx = (MasterbuiltSmoker *)context;
    bool changed = false;
    if (len > 14) {
        if ( ( (data[5] << 8 | data[4]) != ctx->_smoker_temp) || ( (data[7] << 8 | data[6]) != ctx->_probe_temp)  ) changed = true;
        ctx->_smoker_temp = data[5] << 8 | data[4];
        ctx->_probe_temp = data[7] << 8 | data[6];
        ctx->_remaining_minutes = data[9] << 8 | data[8];
        ctx->_set_minutes = data[12] << 8 | data[11];
        ctx->_set_temp = data[14] << 8 | data[13];
    }
    if (ctx->_callback != nullptr) ctx->_callback(*ctx, ctx->_dataChar.UUID(), ctx->_callbackContext);
    if (changed && ctx->_changedCallback != nullptr) ctx->_changedCallback(*ctx, ctx->_changedCallbackContext);
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

 void MasterbuiltSmoker::setNewValueCallback(NewSmokerValue callback, void* context)
 {
     _callback = callback;
     _callbackContext = context;
 }

 void MasterbuiltSmoker::setChangedValueCallback(ChangedSmokerValue callback, void* context)
 {
     _changedCallback = callback;
     _changedCallbackContext = context;
 }

 void MasterbuiltSmoker::loop()
 {

 }

 bool MasterbuiltSmoker::setSmokerTemp(uint16_t temp) 
 {
     //TODO: Write to the smoker with the new temp
     return false;
 }