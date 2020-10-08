#include "services/heart_rate_service.h"

void HeartRateService::onConnect()
{
    _peer.discoverCharacteristicsOfService(service);
    for (auto &ch : _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted())
            {
                case BLE_SIG_HEART_RATE_MEASUREMENT_CHAR:
                    _hrMeasurement = std::make_unique<HeartRateMeasurement>(ch);
                    _hrMeasurement->onConnect();
                    enableNotification();
                    break;
                case BLE_SIG_BODY_SENSOR_LOCATION_CHAR:
                    _sensorLocation = std::make_unique<BodySensorLocation>(ch);
                    break;
                case BLE_SIG_HEART_RATE_CONTROL_POINT_CHAR:
                    _controlPoint = std::make_unique<HeartRateControlPoint>(ch);
                    break;
                default:
                    break;
            }
        }
    }
}

void HeartRateService::setNewValueCallback(void (*callback)(BleUuid, void*), void* context) {
    if (_hrMeasurement) _hrMeasurement->notifyCallback(callback, context);
}