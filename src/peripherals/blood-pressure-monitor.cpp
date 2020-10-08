#include "blood-pressure-monitor.h"

void BloodPressureMonitor::onConnect()
{
    peer.discoverAllServices();
    for (auto& serv: peer.services()) {
        if (serv.UUID() == ORG_BLUETOOTH_SERVICE_BLOOD_PRESSURE) {
            bpService = std::make_unique<BloodPressureService>(serv, peer);
        }
    }
}