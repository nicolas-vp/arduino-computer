#include "serial_loader.h"
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Подключаем необходимые заголовки из проекта
#include "../basic/basic.h"
#include "../host/host.h"
#include "../keyboard/keyboard.h"

// Внешние переменные и функции из basic.cpp
extern int sysPROGEND;
extern uint16_t lineNumber;
extern int tokenize(unsigned char *input, unsigned char *output, int outputSize);
extern unsigned char *findProgLine(uint16_t targetLineNumber);
extern void deleteProgLine(unsigned char *p);
extern int doProgLine(uint16_t lineNumber, unsigned char *tokenPtr, int tokensLength);
extern int processInput(unsigned char *tokenBuf);
extern void listProg(uint16_t first, uint16_t last);
#define TEMP_BUF_SIZE 256
extern unsigned char mem[MEMORY_SIZE];

static bool serialInitialized = false;

void initSerialLoader() {
    if (!serialInitialized) {
        // Serial.begin(115200);
        serialInitialized = true;
        delay(100);
        while (Serial.available()) {
            Serial.read();
        }
    }
}

void handleSerialLoad() {


    initSerialLoader();
    
    // Очищаем экран
    host_cls();
    host_outputString("SERIAL LOADER");
    host_showBuffer();
    
    char lineBuffer[TEMP_BUF_SIZE];
    int linePos = 0;
    bool receiving = true;
    bool errorOccurred = false;
    int lineCount = 0;
    
    memset(lineBuffer, 0, TEMP_BUF_SIZE);
    
    while (receiving && !errorOccurred) {
        if (Serial.available() > 0) {
            char c = Serial.read();
            
            // Проверяем маркер конца ||
            if (c == '|') {
                if (Serial.available()) {
                    char next = Serial.peek();
                    if (next == '|') {
                        Serial.read(); // читаем второй |
                        
                        // Обрабатываем последнюю строку если есть
                        if (linePos > 0) {
                            lineBuffer[linePos] = '\0';
                            
                            // Отображаем полученную строку
                            host_outputString("> ");
                            host_outputString(lineBuffer);
                            host_newLine();
                            host_showBuffer();
                            
                            // Токенизируем
                            unsigned char tokenBuf[TEMP_BUF_SIZE];
                            memset(tokenBuf, 0, TEMP_BUF_SIZE);
                            
                            int ret = tokenize((unsigned char*)lineBuffer, tokenBuf, TEMP_BUF_SIZE);
                            
                            if (ret == ERROR_NONE) {
                                unsigned char *p = tokenBuf;
                                if (*p == TOKEN_INTEGER) {
                                    long lineNum;
                                    memcpy(&lineNum, tokenBuf + 1, sizeof(long));
                                    p += 1 + sizeof(long);
                                    
                                    if (doProgLine((uint16_t)lineNum, p, TEMP_BUF_SIZE - (p - tokenBuf))) {
                                        lineCount++;
                                        host_outputString("  + OK");
                                        host_newLine();
                                    }
                                }
                            } else {
                                // Ошибка токенизации
                                host_outputString("  ERROR: ");
                                if (lineNumber != 0) {
                                    host_outputInt(lineNumber);
                                    host_outputChar('-');
                                }
                                host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
                                host_newLine();
                                errorOccurred = true;
                            }
                            host_showBuffer();
                        }
                        
                        receiving = false;
                        break;
                    }
                }
            }
            
            // Если встретили разделитель | (не маркер конца)
            if (c == '|') {
                // Обрабатываем накопленную строку
                if (linePos > 0) {
                    lineBuffer[linePos] = '\0';

                    unsigned char tokenBuf[TEMP_BUF_SIZE];
                    memset(tokenBuf, 0, TEMP_BUF_SIZE);
                    
                    int ret = tokenize((unsigned char*)lineBuffer, tokenBuf, TEMP_BUF_SIZE);
                    
                    if (ret == ERROR_NONE) {
                        unsigned char *p = tokenBuf;
                        if (*p == TOKEN_INTEGER) {
                            long lineNum;
                            memcpy(&lineNum, tokenBuf + 1, sizeof(long));
                            p += 1 + sizeof(long);
                            
                            if (doProgLine((uint16_t)lineNum, p, TEMP_BUF_SIZE - (p - tokenBuf))) {
                                lineCount++;
                                host_outputString("  + OK");
                                host_newLine();
                            }
                        }
                    } else {
                        // Ошибка токенизации
                        host_outputString("  ERROR: ");
                        if (lineNumber != 0) {
                            host_outputInt(lineNumber);
                            host_outputChar('-');
                        }
                        host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
                        host_newLine();
                        errorOccurred = true;
                    }
                    host_showBuffer();
                    
                    // Сбрасываем буфер строки
                    linePos = 0;
                    memset(lineBuffer, 0, TEMP_BUF_SIZE);
                }
            }
            // Обычный символ
            else if (c != '\r' && c != '\n') {
                if (linePos < TEMP_BUF_SIZE - 1) {
                    lineBuffer[linePos++] = c;
                }
            }
        }
        
        // Проверка ESC для выхода
        if (readKey() == 27) {
            host_outputString("Cancelled!");
            host_newLine();
            receiving = false;
            errorOccurred = true;
        }
    }
    
    if (!errorOccurred) {
        host_cls();
        host_outputString("Done!");
         host_showBuffer();
     }
    // При ошибке экран НЕ очищаем - оставляем сообщение об ошибке
}