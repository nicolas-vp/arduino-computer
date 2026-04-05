#ifndef HOST_H
#define HOST_H

#include <stdint.h>
#include <Arduino.h>

#define MAGIC_AUTORUN_NUMBER    0xFC

// Инициализация
void host_init(int buzzerPin);
void host_startupTone();

// Управление экраном
void host_cls();
void host_showBuffer();
void host_moveCursor(int x, int y);
void host_newLine();
void host_outputChar(char c);
void host_outputString(char *str);
void host_outputProgMemString(const char *str);
void host_outputProgMemDirectString(const char *str);
int host_outputInt(long val);
void host_outputFloat(float f);
char *host_floatToStr(float f, char *buf);
void host_outputFreeMem(unsigned int val);

// Ввод с клавиатуры
char *host_readLine();
char host_getKey();
bool host_ESCPressed();

// Управление пинами
void host_digitalWrite(int pin, int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, int mode);

// Время и звук
void host_sleep(long ms);
void host_play(char *str);

// Внутренняя EEPROM Arduino
void host_saveProgram(bool autoexec);
void host_loadProgram();

// Внешняя EEPROM AT24C256
bool host_saveExtEEPROM(const char* filename);
bool host_loadExtEEPROM(const char* filename);
bool host_removeExtEEPROM(const char* filename);
void host_dirExtEEPROM();
void host_extEEPROM_format();

// Функции для управления LCD
void lcd_cursor();
void lcd_noCursor();
void lcd_blink();
void lcd_noBlink();
void lcd_clear();

void lcd_setCursor(int x, int y);
void lcd_print(const char* str);

#endif