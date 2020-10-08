#include "heart-rate-monitor.h"

void HeartRateMonitor::onConnect()
{
    peer.discoverAllServices();
    for (auto& serv: peer.services()) {
        if (serv.UUID().type() == BleUuidType::SHORT) {
            switch (serv.UUID().shorted())
            {
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

void HeartRateMonitor::setNewValueCallback(NewHeartRateCallback callback, void* context) {
    _callback = callback;
    _callbackContext = context;
}