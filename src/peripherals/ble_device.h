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
    virtual bool pendingData() { return false; };
    /**
     * Called when pairing is complete.
     */
    virtual void onPair(int status) {
        if (status == BLE_GAP_SEC_STATUS_SUCCESS) {
            Log.info("Pairing succeeded");
        } else {
            Log.info("Pairing failed with code: %d", status);
        }
    };
    /**
     * This will be called when passkey input is required by the peripheral, and the Application
     * hasn't set a callback to enter a passkey.
     * 
     * This should be overloaded by a peripheral to hard-code a default passkey value, otherwise
     * it will reject pairing.
     */
    virtual void passkeyInput() {
        BLE.rejectPairing(peer);
    };
    /**
     * This will be called by the Application when it has set a callback to enter a passkey.
     * 
     * @param passkey The passkey entered by the Application
     */
    void passkeyInput(uint8_t* passkey) {
        BLE.setPairingPasskey(peer, passkey);
    };

    BleDevice(BleAddress addr): _gapService(nullptr), address(addr) {};
    
    BleDevice() {};
};