# BLE Gateway Library

This library turns a Particle Gen3 device (Tracker, Boron, Argon) into a Bluetooth Low Energy (BLE) Central device. In this mode, it is able to detect and connect to BLE Peripherals, and expose APIs so that your application can get and/or send data to the peripherals, depending on their capabilities.

If the peripheral that you’d like to connect to is already supported by the library, you can use this without any modifications.

If the peripheral is not currently supported, the library is written in a modular format so that it is easy to add your peripheral.

## Usage

For basic usage, you will need to:

* Enable the type of devices you want to connect to

* Register a callback to be notified when a connection happens

```
#include "ble-device-gateway.h"

void setup() {

  BleDeviceGateway::instance().setup();

  BleDeviceGateway::instance().onConnectCallback(onConnect);

  BleDeviceGateway::instance().enableService(JUMPER_PULSEOX_SERVICE);

  BleDeviceGateway::instance().enableService(BLE_SIG_UUID_HEART_RATE_SVC);

}

void loop() {

  BleDeviceGateway::instance().loop();

}
```

The callback function for when a device is connected has the device class as the parameter. You can find out what type of device it is by checking the `getType()` function, like this:

```
void onConnect(BleDevice& device)

{

  if (device.getType() == BleUuid(JUMPER_PULSEOX_SERVICE))  {

    Log.info("Connected to Jumper Pulse Oximeter");

  }

  if (device.getType() == BleUuid(BLE_SIG_UUID_HEART_RATE_SVC))

  {

    HeartRateMonitor& dev = (HeartRateMonitor&)device;

    uint8_t buf[20];

    if (dev.getManufacturerName(buf, 20) > -1) {

      Log.info("Connected to Heart Rate Monitor named: %s", buf);

    }

    Log.info("Battery Level: %d", dev.getBatteryLevel());

  }

}
```

Here is where you also would add the capabilities that your application needs. For example, a Heart Rate Monitor typically notifies once per second of the heart rate, so the Heart Rate Monitor type in the library has an API to register a callback to receive the notifications.

Another device might instead allow you to read or write to it. In that case, the type for that device would have APIs to allow you to do that within your application.

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

The top-level is the definition of peripheral types, and can be found in the `src/types` directory. A header file for each device type should be placed here, and these are the only APIs that the application needs to be aware of. The peripheral type implementation should abstract the service and characteristics complexities.

## Adding new device types

To add a new type of device to connect to, follow these steps:

* Create any needed characteristics in `src/characteristics`

* Create any needed services in src/services

* In `src/types/`, derive the BleDevice class

* In `src/types/ble-types.h`, include the new header file, add new device type pointer creation to the `if...then...else` statements

* The derived BleDevice class should expose methods to read/set characteristics as appropriate

* If a characteristic is `NOTIFY` or `INDICATE`, the class should have a `setNewValueCallback()` function, which accepts a callback to be called when there’s a new value

* Consider adding a `setAlert()` function, and implement the logic in `loop()`, to minimize application complexity


## LICENSE
Copyright 2020 Mariano Goluboff

Licensed under the <insert your choice of license here> license
