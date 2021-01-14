#include "glucose-monitor.h"

GlucoseMonitor::GlucoseMonitor(BleAddress addr): BleDevice{addr}
{
    os_semaphore_create(&_blockSemaphore, 1, 0);
}

GlucoseMonitor::~GlucoseMonitor()
{
    if (_blockSemaphore) {
        os_semaphore_destroy(_blockSemaphore);
        _blockSemaphore = nullptr;
    }
}

void GlucoseMonitor::onConnect()
{
    peer.discoverAllServices();
    for (auto& serv: peer.services()) {
        if (serv.UUID().type() == BleUuidType::SHORT) {
            switch (serv.UUID().shorted())
            {
            case BLE_SIG_UUID_GENERIC_ACCESS_SVC:
                _gapService = std::make_unique<GapGenericService>(serv, peer);
                _gapService->onConnect();
                break;
            case BLE_SIG_UUID_GLUCOSE_SVC:
                _gService = std::make_unique<GlucoseService>(serv, peer);
                _gService->onConnect();
                _gService->setNewValueCallback(_onNewValue, this);
                break;
            case BLE_SIG_UUID_DEVICE_INFORMATION_SVC:
                _disService = std::make_unique<DeviceInformationService>(serv, peer);
                _disService->onConnect();
                break;
            case BLE_SIG_UUID_BATTERY_SVC:
                _battService = std::make_unique<BatteryService>(serv, peer);
                _battService->onConnect();
                break;
            default:
                break;
            }
        }
    }
    BLE.startPairing(peer);
}

void GlucoseMonitor::loop() {

}

bool GlucoseMonitor::pendingData() {
    return !glValues.isEmpty();
}

void GlucoseMonitor::_onNewValue(BleUuid uuid, void* context) {
    GlucoseMonitor* ctx = (GlucoseMonitor *)context;
    if (uuid == BLE_SIG_RECORD_ACCESS_CONTROL_CHAR) {
        os_semaphore_give(ctx->_blockSemaphore, false);
    }
    else if (uuid == BLE_SIG_GLUCOSE_MEASUREMENT_CHAR) {
        if (!ctx->_gService || !ctx->_gService->gl() || ctx->_gService->gl()->units() > 1) Log.error("No measurement available");
        Measurement m;
        m.mol_per_l = ctx->_gService->gl()->units() == 1;
        m.sequence = ctx->_gService->gl()->sequence();
        ctx->_gService->gl()->getConcentration(m.concentration);
        m.time = ctx->_gService->gl()->getTime();
        m.type = ctx->_gService->gl()->getType();
        m.location = ctx->_gService->gl()->getLocation();
        ctx->glValues.append(m);
    }
    else if (uuid == BLE_SIG_GLUCOSE_MEASUREMENT_CONTEXT_CHAR) {
        Log.trace("Got measurement context callback");
    }
    else {
        if (ctx->_callback != nullptr) ctx->_callback(*ctx, uuid, ctx->_callbackContext);
    }
}

void GlucoseMonitor::setNewValueCallback(NewGlucoseCallback callback, void* context) {
    _callback = callback;
    _callbackContext = context;
    if (_battService != nullptr) _battService->setNewValueCallback(_onNewValue, this);
}

int GlucoseMonitor::getNumberStoredRecords(uint16_t timeout_ms) {
    if (!_gService || !_gService->racp()) return -14;
    /*
     * Glucose Profile requires to write to the RACP Characteristic, and then wait for the notification.
     * Since it's command/response instead of constant (or unprompted) notifications, and in order to 
     * simplify how the Application interacts, here we will do the collection of the notifications,
     * instead of doing a callback. 
     */
    _gService->racp()->sendCommand(RecordAccessControlPoint::OpCode::REPORT_NUMBER_OF_RECORDS, RecordAccessControlPoint::Operator::ALL_RECORDS);
    if (os_semaphore_take(_blockSemaphore, timeout_ms, false)) return -15;   // Timeout waiting for resopnse
    if (!_gService || !_gService->racp()) return -14;                        // Communication error
    // If the device returned an error code, return that value (as a negative). Otherwise return
    // the number of stored records
    if (_gService->racp()->responseCode == RecordAccessControlPoint::OpCode::RESPONSE_CODE) {
        return -((int)_gService->racp()->responseCodeValue);
    }
    return _gService->racp()->numberOfRecords;
}

Vector<GlucoseMonitor::Measurement>& GlucoseMonitor::getMeasurements(uint16_t timeout_ms) {
    int ret = requestMeasurements(timeout_ms, RecordAccessControlPoint::Operator::ALL_RECORDS);
    if (ret != 1) Log.error("Error reading measurements from Glucose Monitor: %d", ret);
    return glValues;
}

Vector<GlucoseMonitor::Measurement>& GlucoseMonitor::getMeasurements(RecordAccessControlPoint::Operator oper, uint16_t min, uint16_t timeout_ms) {
    uint8_t operand[2] = {(uint8_t)(min & 0xff), (uint8_t)( (min & 0xff00) >> 8)};
    int ret = requestMeasurements(timeout_ms, oper, RecordAccessControlPoint::FilterType::SEQUENCE_NUMBER, operand, 2);
    if (ret != 1) Log.error("Error reading measurements from Glucose Monitor: %d", ret);
    return glValues;
}

void GlucoseMonitor::flushBuffer() { glValues.clear(); }

Vector<GlucoseMonitor::Measurement>& GlucoseMonitor::getBufferedMeasurements() { return glValues; }

int GlucoseMonitor::requestMeasurements(uint16_t timeout_ms, RecordAccessControlPoint::Operator oper, RecordAccessControlPoint::FilterType filterType, uint8_t* operand, uint8_t operand_size) {
    /*
     * Glucose Profile requires to write to the RACP Characteristic, and then wait for the notification.
     * Since it's command/response instead of constant (or unprompted) notifications, and in order to 
     * simplify how the Application interacts, here we will do the collection of the notifications,
     * instead of doing a callback. 
     */
    if (!_gService || !_gService->racp()) return -14;
    _gService->racp()->sendCommand(RecordAccessControlPoint::OpCode::REPORT_STORED_RECORDS, oper, filterType, operand, operand_size);
    if (os_semaphore_take(_blockSemaphore, timeout_ms, false)) return -15;
    if (!_gService || !_gService->racp()) return -14;
    return (int)_gService->racp()->responseCodeValue;
}

int GlucoseMonitor::getBatteryLevel() {
    return _battService ? _battService->getBatteryLevel() : -14;
}