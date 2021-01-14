
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
    if (_connected_count < BLE_MAX_PERIPHERALS)
    {
        while ( (_connected_count < BLE_MAX_PERIPHERALS) && !(_waitlist.isEmpty()) )
        {
            _waitlist.first()->peer = BLE.connect(_waitlist.first()->address, false);
            if (_waitlist.first()->peer.connected())
            {
                Log.info("Successfully connected %s", _waitlist.first()->peer.address().toString().c_str());
                delay(50);
                std::shared_ptr<BleDevice> ptr = _waitlist.takeFirst();
                _connectedDevices.append(ptr);
                _connected_count++;
                ptr->onConnect();       // Call the onConnect() for the BLE Device
                if(ptr->peer.connected()) {
                    // if still connected, call the App's onConnect callback
                    if (_connectCallback != nullptr) _connectCallback(*ptr);
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
        _connectedDevices.at(i)->loop();
        if (!_connectedDevices.at(i)->peer.connected() && !_connectedDevices.at(i)->pendingData()) {
            // Check if disconnected device objects are still holding buffered data.
            // If not, then we can remove it from the _connectedDevices vector.
            Log.trace("Remove device");
            _connectedDevices.removeAt(i);
        }
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
    for (int idx = 0; idx < _connectedDevices.size(); idx++) {
        // Check if there's already an object for the device with this address
        if (address == _connectedDevices.at(idx)->address) return false;
    }
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
            // If the device is enabled by Local Name, check whether the scanned device has that name
            // either in the advertisingData or the scanResponse data, as devices could include it in either one.
            if (scanResult->advertisingData().deviceName() == _enabledServices.at(idx).completeLocalName || scanResult->scanResponse().deviceName() == _enabledServices.at(idx).completeLocalName) {
                return idx;
            }
        } else {
            // Get the Service UUIDs from both the advertisingData and the scanResponse into
            // a Vector to then check if it's in either of the enabled UUID lists.
            Vector<BleUuid> foundUuids = scanResult->advertisingData().serviceUUID();
            foundUuids.append(scanResult->scanResponse().serviceUUID());
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
    /*
     * This is run on the BLE thread, so no other BLE APIs can be called. Instead of connecting
     * to devices here, we instead add them to a waitlist, and then when the loop() is executed
     * on the application thread, we'll check the waitlist to connect to devices.
     */ 
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
    /*
     * This is run on the BLE thread, so no other BLE APIs can be called. 
     */
    BleDeviceGateway* ctx = (BleDeviceGateway *)context;
    ctx->_connected_count--;
    Log.trace("Devices connected: %u", ctx->_connected_count);
}

void BleDeviceGateway::onPairing(const BlePairingEvent &event, void *context)
{
    /*
     * This is not run on the BLE thread. You can call other BLE APIs here.
     * For example, you might call BLE.rejectPairing() if you want to reject the pairing process.
     * 
     * This handles callbacks related to the pairing process.
     */
    BleDeviceGateway* ctx = (BleDeviceGateway *)context;
    for (int i = 0; i < ctx->_connectedDevices.size(); i++)
    {
        if (ctx->_connectedDevices.at(i)->peer.address() == event.peer.address())
        {
            switch (event.type)
            {
            case BlePairingEventType::PASSKEY_DISPLAY:
            /*
             * When it's time to display the passkey, check if the Application has registered a callback
             * to display the passkey. If not, then just print out the passkey into the log stream.
             */
                if (ctx->_passkeyDisplayCallback == nullptr)
                {
                    char buf[event.payloadLen+1];
                    memcpy(buf, event.payload.passkey, event.payloadLen);
                    buf[event.payloadLen] = '\0';
                    Log.info("Default passkey display: %s", buf);
                } else {
                    ctx->_passkeyDisplayCallback(*ctx->_connectedDevices.at(i), event.payload.passkey, event.payloadLen);
                }
                break;
            case BlePairingEventType::PASSKEY_INPUT:
            /*
             * This handles when the pairing process requests a passkey input. If the application has
             * registered a callback, then run that. The application can get the passkey, for example, from
             * a keyboard or from a Particle.function() that allows the passkey to be entered from the cloud.
             * 
             * The application needs to send the 6-digit passkey back with something like:
             * BLE.setPairingPasskey(peer, (uint8_t *)"000000");
             * 
             * If the application hasn't registered a callback, then it'll call passkeyInput() on the 
             * BleDevice instance. By default, this will call BLE.rejectPairing(), so if you add a peripheral
             * that always has the same passkey, you should override the passkeyInput() fuction when adding
             * a new peripheral.
             */
            {
                Log.info("Passkey input");
                if (ctx->_passkeyInputCallback == nullptr) {
                    ctx->_connectedDevices.at(i)->passkeyInput();
                } else {
                    ctx->_passkeyInputCallback(*ctx->_connectedDevices.at(i));
                }
            }
                break;
            case BlePairingEventType::STATUS_UPDATED:
            /*
             * This is called when pairing is done. If successful, it'll call the BleDevice instance's onPair()
             * function. Override this on a new peripheral if you want something to happen after pairing.
             */
                ctx->_connectedDevices.at(i)->onPair(event.payload.status.status);
                break;
            default:
                break;
            }
            break;
        }
    }
}