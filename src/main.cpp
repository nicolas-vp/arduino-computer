#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "basic/basic.h"
#include "host/host.h"
#include "eeprom/eeprom.h"
#include "launcher/launcher.h"     // Потом launcher
#include "lang/russian_strings.h"
#include "config.h"

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

unsigned char mem[MEMORY_SIZE];

int freeRam() {
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
    lcd.begin(20, 4);
    lcd.clear();

    host_init(BUZZER_PIN);

    reset();

    lcd.begin(20, 4);
    lcd.clear();

    host_outputProgMemDirectString(LOADING);
    host_showBuffer();

    Wire.begin();
    Wire.setClock(100000);

    int eeprom_status = eeprom_init();
    eeprom_initialized = (eeprom_status == 0);

    host_outputFreeMem(freeRam());
    host_outputProgMemDirectString(BYTES_FREE);
    host_showBuffer();

    host_startupTone();

    launcher_init();
    launcher_run();
}

void loop() {
}