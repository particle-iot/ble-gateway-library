#include "Particle.h"

class CurrentTime: public BleCharacteristic
{
private:
    static const BleUuid _uuid(0x2A2B);

public:
    BleUuid getUuid() {return _uuid;};
};