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

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

PRODUCT_ID(TRACKER_PRODUCT_ID);
PRODUCT_VERSION(TRACKER_PRODUCT_VERSION);

int32_t wheel_size = 2340;

void onConnect(BleDevice& device)
{
  if (device.getType() == BleUuid(BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC)) {
    CyclingSpeedAndCadence& dev = (CyclingSpeedAndCadence&)device;
    /* Set the wheel size for speed calculations when we connect to CSC */
    dev.setWheelSize(wheel_size);
  }
}

/** 
 * This callback will be called when the Tracker is getting ready to publish a location.
 * 
 * Use the writer to add our additional parameters. In this case, we're looping through
 * the connected devices, and adding Heart Rate and Speed if they are there. This also
 * reports the battery levels if below 15%.
*/
void loc_gen_cb(JSONWriter &writer, LocationPoint &point, const void *context)
{
    for (auto& dev : BleDeviceGateway::instance().connectedDevices())
    {
        if (dev->getType() == BleUuid(BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC)) {
            std::shared_ptr<CyclingSpeedAndCadence> csc = std::static_pointer_cast<CyclingSpeedAndCadence>(dev);
            if (csc->getBatteryLevel() == 0) {
                csc->forceBatteryUpdate();
            }
            writer.name("kmph").value((double)(csc->getSpeed()/1000.0));
            if (csc->getBatteryLevel() < 15) {
                writer.name("csc_batt").value(csc->getBatteryLevel());
            }
        } else if (dev->getType() == BleUuid(BLE_SIG_UUID_HEART_RATE_SVC)) {
            std::shared_ptr<HeartRateMonitor> hr = std::static_pointer_cast<HeartRateMonitor>(dev);
            if (hr->getBatteryLevel() == 0) {
                hr->forceBatteryUpdate();
            }
            writer.name("hr").value(hr->getHeartRate());
            if (hr->getBatteryLevel() < 15) {
                writer.name("hr_batt").value(hr->getBatteryLevel());
            }
        }
    }
}

/**
 * Callback to receive wheel size updates from the Tracker Configuration Service in the Console.
 */
static int set_wheel_size_cb(int32_t value, const void *context)
{
    wheel_size = value;
    for (auto& dev : BleDeviceGateway::instance().connectedDevices())
    {
        if (dev->getType() == BleUuid(BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC)) {
            std::shared_ptr<CyclingSpeedAndCadence> csc = std::static_pointer_cast<CyclingSpeedAndCadence>(dev);
            csc->setWheelSize(wheel_size);
        }
    }
    return 0;
}


void setup()
{
    // Custom configuration items to be managed by Tracker Configuration Service
    static ConfigObject ble_sensors(
        "ble_sensors", {
            ConfigInt("wheel_size", config_get_int32_cb, set_wheel_size_cb, &wheel_size, nullptr, 100, 200000)
        }
    ); 
    Tracker::instance().init();
    Tracker::instance().location.regLocGenCallback(loc_gen_cb);
    Tracker::instance().configService.registerModule(ble_sensors);
    BleDeviceGateway::instance().setup();
    BleDeviceGateway::instance().onConnectCallback(onConnect);
    BleDeviceGateway::instance().enableService(BLE_SIG_UUID_HEART_RATE_SVC);
    BleDeviceGateway::instance().enableService(BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC);

    Particle.connect();
}

void loop()
{
     Tracker::instance().loop();
     BleDeviceGateway::instance().loop();
}