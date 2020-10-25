#include "Particle.h"
#include "characteristics/cycling_speed_and_cadence.h"

class CyclingSpeedAndCadenceService
{
private:
    std::unique_ptr<CyclingSpeedAndCadenceChar> _cscMeasurement;
    std::unique_ptr<CSCFeatureChar> _cscFeature; 
    std::unique_ptr<SensorLocation> _sensorLocation; 
    //std::unique_ptr<HeartRateControlPoint> _controlPoint;
    BlePeerDevice _peer;
public:
    BleService service;
    void onConnect();
    uint32_t getWheelRotations() {return (_cscMeasurement) ? _cscMeasurement->getWheelRotations() : 0;}
    uint32_t getCadence() {return (_cscMeasurement) ? _cscMeasurement->getCadence() : 0;}
    uint16_t getLastWheelEvent() {return (_cscMeasurement) ? _cscMeasurement->getLastWheelEvent() : 0;}
    uint16_t getLastCadenceEvent() {return (_cscMeasurement) ? _cscMeasurement->getLastCadenceEvent() : 0;}
    void setNewValueCallback(void (*callback)(BleUuid, void*), void* context);
    SensorLocation::Location getSensorLocation() {return (_sensorLocation) ? _sensorLocation->read() : SensorLocation::Location::ERROR;}
    //int resetEnergyExpended() {return (_controlPoint) ? _controlPoint->resetEnergyExpended() : -1;}
    int enableNotification() {return (_cscMeasurement) ?_cscMeasurement->enableNotification() : -1;}

    CyclingSpeedAndCadenceService(BleService& serv, BlePeerDevice& peer): _peer(peer), service(serv) {}
    ~CyclingSpeedAndCadenceService() {}
};