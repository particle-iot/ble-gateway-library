#include "Particle.h"
#include "characteristics/heart_rate.h"

class HeartRateService 
{
private:
    std::unique_ptr<HeartRateMeasurement> _hrMeasurement;
    std::unique_ptr<BodySensorLocation> _sensorLocation; 
    std::unique_ptr<HeartRateControlPoint> _controlPoint;
    BlePeerDevice _peer;
public:
    BleService service;
    void onConnect();
    /**
     *  Access data as last reported by the device 
     */
    uint16_t getHeartRate() {return (_hrMeasurement) ? _hrMeasurement->getHeartRate() : 0;}
    uint16_t getEnergyExpended() {return (_hrMeasurement) ? _hrMeasurement->getEnergyExpended() : 0;}
    int resetEnergyExpended() {return (_controlPoint) ? _controlPoint->resetEnergyExpended() : -1;}  
    BodySensorLocation::SensorLocation getSensorLocation() {return (_sensorLocation) ? _sensorLocation->read() : BodySensorLocation::SensorLocation::ERROR;}
    
    /**
     *  Register callback for when new data is available
     */
    void setNewValueCallback(void (*callback)(BleUuid, void*), void* context);

    HeartRateService(BleService& serv, BlePeerDevice& peer): _peer(peer), service(serv) {}
};