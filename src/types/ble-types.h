#pragma once

#include "Particle.h"

/** 
 * Add include files for new types here 
 */
#include "types/pulse-oximeter.h"
#include "types/blood-pressure-monitor.h"
#include "types/heart-rate-monitor.h"
#include "types/cycling-sensor.h"


class BleTypes {
public:
    /**
     *  Add new device type pointer creation here. 
     */
    static std::shared_ptr<BleDevice> getDevicePointer(BleUuid &uuid, const BleScanResult *scanResult)
    {
        if (uuid == JUMPER_PULSEOX_SERVICE)
        {
            return std::make_shared<PulseOx>(scanResult->address);
        }
        else if (uuid == BLE_SIG_UUID_HEART_RATE_SVC)
        {
            return std::make_shared<HeartRateMonitor>(scanResult->address);
        }
        else if (uuid == BLE_SIG_UUID_BLOOD_PRESSURE_SVC)
        {
            return std::make_shared<BloodPressureMonitor>(scanResult->address);
        }
        else if (uuid == BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC)
        {
            return std::make_shared<CyclingSpeedAndCadence>(scanResult->address);
        }
        else
        {
            return nullptr;
        }
    }
};