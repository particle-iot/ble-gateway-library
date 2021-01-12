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
        ctx->_time_offset = data[position+1] << 8 | data[position];
        position += 2;
    } else {
        ctx->_time_offset = 0;
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

int GlucoseMeasurement::getConcentration(float& gl) const {return ieee11073_20601a_sfloat(_concentration, gl);}