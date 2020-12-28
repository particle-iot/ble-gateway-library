#include "blood-pressure-monitor.h"

void BloodPressureMonitor::onConnect()
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
            case BLE_SIG_UUID_BLOOD_PRESSURE_SVC:
                _bpService = std::make_unique<BloodPressureService>(serv, peer);
                _bpService->onConnect();
                break;
            case BLE_SIG_UUID_DEVICE_INFORMATION_SVC:
                _disService = std::make_unique<DeviceInformationService>(serv, peer);
                _disService->onConnect();
                break;
            case BLE_SIG_UUID_BATTERY_SVC:
                _battService = std::make_unique<BatteryService>(serv, peer);
                _battService->onConnect();
                break;
            default:
                break;
            }
        }
    }
}

void BloodPressureMonitor::_onNewValue(BleUuid uuid, void* context) {
    BloodPressureMonitor* ctx = (BloodPressureMonitor *)context;
    if (ctx->_callback != nullptr) ctx->_callback(*ctx, uuid, ctx->_callbackContext);
}

void BloodPressureMonitor::setNewValueCallback(NewBloodPressureCallback callback, void* context) {
    _callback = callback;
    _callbackContext = context;
    if (_bpService != nullptr) _bpService->setNewValueCallback(_onNewValue, this);
    if (_battService != nullptr) _battService->setNewValueCallback(_onNewValue, this);
}