#include "glucose.h"
#include "ieee11073_20601a.h"

void GlucoseMeasurement::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
    GlucoseMeasurement* ctx = (GlucoseMeasurement *)context;
    size_t position = 0;
    if (len > 0) ctx->_flags = (Flags)data[position++];
    if (len > (position + 1)) {
        ctx->_sequence_number = data[position+1] << 8 | data[position];
        position += 2;
    }
    if (len > (position + 6)) {
        DateTimeChar::dataReceivedToDateTime(data+position, ctx->dateTime);
        position += 7;
    }
    if ( (ctx->_flags & Flags::TIME_OFFSET_PRESENT) && len > (position + 1)) {
        ctx->_time_offset = (int16_t)(data[position+1] << 8 | data[position]);
        position += 2;
    }
    if ( (ctx->_flags & Flags::CONCENTRATION_TYPE_LOCATION_PRESENT) && len > (position + 2)) {
        ctx->_concentration = data[position+1] << 8 | data[position];
        ctx->_type_location = data[position+2];
        position += 3;
    } else {
        ctx->_concentration = 0x07FF;
        ctx->_type_location = 0;
    }
    if ( (ctx->_flags & Flags::ANNUNCIATION_PRESENT) && len > (position + 2)) {
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
    if ( (_flags & Flags::CONCENTRATION_TYPE_LOCATION_PRESENT) == Flags::NONE) return 3;
    return ( (_flags & Flags::CONCENTRATION_UNITS_MOL) == Flags::NONE) ? 0 : 1;
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

void GlucoseMeasurementContext::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context) {
    GlucoseMeasurementContext* ctx = (GlucoseMeasurementContext *)context;
    size_t position = 0;
    if (len > 0) ctx->_flags = (Flags)data[position++];
    if (len > (position + 1)) {
        ctx->_sequence_number = data[position+1] << 8 | data[position];
        position += 2;
    }
    if (len > position && ((ctx->_flags & Flags::EXTENDED_FLAGS_PRESENT) != Flags::NONE)) {
        ctx->_extendedFlags = (ExtendedFlags)data[position++];
    }
    if (len > (position + 2) && ((ctx->_flags & Flags::CARBOHYDRATE_ID_PRESENT) != Flags::NONE)) {
        ctx->_carbID = (CarbID)data[position];
        ctx->_carbKG = data[position+2] << 8 | data[position+1];
        position += 3;
    } else {
        ctx->_carbID = CarbID::RESERVED_FUTURE_USE;
        ctx->_carbKG = 0x07FF;
    }
    if (len > position && ((ctx->_flags & Flags::MEAL_PRESENT) != Flags::NONE)) {
        ctx->_meal = (Meal)data[position++];
    } else {
        ctx->_meal = Meal::RESERVED_FUTURE_USE;
    }
    if (len > position && ((ctx->_flags & Flags::TESTER_HEALTH_PRESENT) != Flags::NONE)) {
        ctx->_tester_health = data[position++];
    } else {
        ctx->_tester_health = 0;
    }
    if (len > (position + 2) && ((ctx->_flags & Flags::EXERCISE_DURATION_AND_INTENSITY) != Flags::NONE)) {
        ctx->_exercise_duration = data[position+1] << 8 | data[position];
        ctx->_exercise_intensity = data[position+2];
        position += 3;
    } else {
        ctx->_exercise_duration = 0x07FF;
        ctx->_exercise_intensity = 255;
    }
    if (len > (position + 2) && ((ctx->_flags & Flags::MEDICATION_ID_PRESENT) != Flags::NONE)) {
        ctx->_medicationID = (MedicationID)data[position];
        ctx->_medicationAmount = data[position+2] << 8 | data[position+1];
        position += 3;
    } else {
        ctx->_medicationID = MedicationID::RESERVED_FUTURE_USE;
        ctx->_medicationAmount = 0;
    }
    if (len > (position + 1) && ((ctx->_flags & Flags::HBA1C_PRESENT) != Flags::NONE)) {
        ctx->_hba1c = data[position+1] << 8 | data[position];
        position += 2;
    } else {
        ctx->_hba1c = 0x07FF;
    }
    if (ctx->_notifyNewData != nullptr) (ctx->_notifyNewData)(ctx->_characteristic.UUID(), ctx->_notifyContext);
}

void GlucoseMeasurementContext::onConnect() {
    _characteristic.onDataReceived(onDataReceived, this);
};

void GlucoseMeasurementContext::notifyCallback(void (*callback)(BleUuid, void*), void* context) {
    _notifyNewData = callback;
    _notifyContext = context;
    _characteristic.subscribe(true);
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