#pragma once

#include "Particle.h"

/** 
 * Add include files for new types here 
 */
#include "peripherals/pulse-oximeter.h"
#include "peripherals/heart-rate-monitor.h"
#include "peripherals/cycling-sensor.h"
#include "peripherals/masterbuilt-smoker.h"
#include "peripherals/veepeak-obdcheck.h"
//#include "peripherals/blood-pressure-monitor.h"  // This needs LESC to work


class BleTypes {
public:
    /**
     *  Add new device type pointer creation here. 
     */
    static std::shared_ptr<BleDevice> getDevicePointer(BleUuid &uuid, const BleScanResult *scanResult)
    {
        if (uuid == JUMPER_PULSEOX_SERVICE)
        {
            return std::make_shared<PulseOx>(scanResult->address());
        }
        else if (uuid == BLE_SIG_UUID_HEART_RATE_SVC)
        {
            return std::make_shared<HeartRateMonitor>(scanResult->address());
        }
/*  Remove until LESC is supported
        else if (uuid == BLE_SIG_UUID_BLOOD_PRESSURE_SVC)
        {
            return std::make_shared<BloodPressureMonitor>(scanResult->address());
        } */
        else if (uuid == BLE_SIG_UUID_CYCLING_SPEED_CADENCE_SVC)
        {
            return std::make_shared<CyclingSpeedAndCadence>(scanResult->address());
        }
        else if (uuid == MASTERBUILT_SMOKER_SERVICE)
        {
            return std::make_shared<MasterbuiltSmoker>(scanResult->address());
        }
        else if (uuid == VEEPEAK_SERVICE)
        {
            return std::make_shared<VeepeakObd>(scanResult->address());
        }
        else
        {
            return nullptr;
        }
    }
};