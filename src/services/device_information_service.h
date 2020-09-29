#pragma once

#include "Particle.h"

#define BLE_SIG_MANUFACTURER_NAME_STRING_CHAR 0x2A29
#define BLE_SIG_MODEL_NUMBER_STRING_CHAR 0x2A24
#define BLE_SIG_SERIAL_NUMBER_STRING_CHAR 0x2A25
#define BLE_SIG_HARDWARE_REVISION_STRING_CHAR 0x2A27
#define BLE_SIG_FIRMWARE_REVISION_STRING_CHAR 0x2A26
#define BLE_SIG_SOFTWARE_REVISION_STRING_CHAR 0x2A28
#define BLE_SIG_SYSTEM_ID_CHAR 0x2A23
#define BLE_SIG_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST 0x2A2A
#define BLE_SIG_PNP_ID 0x2A50

class DeviceInformationService 
{
private:
    std::unique_ptr<BleCharacteristic> _manufacturerNameString;
    std::unique_ptr<BleCharacteristic> _modelNumberString;
    std::unique_ptr<BleCharacteristic> _serialNumberString;
    std::unique_ptr<BleCharacteristic> _hardwareRevisionString;
    std::unique_ptr<BleCharacteristic> _firmwareRevisionString;
    std::unique_ptr<BleCharacteristic> _softwareRevisionString;
    std::unique_ptr<BleCharacteristic> _systemID;
    std::unique_ptr<BleCharacteristic> _certificationDataList;
    std::unique_ptr<BleCharacteristic> _pnpID;
public:
    BleService service;
    void onConnect();
    int getManufacturerName(uint8_t* buf, size_t len);
    int getModelNumber(uint8_t* buf, size_t len);
    int getSerialNumber(uint8_t* buf, size_t len);
    int getHardwareRevision(uint8_t* buf, size_t len);
    int getFirmwareRevision(uint8_t* buf, size_t len);
    int getSoftwareRevision(uint8_t* buf, size_t len);
    /* TODO: Implement other functions */

    DeviceInformationService(BleService serv): service(serv) {}
    ~DeviceInformationService() {}
};