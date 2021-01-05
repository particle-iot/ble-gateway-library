
#include "ble-gateway.h"
#include "peripherals/masterbuilt-smoker.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

//Serial1LogHandler logHandler(115200, LOG_LEVEL_TRACE);
SerialLogHandler logHandler(115200, LOG_LEVEL_TRACE);

bool publish = false;

void onChangedSmokerValue(MasterbuiltSmoker& smoker, void* context) {
  // This callback executes on the BLE thread, so we need to avoid lengthy operations
  // such as Particle.publish().
  // https://docs.particle.io/reference/device-os/firmware/boron/#ondatareceived-
  publish = true;
}

void onConnect(BleDevice& device)
{
  Log.info("Connect callback");
  uint8_t buf[20];
  int len = device.getDeviceName(buf, 20);
  if (len > -1) {
    Log.info("Device Name: %.*s", len, buf);
  }
  if (device.getType() == BleUuid(MASTERBUILT_SMOKER_SERVICE)) {
    MasterbuiltSmoker& dev = (MasterbuiltSmoker&)device;
    dev.setNewValueCallback(onChangedSmokerValue, NULL, true);
  } 
}

void onPasskeyInput(BleDevice& device) {
  if (device.getType() == BleUuid(MASTERBUILT_SMOKER_SERVICE)) {
    device.passkeyInput((uint8_t *)"000000");
  } else {
    device.passkeyInput();
  }
}

void onPasskeyDisplay(BleDevice& device, const uint8_t* passkey, size_t passkeyLen) {
  char buf[passkeyLen+1];
  memcpy(buf, passkey, passkeyLen);
  buf[passkeyLen+1] = '\0';
  Log.info("App passkey display: %s", buf);
}

int setTempAndTime(String command)
{
  uint16_t temp = 225;
  uint16_t time = 60;
  JSONValue root = JSONValue::parseCopy(command, command.length());
  if(!root.isObject() || !root.isValid()) return -EINVAL;
  JSONObjectIterator iter(root);
  while(iter.next()) {
    if (iter.name() == "temp") {
      temp = iter.value().toInt();
    } else if (iter.name() == "time") {
      time = iter.value().toInt();
    }
  }
  /* The following will update the temperature and time of all connected smokers.
   * If the application will really control more than one, it might make sense to
   * instead make a function that controls them individually, for example by BLE
   * address.
   */
  for (auto& dev : BleDeviceGateway::instance().connectedDevices()) {
        if (dev->getType() == BleUuid(MASTERBUILT_SMOKER_SERVICE)) {
          std::shared_ptr<MasterbuiltSmoker> smoker = std::static_pointer_cast<MasterbuiltSmoker>(dev);
          smoker->setTempAndTime(temp, time);
        }
  }
  return 0;
}

void setup() {
  Particle.function("setTempAndtime", setTempAndTime);
  BleDeviceGateway::instance().setup(BlePairingIoCaps::KEYBOARD_DISPLAY);
  BleDeviceGateway::instance().onConnectCallback(onConnect);
  BleDeviceGateway::instance().enableServiceByName(MasterbuiltSmoker::bleDevicePtr, "Masterbuilt Smoker");
  BleDeviceGateway::instance().onPasskeyDisplay(onPasskeyDisplay);
  BleDeviceGateway::instance().onPasskeyInput(onPasskeyInput);
  BLE.on();
  Particle.connect();
}

void loop() {
  BleDeviceGateway::instance().loop();
  if (publish && Particle.connected()) {
    publish = false;
    char buf[622];
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    for (auto& dev : BleDeviceGateway::instance().connectedDevices()) {
      if (dev->getType() == BleUuid(MASTERBUILT_SMOKER_SERVICE)) {
        std::shared_ptr<MasterbuiltSmoker> smoker = std::static_pointer_cast<MasterbuiltSmoker>(dev);
        writer.name(smoker->peer.address().toString()).beginObject();
        if (smoker->isPowerOn()) {
          writer.name("smoker").value(smoker->getSmokerTemp());
          writer.name("probe").value(smoker->getProbeTemp());
          writer.name("set_temp").value(smoker->getSetTemp());
          writer.name("remain_time").value(smoker->getRemainingTime());
        } else {
          writer.name("power").value("off");
        }
        writer.endObject();
      }
    }
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    Particle.publish("masterbuilt", writer.buffer(), PRIVATE);
  }
}