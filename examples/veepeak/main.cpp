#include "Particle.h"

#include "tracker_config.h"
#include "tracker.h"

#include "ble-gateway.h"
#include "peripherals/veepeak-obdcheck.h"
#include "ELMduino.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

PRODUCT_ID(TRACKER_PRODUCT_ID);
PRODUCT_VERSION(TRACKER_PRODUCT_VERSION);

SerialLogHandler logHandler(115200, LOG_LEVEL_TRACE, {
    { "app.gps.nmea", LOG_LEVEL_INFO },
    { "app.gps.ubx",  LOG_LEVEL_INFO },
    { "ncp.at", LOG_LEVEL_INFO },
    { "net.ppp.client", LOG_LEVEL_INFO },
});

ELM327 myELM327;

void onConnect(BleDevice& device)
{
  uint8_t buf[20];
  int len = device.getDeviceName(buf, 20);
  if (len > -1) {
    Log.info("Device Name: %.*s", len, buf);
  }
  if (device.getType() == BleUuid(VEEPEAK_SERVICE)) {
    VeepeakObd& dev = (VeepeakObd&)device;
    Log.info("Connected to Veepeak OBD BLE");

    // ELMduino library expects an object that derives the Stream
    // class to send commands to and receive responses from.
    if (!myELM327.begin(dev)) {
        Log.info("Could not connect to ELM327");
    }
  } 
}

/**
 * This callback will be called when the Tracker is getting ready to publish a location.
 * 
 * Use the writer to add our additional parameters. In this case, we read the engine
 * RPM, the speed of the vehicle, and the engine temperature from the OBD2 port.
 */
void loc_gen_cb(JSONWriter &writer, LocationPoint &point, const void *context)
{
    for (auto& dev : BleDeviceGateway::instance().connectedDevices())
    {
        if (dev->getType() == BleUuid(VEEPEAK_SERVICE)) {
            std::shared_ptr<VeepeakObd> csc = std::static_pointer_cast<VeepeakObd>(dev);
            float tempRPM = myELM327.rpm();
            if (myELM327.status == ELM_SUCCESS){
                writer.name("rpm").value(tempRPM);
            }
            tempRPM = myELM327.mph();
            if (myELM327.status == ELM_SUCCESS) {
                writer.name("mph").value(tempRPM);
            }
            int temp = myELM327.engine_temp();
            if (myELM327.status == ELM_SUCCESS) {
                writer.name("engine_temp").value(temp);
            }
        } 
    }
}

void setup()
{
    Tracker::instance().init();
    Tracker::instance().location.regLocGenCallback(loc_gen_cb);
    BleDeviceGateway::instance().setup();
    BleDeviceGateway::instance().onConnectCallback(onConnect);
    BleDeviceGateway::instance().enableServiceByName(VeepeakObd::bleDevicePtr ,"VEEPEAK");

    Particle.connect();
}

void loop()
{
     Tracker::instance().loop();
     BleDeviceGateway::instance().loop();
}