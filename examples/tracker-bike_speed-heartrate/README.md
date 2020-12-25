# Bicycle Speed and Heart Rate on Tracker

This example shows how to use a Tracker to connect to a Heart Rate monitor (HRM) and a Bicycle speed sensor (CSC).

## Usage

### Associating data to location

Tracker Edge firmware will publish a location object that is stored by the Particle Tracker Services. Any data
included with the location publish will be associated with the location and stored so that it can later be
retrieved in the console or through the Cloud API.

This makes it very easy for this application to then display speed and heart rate at specific points on a bike
trip. In order to add the custom data to the publish, we register a callback that will be called when the
Tracker is getting ready to publish location:

`Tracker::instance().location.regLocGenCallback(loc_gen_cb);`

The callback will then use the `JSONWriter` that is passed to add the custom data.

### Data update period

Both the HRM and CSC will normally send new values once per second. It would be possible for each time 
that we receive new data to publish it. But instead, this application lets the user control how often
to publish with the built-in Configuration Service. Because the Configuration Service already has
an extensive way to configure publish intervals (based on time, radius, movement), there is no special
code that is needed in this application to make a decision on when to publish.

### Viewing the data

After a bike ride, you can go to the Particle Console and look for the current or historical information
of the Tracker on the map. Because we're adding the heart rate and speed to the location object, it will
be associated and displayed like this:

![Console Map View](map.png)

## Sensor information

### Cycling Speed and Cadence Sensor

The sensor used for this example follows the standard defined by the Bluetooth SIG for the 
[Cycling Speed and Cadence Profile](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=261449).
The sensor works by counting the number of times the wheel turns, and keeping a counter. About once per
second, it notifies the Central (our gateway) the number of wheel turns and the time the last wheel 
revolution happened. By comparing with the previous values sent, we can calculate the wheel revolutions
per second. Multiplying that by the wheel circumference, we can calculate the instantaneous speed.

Note that the sensor doesn't need to know the wheel size, so we store that information on our gateway.
You can set the wheel size with this function:

```c++
void setWheelSize(uint16_t mm);
```

and you can retrieve the instantaneous speed in meters per hour with:

```c++
uint32_t getSpeed();
```

### Heart Rate Sensor

The sensor used for this example follows the standard defined by the Bluetooth SIG for the 
[Heart Rate Profile](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=239865).
The sensor gets the heart rate of the user, and notifies the Central once per second with the
heart rate as well as other values. This library stores the latest heart rate value in a variable
and the value can be retrieved with this function:

```c++
uint16_t getHeartRate();
```