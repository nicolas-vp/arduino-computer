#include <LiquidCrystal.h>
#include <Arduino.h>
#include "../keyboard/keyboard.h"

extern LiquidCrystal lcd;

static int asciiBase = 32;

void printAsciiCell(int x, int y, uint8_t code) {
    lcd.setCursor(x, y);

    if (code < 10) lcd.print("00");
    else if (code < 100) lcd.print("0");

    lcd.print(code);
    lcd.print(":");

    char c = (code >= 32) ? code : ' ';
    lcd.print(c);
}

void drawAsciiScreen() {
    lcd.clear();

    int idx = asciiBase;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            if (idx > 255) return;

            int x = col * 6;
            int y = row;

            printAsciiCell(x, y, idx++);
        }
    }
}

void showAscii() {
    drawAsciiScreen();

    while (1) {
        char c = readKey();
        if (!c) continue;

        // стрелка влево
        if (c == -76) {
            asciiBase -= 12;
            drawAsciiScreen();
        }
        // стрелка вправо
        else if (c == -73) {
            asciiBase += 12;
            drawAsciiScreen();
        }
        // FN или ESC — выход
        else if (c == 0x80 || c == 0x1B) {
            lcd.clear();
            return;
        }
    }
}