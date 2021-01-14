#include "glucose.h"
#include "ieee11073_20601a.h"

void GlucoseMeasurement::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
    GlucoseMeasurement* ctx = (GlucoseMeasurement *)context;
    size_t position = 0;
    if (len > 0) ctx->_flags = (GlucoseMeasurementFlags)data[position++];
    if (len > (position + 1)) {
        ctx->_sequence_number = data[position+1] << 8 | data[position];
        position += 2;
    }
    if (len > (position + 6)) {
        DateTimeChar::dataReceivedToDateTime(data+position, ctx->dateTime);
        position += 7;
    }
    if (ctx->_flags & GlucoseMeasurementFlags::TIME_OFFSET_PRESENT && len > (position + 1)) {
        ctx->_time_offset = (int16_t)(data[position+1] << 8 | data[position]);
        position += 2;
    }
    if (ctx->_flags & GlucoseMeasurementFlags::CONCENTRATION_TYPE_LOCATION_PRESENT && len > (position + 2)) {
        ctx->_concentration = data[position+1] << 8 | data[position];
        ctx->_type_location = data[position+2];
        position += 3;
    } else {
        ctx->_concentration = 0x07FF;
        ctx->_type_location = 0;
    }
    if (ctx->_flags & GlucoseMeasurementFlags::ANNUNCIATION_PRESENT && len > (position + 2)) {
        ctx->_annunciation = (Annunciation)(data[position+1] << 8 | data[position]);
        position += 2;
    } else {
        ctx->_annunciation = Annunciation::NO_ANNUNCIATION;
    }
    if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(ctx->_characteristic.UUID(), ctx->_notifyContext);
}

void GlucoseMeasurement::onConnect() {
    _characteristic.onDataReceived(onDataReceived, this);
};

void GlucoseMeasurement::notifyCallback(void (*callback)(BleUuid, void*), void* context) {
    _notifyNewData = callback;
    _notifyContext = context;
    _characteristic.subscribe(true);
}

uint8_t GlucoseMeasurement::units() const {
    if ( (_flags & GlucoseMeasurementFlags::CONCENTRATION_TYPE_LOCATION_PRESENT) == 0) return 3;
    return ( (_flags & GlucoseMeasurementFlags::CONCENTRATION_UNITS_MOL) == 0) ? 0 : 1;
}

uint16_t GlucoseMeasurement::sequence() const { return _sequence_number; };

int GlucoseMeasurement::getConcentration(float& gl) const {
    return ieee11073_20601a_sfloat(_concentration, gl);
}

DateTimeChar::DateTime GlucoseMeasurement::getTime() const {
    return dateTime + _time_offset;
}

GlucoseMeasurement::Type GlucoseMeasurement::getType() const {
    return (GlucoseMeasurement::Type)(_type_location & 0xf);
}

GlucoseMeasurement::Location GlucoseMeasurement::getLocation() const {
    return (GlucoseMeasurement::Location)((_type_location & 0xf0) >> 4);
}

void RecordAccessControlPoint::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
    RecordAccessControlPoint* ctx = (RecordAccessControlPoint *)context;
    ctx->responseCode = (len > 2) ? (OpCode)data[0] : OpCode::RESERVED_FUTURE_USE;
    if (ctx->responseCode == OpCode::RESPONSE_NUMBER_OF_RECORDS) {
        ctx->numberOfRecords = data[1] << 8 | data[2];
    }
    else if ( ctx->responseCode == OpCode::RESPONSE_CODE ) {
        ctx->requestCode = (OpCode)data[1];
        ctx->responseCodeValue = (ResponseCodeValues)data[2];
    }

    if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(ctx->_characteristic.UUID(), ctx->_notifyContext);
}