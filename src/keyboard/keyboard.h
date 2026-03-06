#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Arduino.h>

// Коды клавиш CardKB
#define KEY_ENTER   '\r'   // 0x0D
#define KEY_BACKSPACE 0x08
#define KEY_DELETE  0x7F
#define KEY_ESC     0x1B

// Стрелки
#define KEY_UP      0xB5   // 181
#define KEY_DOWN    0xB6   // 182
#define KEY_LEFT    0xB4  // Left arrow
#define KEY_RIGHT   0xB7  // Right arrow

// Функциональные клавиши FN+1..4
#define KEY_FN1     0x81   // FN+1
#define KEY_FN2     0x82   // FN+2
#define KEY_FN3     0x83   // FN+3
#define KEY_FN4     0x84   // FN+4

// Специальные клавиши для вашего проекта
#define KEY_LAYOUT  0xAF   // 175 - переключение раскладки
#define KEY_ASCII   0x81   // 129 - показать ASCII таблицу
#define KEY_MUSIC   0x82   // 130 - музыкальный плеер
#define KEY_SERIAL  0x83   // 131 - последовательная загрузка

char readKey();

#endif