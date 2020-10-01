/* ble-devices library by Mariano Goluboff
 */

#include "ble-gateway.h"

#define BLE_MAX_PERIPHERALS     3
#define MAX_WAITLIST            10
#define MAX_DISCOVER_SERVICES   14

void onDisconnected(const BlePeerDevice &peer, void *context);
void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);

BleDeviceGateway *BleDeviceGateway::_instance = nullptr;

void BleDeviceGateway::setup()
{
    _scan_period = 10;
    BLE.onDisconnected(onDisconnected, this);
}

void BleDeviceGateway::loop()
{
    static unsigned int scan_period = System.uptime();
    if (System.uptime() - scan_period > _scan_period)
    {
        scan_period = System.uptime();
        _waitlist.clear();
        BLE.scan(scanResultCallback, this);
    }
    //if (!BLE.scanning())
    if (_connectedDevices.size() < BLE_MAX_PERIPHERALS)
    {
        while ( (_connectedDevices.size() < BLE_MAX_PERIPHERALS) && !(_waitlist.isEmpty()) )
        {
            _waitlist.first()->peer = BLE.connect(_waitlist.first()->address, false);
            if (_waitlist.first()->peer.connected())
            {
                Log.info("Successfully connected");
                std::shared_ptr<BleDevice> ptr = _waitlist.takeFirst();
                ptr->onConnect();
                if(ptr->peer.connected() && _connectCallback != nullptr) {
                    _connectCallback(*ptr);
                    _connectedDevices.append(ptr);
                }
            }
            else
            {
                Log.info("Could not connect");
            }
        }
    }
    for (int i = 0; i < _connectedDevices.size(); i++)
    {
        if (_connectedDevices.at(i)->peer.connected()) _connectedDevices.at(i)->loop();
    }
}

bool BleDeviceGateway::enableService(const char *customService)
{
    if (_enabledCustomServices.contains(customService)) {
        return false;
    } else {
        return _enabledCustomServices.append(customService);
    }
}

bool BleDeviceGateway::enableService(uint16_t sigService) 
{
    if (_enabledStdServices.contains(sigService)) {
        return false;
    } else {
        return _enabledStdServices.append(sigService);
    }
}

bool BleDeviceGateway::rotateDevice(BleDevice& device)
{
    if (_waitlist.isEmpty()) return false;
    return (device.peer.disconnect() == SYSTEM_ERROR_NONE) ? true : false;
}

bool BleDeviceGateway::isAddressConnectable(BleAddress address)
{
    if (_allowlist.isEmpty() && _denylist.isEmpty()) return true;
    if (!_allowlist.isEmpty()) {
        for (auto addr: _allowlist) {
            if (addr.toString() == address.toString()) return true;
        }
        return false;
    }
    if (!_denylist.isEmpty()) {
        for (auto addr: _denylist) {
            if (addr.toString() == address.toString()) return false;
        }
        return true;
    } 
    return false;
}

void BleDeviceGateway::connectableService(const BleScanResult *scanResult, BleUuid *uuid)
{
    BleUuid foundServices[14];
    size_t len = scanResult->advertisingData.serviceUUID(&foundServices[0], 14);
    for (size_t serviceIdx = 0; serviceIdx < len; serviceIdx++) {
        if (foundServices[serviceIdx].type() == BleUuidType::SHORT) {
            for (int i = 0; i < _enabledStdServices.size(); i++) {
                if (foundServices[serviceIdx] == _enabledStdServices.at(i)) *uuid = BleUuid(_enabledStdServices.at(i));
            }
        } else {
            for (int i = 0; i < _enabledCustomServices.size(); i++) {
                if (foundServices[serviceIdx] == _enabledCustomServices.at(i)) *uuid = BleUuid(_enabledCustomServices.at(i));
            }
        }
    }
}

void BleDeviceGateway::scanResultCallback(const BleScanResult *scanResult, void *context)
{
    BleDeviceGateway* ctx = (BleDeviceGateway *)context;
    if (ctx->isAddressConnectable(scanResult->address))
    {
        BleUuid uuid;
        ctx->connectableService(scanResult, &uuid);
        if (uuid.isValid())
        {
            if (ctx->_waitlist.size() < MAX_WAITLIST)
            {
                int i;
                for (i = 0; i < ctx->_waitlist.size(); i++)
                {
                    if (ctx->_waitlist.at(i)->address == scanResult->address)
                        break;
                }
                if (i == ctx->_waitlist.size())
                {
                    std::shared_ptr<BleDevice> newDev = BleTypes::getDevicePointer(uuid, scanResult);
                    if (newDev != nullptr) ctx->_waitlist.append(newDev);
                }
            }
            else {
                BLE.stopScanning();
            }
        }
    }
}

void BleDeviceGateway::onDisconnected(const BlePeerDevice &peer, void *context)
{
    BleDeviceGateway* ctx = (BleDeviceGateway *)context;
    Log.info("BLE Device disconnected: %s", peer.address().toString().c_str());

    for (int i = 0; i < ctx->_connectedDevices.size(); i++)
    {
        if (ctx->_connectedDevices.at(i)->peer.address() == peer.address())
        {
            ctx->_connectedDevices.removeAt(i);
            break;
        }
    }
}
