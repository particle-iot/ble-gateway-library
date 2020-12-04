#pragma once

/* ble-devices library by Mariano Goluboff
 */

#include "Particle.h"
#include "peripherals/ble-peripherals.h"

class BleDeviceGateway;

typedef void (*BleDeviceGatewayConnection)(BleDevice& device);
#if (SYSTEM_VERSION >= SYSTEM_VERSION_v200RC4)
typedef void (*BlePasskeyDisplay)(const uint8_t* passkey, size_t passkeyLen);
typedef int (*BlePasskeyInput)(uint8_t* passkey);
#endif

class BleDeviceGateway
{
private:
  Vector<BleAddress> _allowlist, _denylist;
  Vector<std::shared_ptr<BleDevice>> _waitlist, _connectedDevices;
  Vector<uint16_t> _enabledStdServices;
  Vector<const char*> _enabledCustomServices, _enabledLocalNames, _localNamesUuid;
  uint16_t _scan_period;
  static void scanResultCallback(const BleScanResult *scanResult, void *context);
  static void onDisconnected(const BlePeerDevice &peer, void *context);
#if (SYSTEM_VERSION >= SYSTEM_VERSION_v200RC4)
  static void onPairing(const BlePairingEvent &event, void *context);
  BlePasskeyDisplay _passkeyDisplayCallback;
  BlePasskeyInput _passkeyInputCallback;
#endif
  BleDeviceGatewayConnection _connectCallback;
  // Singleton instance
  static BleDeviceGateway* _instance;
  bool isAddressConnectable(const BleAddress& address) const;
  void connectableService(const BleScanResult *scanResult, BleUuid *uuid) const;
  BleDeviceGateway():  
#if (SYSTEM_VERSION >= SYSTEM_VERSION_v200RC4)
      _passkeyDisplayCallback(nullptr),
      _passkeyInputCallback(nullptr),
#endif
      _connectCallback(nullptr) {};

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
#if (SYSTEM_VERSION >= SYSTEM_VERSION_v200RC4)
  void setup(BlePairingIoCaps capabilities = BlePairingIoCaps::NONE);
  void onPasskeyDisplay(BlePasskeyDisplay callback) {_passkeyDisplayCallback = callback;};
  void onPasskeyInput(BlePasskeyInput callback) {_passkeyInputCallback = callback;};
#else
  void setup();
#endif
  void loop();

  bool enableService(const char *customService);
  bool enableService(const char *completeLocalName, const char *customService);
  bool enableService(uint16_t sigService);

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
};
