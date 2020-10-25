#pragma once

#include "Particle.h"
#include "characteristics/blood_pressure_measurement.h"


class BloodPressureService
{
private:
    std::unique_ptr<BloodPressureMeasurement> _bpMeasurement;
    std::unique_ptr<IntermediateCuffPressureChar> _intermediateCuff;
    std::unique_ptr<BloodPressureFeatureChar> _bpFeature;
    BlePeerDevice _peer;
public:
    BleService service;
    void onConnect();
    /**
     *  Access data as last reported by the device 
     */
    int getSystolic(float& bp) const {return (_bpMeasurement) ? _bpMeasurement->getSystolic(bp) : -1;}
    int getDiastolic(float& bp) const {return (_bpMeasurement) ? _bpMeasurement->getDiastolic(bp) : -1;}
    int getMeanArterial(float& bp) const {return (_bpMeasurement) ? _bpMeasurement->getMeanArterial(bp) : -1;}
    int getIntermediateCuffPressure(float& bp) const {return (_intermediateCuff) ? _intermediateCuff->getCuffPressure(bp) : -1;}
    /**
     *  Register callback for when new data is available
     */
    void setNewValueCallback(void (*callback)(BleUuid, void*), void* context);

    bool isBodyMovementDetectionSupported() const {return (_bpFeature) ? _bpFeature->isBodyMovementDetectionSupported() : false;}
    bool isCuffFitDetectionSupported() const {return (_bpFeature) ? _bpFeature->isCuffFitDetectionSupported() : false;}
    bool isIrregularPulseDetectionSupported() const {return (_bpFeature) ? _bpFeature->isIrregularPulseDetectionSupported() : false;}
    bool isPulseRateRangeDetectionSupported() const {return (_bpFeature) ? _bpFeature->isPulseRateRangeDetectionSupported() : false;}
    bool isMeasurementPositionDetectionSupported() const {return (_bpFeature) ? _bpFeature->isMeasurementPositionDetectionSupported() : false;}
    bool isMultipleBondSupported() const {return (_bpFeature) ? _bpFeature->isMultipleBondSupported() : false;}

    BloodPressureService(BleService& serv, BlePeerDevice& peer): _peer(peer), service(serv) {};
};