/*
 * Copyright (c) 2020 Particle Industries, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Particle.h"

#include "tracker_config.h"
#include "tracker.h"

#include "ble-gateway.h"
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
    if (!myELM327.begin(dev)) {
        Log.info("Could not connect to ELM327");
    }
  } 
}

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
    BleDeviceGateway::instance().enableService("VEEPEAK", VEEPEAK_SERVICE);

    Particle.connect();
}

void loop()
{
     Tracker::instance().loop();
     BleDeviceGateway::instance().loop();
}