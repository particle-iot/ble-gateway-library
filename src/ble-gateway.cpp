
#include "ble-gateway.h"

#define BLE_MAX_PERIPHERALS     3
#define MAX_WAITLIST            10
#define MAX_DISCOVER_SERVICES   14

void onDisconnected(const BlePeerDevice &peer, void *context);
void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
void onPairing(const BlePairingEvent &event, void *context);

BleDeviceGateway *BleDeviceGateway::_instance = nullptr;

void BleDeviceGateway::setup(BlePairingIoCaps capabilities)
{
    _scan_period = 10;
    BLE.onDisconnected(onDisconnected, this);
    BLE.setPairingIoCaps(capabilities);
    BLE.onPairingEvent(onPairing, this);
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
                delay(50);
                std::shared_ptr<BleDevice> ptr = _waitlist.takeFirst();
                _connectedDevices.append(ptr);
                ptr->onConnect();
                if(ptr->peer.connected()) {
                    if (_connectCallback != nullptr) _connectCallback(*ptr);
                } else {
                    for (int i = 0; i < _connectedDevices.size(); i++)
                    {
                        if (_connectedDevices.at(i)->peer.address() == ptr->address)
                        {
                            _connectedDevices.removeAt(i);
                            break;
                        }
                    }
                }
            }
            else
            {
                Log.info("Could not connect");
                _waitlist.takeFirst();
            }
        }
    }
    for (int i = 0; i < _connectedDevices.size(); i++)
    {
        if (_connectedDevices.at(i)->peer.connected()) _connectedDevices.at(i)->loop();
    }
}

void BleDeviceGateway::enableServiceByName(BleDeviceGatewayDevicePtrGen bleDevicePtrGen, const char* completeName)
{
    EnabledService service;
    service.completeLocalName = completeName;
    service.bleDevicePtrGen = bleDevicePtrGen;
    _enabledServices.append(service);
}

void BleDeviceGateway::enableServiceCustom(BleDeviceGatewayDevicePtrGen bleDevicePtrGen, const char* customService)
{
    EnabledService service;
    service.customService = customService;
    service.bleDevicePtrGen = bleDevicePtrGen;
    _enabledServices.append(service);
}
void BleDeviceGateway::enableService(BleDeviceGatewayDevicePtrGen bleDevicePtrGen, uint16_t sigService)
{
    EnabledService service;
    service.stdService = sigService;
    service.bleDevicePtrGen = bleDevicePtrGen;
    _enabledServices.append(service);
}



bool BleDeviceGateway::rotateDevice(BleDevice& device) const
{
    if (_waitlist.isEmpty()) return false;
    return (device.peer.disconnect() == SYSTEM_ERROR_NONE) ? true : false;
}

bool BleDeviceGateway::isAddressConnectable(const BleAddress& address) const
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

int BleDeviceGateway::connectableService(const BleScanResult *scanResult) const
{
    for (int idx = 0; idx < _enabledServices.size(); idx++) {
        if ( _enabledServices.at(idx).completeLocalName != nullptr ) {
            if (scanResult->advertisingData().deviceName() == _enabledServices.at(idx).completeLocalName || scanResult->scanResponse().deviceName() == _enabledServices.at(idx).completeLocalName) {
                return idx;
            }
        } else {
            Vector<BleUuid> foundUuids = scanResult->advertisingData().serviceUUID();
            if ( _enabledServices.at(idx).stdService != 0) {
                if (foundUuids.contains(_enabledServices.at(idx).stdService)) return idx;
            } else if (_enabledServices.at(idx).customService != nullptr) {
                if (foundUuids.contains(_enabledServices.at(idx).customService)) return idx;
            }
        }
    }
    return -1;
}

void BleDeviceGateway::scanResultCallback(const BleScanResult *scanResult, void *context)
{
    BleDeviceGateway* ctx = (BleDeviceGateway *)context;
    if (ctx->isAddressConnectable(scanResult->address()))
    {
        int idx = ctx->connectableService(scanResult);
        if (idx >= 0)
        {
            if (ctx->_waitlist.size() < MAX_WAITLIST)
            {
                int i;
                for (i = 0; i < ctx->_waitlist.size(); i++)
                {
                    if (ctx->_waitlist.at(i)->address == scanResult->address())
                        break;
                }
                if (i == ctx->_waitlist.size())
                {
                    std::shared_ptr<BleDevice> newDev = (ctx->_enabledServices.at(idx).bleDevicePtrGen)(scanResult);
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

    for (int i = 0; i < ctx->_connectedDevices.size(); i++)
    {
        if (ctx->_connectedDevices.at(i)->peer.address() == peer.address())
        {
            ctx->_connectedDevices.removeAt(i);
            break;
        }
    }
}

void BleDeviceGateway::onPairing(const BlePairingEvent &event, void *context)
{
    BleDeviceGateway* ctx = (BleDeviceGateway *)context;
    for (int i = 0; i < ctx->_connectedDevices.size(); i++)
    {
        if (ctx->_connectedDevices.at(i)->peer.address() == event.peer.address())
        {
            switch (event.type)
            {
            case BlePairingEventType::PASSKEY_DISPLAY:
                if (ctx->_passkeyDisplayCallback == nullptr)
                {
                    char buf[event.payloadLen+1];
                    memcpy(buf, event.payload.passkey, event.payloadLen);
                    buf[event.payloadLen+1] = '\0';
                    Log.info("Default passkey display: %s", buf);
                } else {
                    ctx->_passkeyDisplayCallback(event.payload.passkey, event.payloadLen);
                }
                break;
            case BlePairingEventType::PASSKEY_INPUT:
            {
                uint8_t* passkey = nullptr;
                if (ctx->_passkeyInputCallback == nullptr) {
                    if (ctx->_connectedDevices.at(i)->passkeyInput(passkey) >= 0) {
                        BLE.setPairingPasskey(ctx->_connectedDevices.at(i)->peer, passkey);
                    } else {
                        BLE.rejectPairing(ctx->_connectedDevices.at(i)->peer);
                    }
                } else {
                    if (ctx->_passkeyInputCallback(passkey) >= 0) {
                        BLE.setPairingPasskey(ctx->_connectedDevices.at(i)->peer, passkey);
                    } else {
                        BLE.rejectPairing(ctx->_connectedDevices.at(i)->peer);
                    }
                }
            }
                break;
            case BlePairingEventType::STATUS_UPDATED:
                if (event.payload.status.status == BLE_GAP_SEC_STATUS_SUCCESS) {
                    ctx->_connectedDevices.at(i)->onPair();
                } else {
                    Log.info("Pairing failed with code: %d", event.payload.status.status);
                }
                break;
            default:
                break;
            }
            break;
        }
    }
}