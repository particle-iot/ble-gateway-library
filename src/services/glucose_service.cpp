#include "services/glucose_service.h"

void GlucoseService::onConnect()
{
    _peer.discoverCharacteristicsOfService(service);
    for (auto &ch : _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted())
            {
                case BLE_SIG_GLUCOSE_MEASUREMENT_CHAR:
                    _gMeasurement = std::make_unique<GlucoseMeasurement>(ch);
                    _gMeasurement->onConnect();
                    break;
                case BLE_SIG_GLUCOSE_FEATURE_CHAR:
                    _gFeature = std::make_unique<GlucoseFeatureChar>(ch);
                    break;
                case BLE_SIG_RECORD_ACCESS_CONTROL_CHAR:
                    _racp = std::make_unique<RecordAccessControlPoint>(ch);
                    _racp->onConnect();
                    break;
                default:
                    break;
            }
        }
    }
}

void GlucoseService::setNewValueCallback(void (*callback)(BleUuid, void*), void* context) {
    if (_gMeasurement != nullptr) _gMeasurement->notifyCallback(callback, context);
    if (_racp != nullptr) _racp->notifyCallback(callback, context);
}