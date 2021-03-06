#include "services/battery_service.h"

void BatteryService::onConnect() {
    _peer.discoverCharacteristicsOfService(service);
    for (auto& ch: _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted())
            {
                case BLE_SIG_BATTERY_LEVEL_CHAR:
                    _batteryLevel = ch;
                    break;
                default:
                    break;
            }
        }
    }
}

int BatteryService::getBatteryLevel() {
    return (_batteryLevel.isValid()) ? _level : -1;
}

int BatteryService::forceBatteryUpdate() {
    if (_batteryLevel.isValid()) {
        _batteryLevel.getValue(&_level, 1);
        return _level;
    } else {
        return -1;
    }
}

bool BatteryService::supportsNotify() {
    return _batteryLevel.properties().isSet(BleCharacteristicProperty::NOTIFY);
}

void BatteryService::setNewValueCallback(void (*callback)(BleUuid, void*), void* context) {
    if (_batteryLevel.properties().isSet(BleCharacteristicProperty::NOTIFY)) {
        _notifyNewData = callback;
        _notifyContext = context;
        _batteryLevel.onDataReceived(onDataReceived, this);
        _batteryLevel.subscribe(true);
    };
}

void BatteryService::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
    BatteryService* ctx = (BatteryService *)context;
    if (len > 0) {
        ctx->_level = data[0];
        if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(BleUuid(BLE_SIG_BATTERY_LEVEL_CHAR), ctx->_notifyContext);
    }
}