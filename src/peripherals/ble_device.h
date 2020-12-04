#pragma once

#include "Particle.h"
#include "services/gap_generic_service.h"


class BleDevice
{
protected:
    std::unique_ptr<GapGenericService> _gapService;
public:
    BleAddress address;
    BlePeerDevice peer;

    /**
     * Get Device Name from the GAP Generic Service
     * 
     * When called, it will start a BLE transaction to retrieve the value.
     * 
     * @param buf The buffer that will be filled the with the value
     * @param len The size of the buffer
     * 
     * @return The length of the value
     * @return Negative if there's an error
     */
    int getDeviceName(uint8_t* buf, size_t len) const {return (_gapService) ? _gapService->getDeviceName(buf, len) : -1;}
    /**
     * Set Device Name from the GAP Generic Service
     * 
     * When called, it will start a BLE transaction to set the value.
     * 
     * @param str Null-terminated buffer with the name to be set
     * 
     * @return The length of the value
     * @return Negative if there's an error
     */
    int setDeviceName(const char* str) const {return (_gapService) ? _gapService->setDeviceName(str) : -1;}
    /**
     * Get Appearance from the GAP Generic Service
     * 
     * When called, it will start a BLE transaction to retrieve the value.
     * 
     * @return The appearance value
     */
    Appearance::Value getAppearance() const {return (_gapService) ? _gapService->getAppearance() : Appearance::Value::NONE;}
    /**
     * Set Appearance from the GAP Generic Service
     * 
     * When called, it will start a BLE transaction to set the value.
     * 
     * @param appearance Appearance value to set
     * 
     * @return Negative if there's an error
     */
    int setAppearance(Appearance::Value appearance) const {return (_gapService) ? _gapService->setAppearance(appearance) : -1;}

    virtual BleUuid getType() = 0;
    virtual void loop() {};
    virtual void onConnect() {};
#if (SYSTEM_VERSION >= SYSTEM_VERSION_v200RC4)
    virtual void onPair() {Log.info("Pairing succeeded");};
    virtual int passkeyInput(uint8_t* passkey) {return -1;};
#endif

    BleDevice(BleAddress addr): _gapService(nullptr), address(addr) {};
    
    BleDevice() {};
};