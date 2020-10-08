#include "services/device_information_service.h"

void DeviceInformationService::onConnect()
{
    _peer.discoverCharacteristicsOfService(service);
    for (auto &ch : _peer.characteristics(service)) {
        if (ch.UUID().type() == BleUuidType::SHORT) {
            switch (ch.UUID().shorted()) {
                case BLE_SIG_MANUFACTURER_NAME_STRING_CHAR:
                    _manufacturerNameString = std::make_unique<BleCharacteristic>();
                    *_manufacturerNameString = ch;
                    break;
                case BLE_SIG_MODEL_NUMBER_STRING_CHAR:
                    _modelNumberString = std::make_unique<BleCharacteristic>();
                    *_modelNumberString = ch;
                    break;
                case BLE_SIG_SERIAL_NUMBER_STRING_CHAR:
                    _serialNumberString = std::make_unique<BleCharacteristic>();
                    *_serialNumberString = ch;
                    break;
                case BLE_SIG_HARDWARE_REVISION_STRING_CHAR:
                    _hardwareRevisionString = std::make_unique<BleCharacteristic>();
                    *_hardwareRevisionString = ch;
                    break;
                case BLE_SIG_FIRMWARE_REVISION_STRING_CHAR:
                    _firmwareRevisionString = std::make_unique<BleCharacteristic>();
                    *_firmwareRevisionString = ch;
                    break;
                case BLE_SIG_SOFTWARE_REVISION_STRING_CHAR:
                    _softwareRevisionString = std::make_unique<BleCharacteristic>();
                    *_softwareRevisionString = ch;
                    break;
                case BLE_SIG_SYSTEM_ID_CHAR:
                    _systemID = std::make_unique<BleCharacteristic>();
                    *_systemID = ch;
                    break;
                case BLE_SIG_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST:
                    _certificationDataList = std::make_unique<BleCharacteristic>();
                    *_certificationDataList = ch;
                    break;
                case BLE_SIG_PNP_ID:
                    _pnpID = std::make_unique<BleCharacteristic>();
                    *_pnpID = ch;
                    break; 
                default:
                    break;
            }
        }
    }
}

int DeviceInformationService::getManufacturerName(uint8_t* buf, size_t len) {
    if (_manufacturerNameString) {
        return _manufacturerNameString->getValue(buf, len);
    } else {
        return -1;
    }
}
int DeviceInformationService::getModelNumber(uint8_t* buf, size_t len) {
    if (_modelNumberString) {
        return _modelNumberString->getValue(buf, len);
    } else {
        return -1;
    }
}
int DeviceInformationService::getSerialNumber(uint8_t* buf, size_t len) {
    if (_serialNumberString) {
        return _serialNumberString->getValue(buf, len);
    } else {
        return -1;
    }
}
int DeviceInformationService::getHardwareRevision(uint8_t* buf, size_t len) {
    if (_hardwareRevisionString) {
        return _hardwareRevisionString->getValue(buf, len);
    } else {
        return -1;
    }
}
int DeviceInformationService::getFirmwareRevision(uint8_t* buf, size_t len) {
    if (_firmwareRevisionString) {
        return _firmwareRevisionString->getValue(buf, len);
    } else {
        return -1;
    }
}
int DeviceInformationService::getSoftwareRevision(uint8_t* buf, size_t len) {
    if (_softwareRevisionString) {
        return _softwareRevisionString->getValue(buf, len);
    } else {
        return -1;
    }
}