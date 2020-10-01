#include "Particle.h"
#include "characteristics/heart_rate.h"

class HeartRateService 
{
private:
    std::unique_ptr<HeartRateMeasurement> _hrMeasurement;
    std::unique_ptr<BodySensorLocation> _sensorLocation; 
    std::unique_ptr<HeartRateControlPoint> _controlPoint;

public:
    BleService service;
    void onConnect();
    uint16_t getHeartRate() {return (_hrMeasurement) ? _hrMeasurement->getHeartRate() : 0;}
    uint16_t getEnergyExpended() {return (_hrMeasurement) ? _hrMeasurement->getEnergyExpended() : 0;}
    void setNewValueCallback(void (*callback)(BleUuid, void*), void* context);
    BodySensorLocation::SensorLocation getSensorLocation() {return (_sensorLocation) ? _sensorLocation->read() : BodySensorLocation::SensorLocation::ERROR;}
    int resetEnergyExpended() {return (_controlPoint) ? _controlPoint->resetEnergyExpended() : -1;}
    int enableNotification() {return (_hrMeasurement) ?_hrMeasurement->enableNotification() : -1;}

    HeartRateService(BleService serv): service(serv) {}
    ~HeartRateService() {}
};