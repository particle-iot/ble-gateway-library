#include "peripherals/pulse-oximeter.h"

const BleUuid pulseOxCharUuid("cdeacb81-5235-4c07-8846-93a37ee6b86d");

void PulseOx::onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
    PulseOx* pox = (PulseOx *)context;
    if (data[0] == 0x81)
    {
        uint8_t hr = data[1];
        uint8_t spo = data[2];

        if (hr > 0 && spo > 0)
        {
            pox->_hr = hr;
            pox->_spo = spo;
        }
    }
}

void PulseOx::onConnect()
{
    peer.discoverAllServices();
    peer.getServiceByUUID(dataService, BleUuid(JUMPER_PULSEOX_SERVICE));
    Vector<BleCharacteristic> ch = peer.discoverCharacteristicsOfService(dataService);
    if (!peer.getCharacteristicByUUID(dataService, dataChar, pulseOxCharUuid) ) {
        Log.info("Didn't find characteristic");
    }
    dataChar.onDataReceived(onDataReceived, this);
    dataChar.subscribe(true);
}

void PulseOx::loop()
{
    for (int i = 0; i < _alerts.size(); i++)
    {
        PulseOxAlertType send_alert = PulseOxAlertType::NONE;
        PulseOxAlert alert = _alerts.takeAt(i);
        if (alert.type == PulseOxAlertType::SPO_LOW && getSpo() > 0)
        {
            if (getSpo() > alert.min)
            {
                alert.cleared = true;
            }
            else
            {
                if (alert.period > 0)
                {
                    if (System.uptime() - alert.last_published > alert.period)
                    {
                        send_alert = PulseOxAlertType::SPO_LOW;
                    }
                }
                else
                {
                    if (alert.cleared)
                    {
                        alert.cleared = false;
                        send_alert = PulseOxAlertType::SPO_LOW;
                    }
                }
            }
        }
        else if (alert.type == PulseOxAlertType::HR_LOW_HIGH && getHr() > 0)
        {
            if (getHr() > alert.min && getHr() < alert.max)
            {
                alert.cleared = true;
            }
            else
            {
                if (alert.period > 0)
                {
                    if (System.uptime() - alert.last_published > alert.period) send_alert = PulseOxAlertType::HR_LOW_HIGH;
                }
                else
                {
                    if (alert.cleared)
                    {
                        alert.cleared = false;
                        send_alert = PulseOxAlertType::HR_LOW_HIGH;
                    }
                }   
            }
        }
        if (send_alert != PulseOxAlertType::NONE) 
        {
            alert.callback(*this, send_alert);
            alert.last_published = System.uptime();
        }
        _alerts.insert(i, alert);
    }
}

int PulseOx::setAlert(PulseOxAlertType type, PulseOxAlertCallback callback, uint16_t period, uint8_t low, uint8_t high)
{
    PulseOxAlert alert = PulseOxAlert();
    alert.type = type;
    alert.callback = callback;
    alert.min = low;
    alert.max = high;
    alert.period = period;
    _alerts.append(alert);
    return 0;
}
