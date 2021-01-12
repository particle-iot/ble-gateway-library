#pragma once

#include "Particle.h"
#include "characteristics/glucose.h"


class GlucoseService
{
private:
    std::unique_ptr<GlucoseMeasurement> _gMeasurement;
    std::unique_ptr<GlucoseFeatureChar> _gFeature;
    std::unique_ptr<RecordAccessControlPoint> _racp;
    BlePeerDevice _peer;
public:
    BleService service;
    void onConnect();
    /**
     *  Access data as last reported by the device 
     */
    std::unique_ptr<RecordAccessControlPoint>& racp() {return _racp;}
    std::unique_ptr<GlucoseMeasurement>& gl() {return _gMeasurement;}
    /**
     *  Register callback for when new data is available
     */
    void setNewValueCallback(void (*callback)(BleUuid, void*), void* context);


    GlucoseService(BleService& serv, BlePeerDevice& peer): _peer(peer), service(serv) {};
};