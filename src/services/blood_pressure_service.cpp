#include "services/blood_pressure_service.h"

void BloodPressureService::onConnect()
{
    _peer.discoverCharacteristicsOfService(service);
    for (auto &ch : _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted())
            {
                case BLE_SIG_BLOOD_PRESSURE_MEASUREMENT_CHAR:
                    _bpMeasurement = std::make_unique<BloodPressureMeasurement>(ch);
                    _bpMeasurement->onConnect();
                    break;
                case BLE_SIG_INTERMEDIATE_CUFF_PRESSURE_CHAR:
                    _intermediateCuff = std::make_unique<IntermediateCuffPressureChar>(ch);
                    _intermediateCuff->onConnect();
                    break;
                case BLE_SIG_BLOOD_PRESSURE_FEATURE_CHAR:
                    _bpFeature = std::make_unique<BloodPressureFeatureChar>(ch);
                    break;
                default:
                    break;
            }
        }
    }
}

void BloodPressureService::setNewValueCallback(void (*callback)(BleUuid, void*), void* context) {
    if (_bpMeasurement != nullptr) _bpMeasurement->notifyCallback(callback, context);
    if (_intermediateCuff != nullptr) _intermediateCuff->notifyCallback(callback, context);
}