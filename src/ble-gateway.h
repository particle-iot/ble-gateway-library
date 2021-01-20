#pragma once


#include "Particle.h"
#include "peripherals/ble_device.h"

class BleDeviceGateway;

typedef void (*BleDeviceGatewayConnection)(BleDevice& device);
typedef void (*BlePasskeyDisplay)(BleDevice& device, const uint8_t* passkey, size_t passkeyLen);
typedef void (*BlePasskeyInput)(BleDevice& device);
typedef std::shared_ptr<BleDevice> (*BleDeviceGatewayDevicePtrGen)(const BleScanResult*);

class BleDeviceGateway
{
public:
  /**
   * @brief Singleton class instance access for BleDeviceGateway.
   *
   * @return BleDeviceGateway&
   */
  static BleDeviceGateway& instance() {
    if (!_instance) {
      _instance = new BleDeviceGateway();
    }
    return *_instance;
  }
  /**
   * In your setup(), initialize the BLE Gateway by calling this. For example:
   * 
   * BleDeviceGateway::instance().setup(BlePairingIoCaps::KEYBOARD_DISPLAY);
   * 
   * @param capabilities These are the pairing input/output capabilities of your device. They can be:
   *  NONE - default, no input/ouput
   *  DISPLAY_ONLY - can display a passkey, but no input
   *  DISPLAY_YESNO - can display a passkey, and user can enter yes/no
   *  KEYBOARD_ONLY - user can enter a passkey, but no display
   *  KEYBOARD_DISPLAY - user can enter a passkey, and device can display a passkey
   * 
   */
  void setup(BlePairingIoCaps capabilities = BlePairingIoCaps::NONE);
  /**
   * Register a callback that will be called for the Application to display
   * the passkey being sent by the peripheral. If no callback is registered and the
   * Gateway receives a passkey, it will be logged via Log.info
   */
  void onPasskeyDisplay(BlePasskeyDisplay callback) {_passkeyDisplayCallback = callback;};
  /**
   * Register a callback that will be called for the Application to be able to enter
   * a passkey to send to the peripheral. If the peripheral requests a passkey and
   * the Application does not have this callback registered, then the Gateway will call
   * the passkeyInput() function in ble_device.h. If the library doesn't have an 
   * override for that function, pairing will be rejected.
   */
  void onPasskeyInput(BlePasskeyInput callback) {_passkeyInputCallback = callback;};
  void loop();

  /**
   * Enable the services that the Gateway will be able to connect to. You can enable by:
   * 
   * enableServiceByName: this is for devices that advertise by a Local Name, instead of by
   * advertising a primary service.
   * 
   * enableServiceCustom: this is for devices that advertise a primary service that uses
   * a custom UUID.
   * 
   * enableService: this is for devices that advertise a primary service that uses a
   * UUID defined by the Bluetooth SIG.
   */
  void enableServiceByName(BleDeviceGatewayDevicePtrGen bleDevicePtrGen, const char* completeName);
  void enableServiceCustom(BleDeviceGatewayDevicePtrGen bleDevicePtrGen, const char *customService);
  void enableService(BleDeviceGatewayDevicePtrGen bleDevicePtrGen, uint16_t sigService);


  Vector<std::shared_ptr<BleDevice>>& connectedDevices() {return _connectedDevices;}

  /**
   * If there are scanned devices in the waitlist to be connected, this will disconnect from
   * the device. It will be appended to the waitlist at the next scan in last place.
   * 
   * If there are no devices in the waitlist, then nothing will happen.
   * 
   * @param device The device to be disconnected from
   * @return true if disconnected, false if not disconnected
   */
  bool rotateDevice(BleDevice& device) const;
  /**
   * Register a callback to be called when any peripheral is connected to.
   * 
   * The callback will receive the device as a parameter. Use device.getType() to determine what
   * kind of device it is (getType() returns the primary service UUID).
   */
  int onConnectCallback(BleDeviceGatewayConnection callback) {_connectCallback = callback; return 0;};
  void addToAllowList(BleAddress address) {_allowlist.append(address);};
  void addToDenyList(BleAddress address) {_denylist.append(address); }

private:
  struct EnabledService {
    uint16_t stdService;
    const char *customService;
    const char *completeLocalName;
    BleDeviceGatewayDevicePtrGen bleDevicePtrGen;
    EnabledService(): stdService(0), customService(nullptr), completeLocalName(nullptr) {};
  };
  Vector<BleAddress> _allowlist, _denylist;
  Vector<std::shared_ptr<BleDevice>> _waitlist, _connectedDevices;
  Vector<EnabledService> _enabledServices;
  uint16_t _scan_period;
  uint8_t _connected_count;
  static void scanResultCallback(const BleScanResult *scanResult, void *context);
  static void onPairing(const BlePairingEvent &event, void *context);
  BlePasskeyDisplay _passkeyDisplayCallback;
  BlePasskeyInput _passkeyInputCallback;
  BleDeviceGatewayConnection _connectCallback;
  // Singleton instance
  static BleDeviceGateway* _instance;
  bool isAddressConnectable(const BleAddress& address) const;
  int connectableService(const BleScanResult *scanResult) const;
  BleDeviceGateway():  
    _connected_count(0),
    _passkeyDisplayCallback(nullptr),
    _passkeyInputCallback(nullptr),
    _connectCallback(nullptr) {};
};
