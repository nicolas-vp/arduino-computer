#ifndef SERIAL_LOADER_H
#define SERIAL_LOADER_H

#include <Arduino.h>

// Константы из basic.h которые нам нужны
#ifndef ERROR_NONE
#define ERROR_NONE 0
#endif

#ifndef TOKEN_INTEGER
#define TOKEN_INTEGER 0x81
#endif

#ifndef TOKEN_EOL
#define TOKEN_EOL 0x14
#endif

// Скорость Serial порта
#define SERIAL_BAUD 115200

// Прототипы функций
void initSerialLoader();
void handleSerialLoad();

#endif