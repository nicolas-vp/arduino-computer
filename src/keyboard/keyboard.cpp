#include <Wire.h>

#define CARDKB_ADDR 0x5F

char readKey() {
    Wire.requestFrom(CARDKB_ADDR, 1);
    if (Wire.available()) {
        char c = Wire.read();
        return c;
    }
    return 0;
}