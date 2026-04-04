#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#include "basic/basic.h"
#include "host/host.h"
#include "eeprom/eeprom.h"
#include "keyboard/keyboard.h"     // Сначала keyboard
#include "launcher/launcher.h"     // Потом launcher

#include "lang/russian_strings.h"
#include "serialloader/serial_loader.h"

#include "config.h"

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

static unsigned char inputBuf[64];
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];
char autorun = 0;

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

    Serial.begin(9600);

    lcd.begin(20, 4);
    lcd.clear();

    Wire.begin();
    Wire.setClock(100000);
    //delay(100);

    int eeprom_status = eeprom_init();
    eeprom_initialized = (eeprom_status == 0); // Предположим, что 0 означает успешную инициализацию

    //host_outputInt(eeprom_status);
    //host_outputFreeMem(freeRam());
    //host_newLine();

    //host_showBuffer();
    //host_startupTone();

    // Запускаем лаунчер при старте
    launcher_init();
    launcher_run();
}

void loop() {
    // Ничего не делаем автоматически
    // Ожидаем ввода пользователя
}