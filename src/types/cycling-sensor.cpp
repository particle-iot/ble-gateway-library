#include "cycling-sensor.h"

void CyclingSpeedAndCadence::onConnect()
{
    peer.discoverAllServices();
    for (auto& serv: peer.services()) {
        if (serv.UUID().type() == BleUuidType::SHORT) {
            switch (serv.UUID().shorted())
            {
            case BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC:
                cscService = std::make_unique<CyclingSpeedAndCadenceService>(serv);
                cscService->onConnect();
                cscService->setNewValueCallback(_onNewValue, this);
                break;
            /*case BLE_SIG_UUID_DEVICE_INFORMATION_SVC:
                disService = std::make_unique<DeviceInformationService>(serv);
                disService->onConnect();
                break;
            case BLE_SIG_UUID_BATTERY_SVC:
                battService = std::make_unique<BatteryService>(serv);
                battService->onConnect();
                battService->setNewValueCallback(_onNewValue, this);
                break; */
            default:
                break;
            }
        }
    }
}

void CyclingSpeedAndCadence::_onNewValue(BleUuid uuid, void* context) {
    CyclingSpeedAndCadence* ctx = (CyclingSpeedAndCadence *)context;
    if (ctx->_callback != nullptr) {
        ctx->_callback(*ctx, uuid, ctx->_callbackContext);
    }
}

void CyclingSpeedAndCadence::setNewValueCallback(NewCyclingCallback callback, void* context) {
    _callback = callback;
    _callbackContext = context;
}