# BLE Gateway Library

__This library requires DeviceOS version 3.0.0 or higher__

This library turns a Particle Gen3 device (Tracker, Boron, Argon) into a Bluetooth Low Energy (BLE) Central device. In this mode, it is able to detect and connect to BLE Peripherals, and expose APIs so that your application can get and/or send data to the peripherals, depending on their capabilities.

If the peripheral that you’d like to connect to is already supported by the library, you can use this without any modifications. Here's a list of peripherals currently supported:
* Heart Rate Monitor
* Cycling Speed and Cadence Sensor
* Jumper brand Pulse Oximeter

If the peripheral is not currently supported, the library is written in a modular format so that it is easy to add your peripheral.

## Usage

For basic usage, you will need to:

* Include the header files for the peripherals you want to use
* Enable the type of devices you want to connect to
* Register a callback to be notified when a connection happens

```c++
#include "ble-device-gateway.h"
#include "peripherals/pulse-oximeter.h"
#include "peripherals/cycling-sensor.h"
#include "peripherals/heart-rate-monitor.h"
#include "peripherals/masterbuilt-smoker.h"

void setup() {
  BleDeviceGateway::instance().setup();
  BleDeviceGateway::instance().onConnectCallback(onConnect);
  BleDeviceGateway::instance().enableServiceCustom(PulseOx::bleDevicePtr, JUMPER_PULSEOX_SERVICE);
  BleDeviceGateway::instance().enableService(HeartRateMonitor::bleDevicePtr, BLE_SIG_UUID_HEART_RATE_SVC);
  BleDeviceGateway::instance().enableService(CyclingSpeedAndCadence::bleDevicePtr ,BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC);
  BleDeviceGateway::instance().enableServiceByName(MasterbuiltSmoker::bleDevicePtr ,"Masterbuilt Smoker");
}

void loop() {
  BleDeviceGateway::instance().loop();
}
```

The callback function for when a device is connected has the device class as the parameter. You can find out what type of device it is by checking the `getType()` function, like this:

```c++
void onConnect(BleDevice& device)
{
  if (device.getType() == BleUuid(JUMPER_PULSEOX_SERVICE))  {
    Log.info("Connected to Jumper Pulse Oximeter");
  } else if (device.getType() == BleUuid(BLE_SIG_UUID_HEART_RATE_SVC))
  {
    HeartRateMonitor& dev = (HeartRateMonitor&)device;
    dev.setNewValueCallback(onNewHrValue, NULL);
    uint8_t buf[20];
    if (dev.getManufacturerName(buf, 20) > -1) {
      Log.info("Connected to Heart Rate Monitor named: %s", buf);
    }
    Log.info("Battery Level: %d", dev.getBatteryLevel());
  }
}
```

Here is where you also would add the capabilities that your application needs. For example, a Heart Rate Monitor typically notifies once per second of the heart rate, so the Heart Rate Monitor type in the library has an API to register a callback to receive the notifications. The battery level is usually notified only when it changes. The `NOTIFY` property is Optional for the Battery Service, while `READ` is mandatory. If you know the heart rate monitor that you're using implements `NOTIFY`, then you can also get the battery level in the same callback as the heart rate measurement. For example:

```c++
void onNewHrValue(HeartRateMonitor& monitor, BleUuid uuid, void* context) {
  if (uuid == BLE_SIG_HEART_RATE_MEASUREMENT_CHAR) {
    //Log.info("Heart Rate: %u", monitor.getHeartRate() );
  } else if (uuid == BLE_SIG_BATTERY_LEVEL_CHAR) {
    Log.info("Battery callback level: %d", monitor.getBatteryLevel() );
  } 
}
```

If you're not sure if the Heart Rate monitor supports `NOTIFY`, you can use the `bool batterySupportsNotify()` API to find out.

Another device might instead allow you to read or write to it. In that case, the type for that device would have APIs to allow you to do that within your application.

### Handle Pairing

The Gateway Library can help with pairing to a device, whether the pairing is Just Works, or it requires a passkey. Since pairing can involve entering or displaying a passkey, during `setup()` you need to tell the library what the input/output
capabilities of your device are. For example:

```c++
void setup() {
  ...
  BleDeviceGateway::instance().setup(BlePairingIoCaps::KEYBOARD_DISPLAY);
}
```

The options are:
   *  NONE - default, no input/ouput
   *  DISPLAY_ONLY - can display a passkey, but no input
   *  DISPLAY_YESNO - can display a passkey, and user can enter yes/no
   *  KEYBOARD_ONLY - user can enter a passkey, but no display
   *  KEYBOARD_DISPLAY - user can enter a passkey, and device can display a passkey

#### Display a Passkey

For displaying a passkey that is sent by the peripheral, you register a callback like this:

```c++
void onPasskeyDisplay(BleDevice& device, const uint8_t* passkey, size_t passkeyLen);

void setup() {
  ...
  BleDeviceGateway::instance().onPasskeyDisplay(onPasskeyDisplay);
}
```

The callback will be called when we receive a passkey from the device. The callback can then display it as needed. If there is no callback registered, by default the library will use `Log.info` to log the passkey received.

#### Input a passkey

For entering a passkey value, you can register a callback like this:

```c++
void onPasskeyInput(BleDevice& device) {
  if (device.getType() == BleUuid(MASTERBUILT_SMOKER_SERVICE)) {
    device.passkeyInput((uint8_t *)"000000");
  } else {
    device.passkeyInput();
  }
}

void setup() {
  ...
  BleDeviceGateway::instance().onPasskeyInput(onPasskeyInput);
}
```

The callback must call the `passkeyInput` function on the `BleDevice` object. Either call it with a pointer to the passkey (in the example above, 000000), or call it without an argument to use the default passkey
for that type of peripheral. Note that if the type of peripheral doesn't override `passkeyInput()`, the library will reject the pairing.

## Examples

* [Tracker Bike Speed and Heart Rate](examples/tracker-bike_speed-heartrate): use a Bicycle Speed sensor and a Heart Rate monitor to track your speed and heart rate as you bike around town.
* [Read OBD2 on Tracker with Veepeak OBDCheck BLE+](examples/veepeak): use an OBD2 BLE dongle to read CAN parameters from your car and track speed, RPM, and engine temperature associated with your car's location.
* [Remotely view and control a food smoker](examples/masterbuilt-smoker): view the temperature of food and smoker, as well as remotely change the temperature and timer.
* [Get readings from Glucose Monitors](examples/glucose-meter): get the readings from a glucose meter and publish the last 5 to the cloud.

## Library Design

The BLE specification takes a modular approach to building a device. Peripherals are structured like this:

* Peripheral includes one or more services
* Service includes one or more characteristics
* Characteristic data may be written to, read from (polled), or sent from the device (pushed). A characteristic may support one or more of these methods of data transfer

This library takes a similar modular approach with the following goals:

* Application only needs to know about the peripheral type APIs
* Characteristics and services can be reused when adding peripheral types
* Easily add new peripheral types, characteristics, or services

### Characteristics

The definition for characteristics are found in the `src/characteristics` directory. The characteristics can be ones that are defined by the Bluetooth SIG standards, or custom characteristics for a specific device type. Standards based characteristics have a 16-bit UUID, which is assigned by the Bluetooth SIG and can be found here: [Characteristics assigned numbers](https://www.bluetooth.com/specifications/gatt/characteristics/). Custom characteristics have a 128-bit UUID that is selected by the manufacturer of the peripheral device.

### Services

The definition for services are found in the `src/services` directory. Just like with characteristics, both 16-bit standards based and 128-bit custom UUIDs are supported. The services defined by the Bluetooth SIG can be found here: [Services assigned numbers](https://www.bluetooth.com/specifications/gatt/services/)

### Peripheral types

The top-level is the definition of peripheral types, and can be found in the `src/peripherals` directory. A header file for each device type should be placed here, and these are the only APIs that the application needs to be aware of. The peripheral type implementation should abstract the service and characteristics complexities.

## Adding new device types

To add a new type of device to connect to, follow these steps:

* Create any needed characteristics in `src/characteristics`
* Create any needed services in `src/services`
* In `src/peripherals/<new-peripheral-type>.h`, derive the BleDevice class
* The derived BleDevice class should expose methods to read/set characteristics as appropriate, and must override the `getType()` function.
* If a characteristic is `NOTIFY` or `INDICATE`, the class should have a `setNewValueCallback()` function, which accepts a callback to be called when there’s a new value


## LICENSE
Copyright 2020 Mariano Goluboff

Licensed under the MIT license
