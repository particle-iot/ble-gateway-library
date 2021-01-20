#include "ble-gateway.h"
#include "peripherals/glucose-meter.h"

SYSTEM_THREAD(ENABLED);

void setup() {
  // Enable Glucose Monitor
  BleDeviceGateway::instance().enableService(GlucoseMeter::bleDevicePtr, BLE_SIG_UUID_GLUCOSE_SVC);
}

void loop() {
  static char buf[622];
  BleDeviceGateway::instance().loop();

  // Only get the data when connected to the cloud, as that's when we'll be publishing.
  if (Particle.connected()) {
    bool publish = false;
    JSONBufferWriter writer(buf, sizeof(buf) - 1);
    writer.beginObject();
    for (auto& dev : BleDeviceGateway::instance().connectedDevices()) {
      // Make sure that BLE pairing is complete first, so that we can properly write to 
      // the device (i.e. check BLE.isPaired())
      if (dev->getType() == BleUuid(BLE_SIG_UUID_GLUCOSE_SVC) && BLE.isPaired(dev->peer)) {      
        publish = true;
        std::shared_ptr<GlucoseMeter> meter = std::static_pointer_cast<GlucoseMeter>(dev);
        Vector<GlucoseMeter::Measurement> measurements;
        int total = meter->getNumberStoredRecords();

        // Here, we'll check if there's no error in getting the number of measurements.
        // Also, if we have less than 5, then publish all the measurements. If we have 
        // more than 5, publish the last 5.
        if (total > 0 && total < 5) {
          measurements = meter->getMeasurements();
        } else if (total > 0) {
          measurements = meter->getMeasurements(RecordAccessControlPoint::Operator::GREATER_THAN_OR_EQUAL, total - 5);
        }
        // Create an array for each meter. The array will have the measurements.
        writer.name(meter->peer.address().toString()).beginArray();
        for (const auto& m : measurements) {
          writer.beginObject();
          writer.name("seq").value(m.sequence);
          if (m.mol_per_l) {
            writer.name("mol/L").value(m.concentration);
          } else {
            writer.name("kg/L").value(m.concentration);
          }
          writer.endObject();
        }
        writer.endArray();
        // Always flush the measurement buffer after we're done using it.
        meter->flushBuffer();
        // Add the meter's address to the Deny List, so that we don't get data from
        // the same meter twice. Here, it might make sense to have more intelligence,
        // like checking the date of the last measurement, or something similar. But
        // this is good for testing.
        BleDeviceGateway::instance().addToDenyList(meter->peer.address());
        // The disconnect is not necessary, as the user could disconnect by turning
        // the device off. But it can help remove that step.
        BLE.disconnect(dev->peer);
      }
    }
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    if (publish) Particle.publish("glucose", writer.buffer(), PRIVATE);
  }
}