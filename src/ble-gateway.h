#pragma once

/* ble-devices library by Mariano Goluboff
 */

#include "Particle.h"
#include "types/ble-types.h"

class BleDeviceGateway;

typedef void (*BleDeviceGatewayConnection)(BleDevice& device);

class BleDeviceGateway
{
private:
  Vector<BleAddress> _allowlist, _denylist;
  Vector<std::shared_ptr<BleDevice>> _waitlist, _connectedDevices;
  Vector<uint16_t> _enabledStdServices;
  Vector<const char*> _enabledCustomServices;
  uint16_t _scan_period;
  static void scanResultCallback(const BleScanResult *scanResult, void *context);
  static void onDisconnected(const BlePeerDevice &peer, void *context);
  BleDeviceGatewayConnection _connectCallback;
  // Singleton instance
  static BleDeviceGateway* _instance;
  bool isAddressConnectable(BleAddress address);
  void connectableService(const BleScanResult *scanResult, BleUuid *uuid);
  BleDeviceGateway(): _connectCallback(nullptr) {};

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

  void setup();
  void loop();

  bool enableService(const char *customService);
  bool enableService(uint16_t sigService);

  Vector<std::shared_ptr<BleDevice>>& connectedDevices() {return _connectedDevices;}

  bool rotateDevice(BleDevice& device);

  int onConnectCallback(BleDeviceGatewayConnection callback) {_connectCallback = callback; return 0;};
  void addToAllowList(BleAddress address) {_allowlist.append(address);};
};
