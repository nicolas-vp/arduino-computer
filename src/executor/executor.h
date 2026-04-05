#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <Arduino.h>
#include "../basic/basic.h"
#include "../config.h"

// Запуск программы из EEPROM
bool executor_run_program(const char* filename);

// Запуск программы из буфера
bool executor_run_buffer(const uint8_t* buffer, size_t size);

#endif