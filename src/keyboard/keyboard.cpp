#include <Wire.h>

#define CARDKB_ADDR 0x5F

char readKey() {
    Wire.requestFrom(CARDKB_ADDR, 1);
    if (Wire.available()) {
        char c = Wire.read();
        // CardKB отправляет функциональные клавиши как 0x81, 0x82 и т.д.
        // Оставляем их как есть для обработки в лаунчере
        return c;
    }
    return 0;
}