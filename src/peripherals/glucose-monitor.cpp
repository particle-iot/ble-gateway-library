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
        ctx->glValues.append(m);
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
    if (!_gService || !_gService->racp()) return -2;
    _gService->racp()->sendCommand(RecordAccessControlPoint::OpCode::REPORT_NUMBER_OF_RECORDS, RecordAccessControlPoint::Operator::ALL_RECORDS);
    if (os_semaphore_take(_blockSemaphore, timeout_ms, false)) return -1;
    return (_gService && _gService->racp()) ? _gService->racp()->numberOfRecords : -2;
}

Vector<GlucoseMonitor::Measurement>& GlucoseMonitor::getMeasurements(uint16_t timeout_ms) {
    if (!_gService || !_gService->racp()) return glValues;;
    _gService->racp()->sendCommand(RecordAccessControlPoint::OpCode::REPORT_STORED_RECORDS, RecordAccessControlPoint::Operator::ALL_RECORDS);
    if (os_semaphore_take(_blockSemaphore, timeout_ms, false)) return glValues;
    return glValues;
}

void GlucoseMonitor::flushBuffer() { glValues.clear(); }

Vector<GlucoseMonitor::Measurement>& GlucoseMonitor::getBufferedMeasurements() { return glValues; }

int GlucoseMonitor::getBatteryLevel() {
    return _battService ? _battService->getBatteryLevel() : -1;
}