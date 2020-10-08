#include "services/cycling_speed_and_cadence.h"

void CyclingSpeedAndCadenceService::onConnect()
{
    _peer.discoverCharacteristicsOfService(service);
    for (auto &ch : _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted())
            {
                case BLE_SIG_CSC_MEASUREMENT_CHAR:
                    _cscMeasurement = std::make_unique<CyclingSpeedAndCadenceChar>(ch);
                    _cscMeasurement->onConnect();
                    enableNotification();
                    break;
                case BLE_SIG_CSC_FEATURE_CHAR:
                    _cscFeature = std::make_unique<CSCFeatureChar>(ch);
                    _cscFeature->onConnect();
                    break;
                /*case BLE_SIG_HEART_RATE_CONTROL_POINT_CHAR:
                    _controlPoint = std::make_unique<HeartRateControlPoint>(ch);
                    break; */
                default:
                    break;
            }
        }
    }
}

void CyclingSpeedAndCadenceService::setNewValueCallback(void (*callback)(BleUuid, void*), void* context) {
    if (_cscMeasurement) _cscMeasurement->notifyCallback(callback, context);
}