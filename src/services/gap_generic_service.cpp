#include "services/gap_generic_service.h"

void GapGenericService::onConnect()
{
    _peer.discoverCharacteristicsOfService(service);
    for (auto &ch : _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted())
            {
                case BLE_SIG_DEVICE_NAME_CHAR:
                    _deviceName = std::make_unique<DeviceNameCharacteristic>(ch);
                    break;
                case BLE_SIG_APPEARANCE_CHAR:
                    _appearance = std::make_unique<Appearance>(ch);
                    break;
                case BLE_SIG_PERIPHERAL_PREFERRED_CONNECTION_PARAMS_CHAR:
                    _peripheralConnParams = std::make_unique<PeripheralPreferredConnectionParams>(ch);
                    break;
                default:
                    break;
            }
        }
    }
}