#include "heart-rate-monitor.h"

void HeartRateMonitor::onConnect()
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
            case BLE_SIG_UUID_HEART_RATE_SVC:
                hrService = std::make_unique<HeartRateService>(serv, peer);
                hrService->onConnect();
                hrService->setNewValueCallback(_onNewValue, this);
                break;
            case BLE_SIG_UUID_DEVICE_INFORMATION_SVC:
                disService = std::make_unique<DeviceInformationService>(serv, peer);
                disService->onConnect();
                break;
            case BLE_SIG_UUID_BATTERY_SVC:
                battService = std::make_unique<BatteryService>(serv, peer);
                battService->onConnect();
                battService->setNewValueCallback(_onNewValue, this);
                break;
            default:
                break;
            }
        }
    }
}

void HeartRateMonitor::_onNewValue(BleUuid uuid, void* context) {
    HeartRateMonitor* ctx = (HeartRateMonitor *)context;
    if (ctx->_callback != nullptr) {
        ctx->_callback(*ctx, uuid, ctx->_callbackContext);
    }
}

void HeartRateMonitor::setNewValueCallback(NewHeartRateCallback callback, void* context, bool onNewValueOnly) {
    _callback = callback;
    _callbackContext = context;
    if (hrService != nullptr) hrService->setNewValueCallback(_onNewValue, this, onNewValueOnly);
    // We don't need to update the battService callback to only on changed value or not, since by definition
    // this service only sends a new value when there's a change.
}