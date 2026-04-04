#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <string.h>
#include "host.h"
#include "../basic/basic.h"
#include "../keyboard/keyboard.h"
#include "../utils/utils_ascii.h"
#include "../utils/utils_music.h"
#include "../eeprom/eeprom.h"
#include "../lang/russian_strings.h"
#include "../serialloader/serial_loader.h"

// Уменьшаем размеры буферов
#define TEMP_BUFFER_SIZE 1024  // Размер буфера для программ
#define INPUT_BUF_SIZE 32      // Уменьшили с 40 до 32

static uint8_t temp_buffer[TEMP_BUFFER_SIZE];
static char inputLine[INPUT_BUF_SIZE];

extern LiquidCrystal lcd;

unsigned char screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
unsigned char lineDirty[SCREEN_HEIGHT];
int curX = 0, curY = 0;
char inputMode = 0;

// Уменьшаем историю команд
#define HISTORY_SIZE 3  // Уменьшили с 10 до 3
static char history[HISTORY_SIZE][INPUT_BUF_SIZE];
static int historyCount = 0;
static int historyIndex = -1;
static char savedInput[INPUT_BUF_SIZE];

char buzPin = 8;

const unsigned bytesFreeStr[] PROGMEM = {161, 160, 169, 226, 32, 225, 162, 174, 161, 174, 164, 173, 174, 0};
bool ruMode = false;

struct RuMap {
    unsigned char eng;
    unsigned char rus;
    unsigned char lcd;
};

// Оптимизируем таблицу раскладки - делаем её компактнее
static const RuMap ruMap[] PROGMEM = {
    {'q', 169, 185}, {'w', 230, 229}, {'e', 227, 121},
    {'r', 170, 186}, {'t', 165, 101}, {'y', 173, 189},
    {'u', 163, 180}, {'i', 232, 193}, {'o', 233, 230},
    {'p', 167, 183}, {'a', 228, 228}, {'s', 235, 195},
    {'d', 162, 179}, {'f', 160, 97}, {'g', 175, 190},
    {'h', 224, 112}, {'j', 174, 111}, {'k', 171, 187},
    {'l', 164, 227}, {'z', 239, 199}, {'x', 231, 192},
    {'c', 225, 99}, {'v', 172, 188}, {'b', 168, 184},
    {'n', 226, 191}, {'m', 236, 196}, {',', 161, 178},
    {'.', 238, 198}, {'7', 229, 120}, {'8', 234, 194},
    {'9', 166, 182}, {'0', 237, 197},
    {'Q', 137, 166}, {'W', 150, 225}, {'E', 147, 169},
    {'R', 138, 75}, {'T', 133, 69}, {'Y', 141, 72},
    {'U', 131, 161}, {'I', 152, 172}, {'O', 153, 226},
    {'P', 135, 164}, {'A', 148, 170}, {'S', 155, 174},
    {'D', 130, 66}, {'F', 128, 65}, {'G', 143, 168},
    {'H', 144, 80}, {'J', 142, 79}, {'K', 139, 167},
    {'L', 132, 224}, {'Z', 159, 177}, {'X', 151, 171},
    {'C', 145, 67}, {'V', 140, 77}, {'B', 136, 165},
    {'N', 146, 84}, {'M', 156, 98}, {'<', 129, 160},
    {'>', 158, 176}, {'&', 149, 88}, {'*', 154, 173},
    {'(', 134, 163}, {')', 157, 175},
    {240, 240, 126}
};

// Объявление функций
unsigned char mapLayout(unsigned char c);
unsigned char mapLcdChar(unsigned char c);


unsigned char mapLayout(unsigned char c) {
    if (!ruMode) return c;
    int arraySize = sizeof(ruMap)/sizeof(RuMap);
    for (int i = 0; i < arraySize; i++) {
        RuMap m;
        memcpy_P(&m, &ruMap[i], sizeof(RuMap));
        if (c == m.eng) return m.rus;
    }
    return c;
}

unsigned char mapLcdChar(unsigned char c) {
    if (c <= 127) return c;
    int arraySize = sizeof(ruMap)/sizeof(RuMap);
    for (int i = 0; i < arraySize; i++) {
        RuMap m;
        memcpy_P(&m, &ruMap[i], sizeof(RuMap));
        if (c == m.rus) return m.lcd;
    }
    return c;
}

void host_init(int buzzerPin) {
    buzPin = buzzerPin;
    lcd.clear();
    memset(screenBuffer, ' ', sizeof(screenBuffer));
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curX = curY = 0;
    if (buzPin) pinMode(buzPin, OUTPUT);
}

void host_startupTone() {
    if (!buzPin) return;
    for (int i=1; i<=2; i++) {
        for (int j=0; j<50*i; j++) {
            digitalWrite(buzPin, HIGH);
            delay(3-i);
            digitalWrite(buzPin, LOW);
            delay(3-i);
        }
        delay(100);
    }    
}

void host_cls() {
    memset(screenBuffer, ' ', sizeof(screenBuffer));
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curX = curY = 0;
    lcd.clear();
}

void host_moveCursor(int x, int y) {
    curX = constrain(x, 0, SCREEN_WIDTH - 1);
    curY = constrain(y, 0, SCREEN_HEIGHT - 1);
}

// Упрощенная прокрутка без сохранения в буфер
void scrollScreen() {
    memmove(screenBuffer,
            screenBuffer + SCREEN_WIDTH,
            SCREEN_WIDTH * (SCREEN_HEIGHT - 1));
    memset(screenBuffer + SCREEN_WIDTH * (SCREEN_HEIGHT - 1),
           ' ', SCREEN_WIDTH);
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curY--;
}

void host_showBuffer() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // Всегда обновляем строку с курсором в режиме ввода
        if (lineDirty[y] || (inputMode && y == curY)) {
            lcd.setCursor(0, y);
            
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                unsigned char c = screenBuffer[y * SCREEN_WIDTH + x];
                if (c < 32) c = ' ';
                c = mapLcdChar(c);
                lcd.write(c);
            }
            lineDirty[y] = 0;
        }
    }
}

void host_outputChar(char c) {
    int pos = curY * SCREEN_WIDTH + curX;
    screenBuffer[pos] = c;
    lineDirty[curY] = 1;
    curX++;
    if (curX >= SCREEN_WIDTH) {
        curX = 0;
        curY++;
        if (curY >= SCREEN_HEIGHT) {
            scrollScreen();
            curY = SCREEN_HEIGHT - 1;
        }
    }
}

void host_outputCharDirect(char c) {
    int pos = curY * SCREEN_WIDTH + curX;
    screenBuffer[pos] = c;
    lineDirty[curY] = 1;
    curX++;
}

void host_outputString(char *str) {
    while (*str) host_outputChar(*str++);
}

void host_newLine() {
    curX = 0;
    curY++;
    if (curY >= SCREEN_HEIGHT) {
        scrollScreen();
        curY = SCREEN_HEIGHT - 1;
    }
    memset(screenBuffer + curY * SCREEN_WIDTH, ' ', SCREEN_WIDTH);
    lineDirty[curY] = 1;
}

void switchLayout() {
    ruMode = !ruMode;
    lcd.setCursor(SCREEN_WIDTH - 3, 0);
    if (ruMode) {
        lcd.write(146); lcd.write('P'); lcd.write(169);
    } else {
        lcd.write(147); lcd.write('E'); lcd.write('N');
    }
}

void addToHistory(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return;
    if (historyCount > 0 && strcmp(cmd, history[historyCount - 1]) == 0) return;
    
    // Простое кольцевое переполнение
    if (historyCount < HISTORY_SIZE) {
        strncpy(history[historyCount], cmd, INPUT_BUF_SIZE - 1);
        history[historyCount][INPUT_BUF_SIZE - 1] = 0;
        historyCount++;
    } else {
        // Сдвигаем все записи
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strcpy(history[i], history[i+1]);
        }
        strncpy(history[HISTORY_SIZE-1], cmd, INPUT_BUF_SIZE - 1);
        history[HISTORY_SIZE-1][INPUT_BUF_SIZE - 1] = 0;
    }
    historyIndex = -1;
}

static bool ensure_eeprom() {
    extern bool eeprom_initialized;
    extern int eeprom_init();
    if (!eeprom_initialized) {
        int status = eeprom_init();
        if (status != EEPROM_OK && status != EEPROM_OK_EMPTY) {
            host_outputProgMemString(EEPROM_NOT_AVAILABLE);
            host_newLine();
            host_showBuffer();
            return false;
        }
    }
    return true;
}

char *host_readLine() {
    inputMode = 1;
    lcd.cursor();
    lcd.blink();
    if (curX != 0) host_newLine();
    lcd.setCursor(curX, curY);
    int inputLen = 0;
    inputLine[0] = 0;
    historyIndex = -1;

    while (1) {
        unsigned char c = readKey();
        if (!c) continue;

        // Навигация по истории команд (влево/вправо)
        if (c == KEY_LEFT && historyCount > 0) {
            if (historyIndex == -1) {
                strncpy(savedInput, inputLine, INPUT_BUF_SIZE - 1);
                savedInput[INPUT_BUF_SIZE - 1] = 0;
            }
            if (historyIndex < historyCount - 1) {
                historyIndex++;
                int histPos = historyCount - 1 - historyIndex;
                strncpy(inputLine, history[histPos], INPUT_BUF_SIZE - 1);
                inputLine[INPUT_BUF_SIZE - 1] = 0;
                inputLen = strlen(inputLine);
                memset(screenBuffer + curY * SCREEN_WIDTH, ' ', SCREEN_WIDTH);
                for (int i = 0; i < inputLen && i < SCREEN_WIDTH; i++) {
                    screenBuffer[curY * SCREEN_WIDTH + i] = inputLine[i];
                }
                curX = min(inputLen, SCREEN_WIDTH - 1);
                lineDirty[curY] = 1;
                host_showBuffer();
                lcd.setCursor(curX, curY);
            }
            continue;
        }

        if (c == KEY_RIGHT) {
            if (historyIndex > 0) {
                historyIndex--;
                int histPos = historyCount - 1 - historyIndex;
                strncpy(inputLine, history[histPos], INPUT_BUF_SIZE - 1);
                inputLine[INPUT_BUF_SIZE - 1] = 0;
                inputLen = strlen(inputLine);
            } else if (historyIndex == 0) {
                historyIndex = -1;
                strncpy(inputLine, savedInput, INPUT_BUF_SIZE - 1);
                inputLine[INPUT_BUF_SIZE - 1] = 0;
                inputLen = strlen(inputLine);
            } else {
                continue;
            }
            memset(screenBuffer + curY * SCREEN_WIDTH, ' ', SCREEN_WIDTH);
            for (int i = 0; i < inputLen && i < SCREEN_WIDTH; i++) {
                screenBuffer[curY * SCREEN_WIDTH + i] = inputLine[i];
            }
            curX = min(inputLen, SCREEN_WIDTH - 1);
            lineDirty[curY] = 1;
            host_showBuffer();
            lcd.setCursor(curX, curY);
            continue;
        }

        // Убрали обработку KEY_UP и KEY_DOWN для скролла экрана
        // Теперь стрелки вверх/вниз просто игнорируются в режиме ввода
        if (c == KEY_UP || c == KEY_DOWN) {
            continue;
        }

        if (c == KEY_ENTER) {
            // Сохраняем введенную команду
            if (inputLen > 0) {
                addToHistory(inputLine);
            }
            host_showBuffer();
            break;
        }

        if ((int)c == 175) {
            switchLayout();
            host_showBuffer();
            continue;
        }
        if ((int)c == 129) {
            showAscii();
            continue;
        }
        if ((int)c == 130) {
            musicPlayer();
            continue;
        }
        if ((int)c == 131) {
            handleSerialLoad();
            continue;
        }

        if ((c == 0x08 || c == 0x7F)) {
            historyIndex = -1;
            if (inputLen > 0) {
                inputLen--;
                if (curX > 0) {
                    curX--;
                } else if (curY > 0) {
                    curY--;
                    curX = SCREEN_WIDTH - 1;
                }
                screenBuffer[curY * SCREEN_WIDTH + curX] = ' ';
                inputLine[inputLen] = 0;
            }
            host_showBuffer();
            lcd.setCursor(curX, curY);
            continue;
        }

        if (c >= 32 && c <= 126 && inputLen < INPUT_BUF_SIZE - 1) {
            historyIndex = -1;
            c = mapLayout(c);
            inputLine[inputLen++] = c;
            inputLine[inputLen] = 0;
            screenBuffer[curY * SCREEN_WIDTH + curX] = c;
            curX++;
            if (curX >= SCREEN_WIDTH) {
                curX = 0;
                curY++;
                if (curY >= SCREEN_HEIGHT) {
                    scrollScreen();
                    curY = SCREEN_HEIGHT - 1;
                }
            }
            host_showBuffer();
            lcd.setCursor(curX, curY);
            continue;
        }
    }

    inputLine[inputLen] = 0;
    inputMode = 0;
    lcd.noCursor();
    lcd.noBlink();
    host_showBuffer();
    return inputLine;
}

char host_getKey() {
    char c = readKey();
    return (c >= 32 && c <= 126) ? c : 0;
}

bool host_ESCPressed() {
    return (readKey() == 0x1B);
}

int host_outputInt(long num) {
    char buf[12];  // Уменьшили с 16 до 12
    ltoa(num, buf, 10);
    host_outputString(buf);
    return strlen(buf);
}

void host_outputFloat(float f) {
    char buf[12];  // Уменьшили с 16 до 12
    dtostrf(f, 1, 3, buf);
    host_outputString(buf);
}

void host_outputFreeMem(unsigned int val) {
    host_newLine();
    host_outputInt(val);
    host_outputChar(' ');
    // host_outputProgMemString(bytesFreeStr);
}

void host_saveProgram(bool autoexec) {
    EEPROM.write(0, autoexec ? MAGIC_AUTORUN_NUMBER : 0x00);
    EEPROM.write(1, sysPROGEND & 0xFF);
    EEPROM.write(2, (sysPROGEND >> 8) & 0xFF);
    for (int i = 0; i < sysPROGEND; i++) EEPROM.write(3 + i, mem[i]);
}

void host_loadProgram() {
    sysPROGEND = EEPROM.read(1) | (EEPROM.read(2) << 8);
    for (int i = 0; i < sysPROGEND; i++) mem[i] = EEPROM.read(i + 3);
}

char *host_floatToStr(float f, char *buf) {
    float a = fabs(f);
    if (f == 0.0f) {
        buf[0] = '0';
        buf[1] = 0;
    } else if (a < 0.0001 || a > 1000000) {
        dtostre(f, buf, 6, 0);
    } else {
        int decPos = 7 - (int)(floor(log10(a)) + 1.0f);
        if (decPos < 0) decPos = 0;
        dtostrf(f, 1, decPos, buf);
        char *p = buf;
        while (*p) p++;
        p--;
        while (*p == '0') *p-- = 0;
        if (*p == '.') *p = 0;
    }
    return buf;
}

void host_outputProgMemString(const char *p) {
    while (1) {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0) break;
        host_outputChar(c);
    }
}

void host_outputProgMemDirectString(const char *p) {
    while (1) {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0) break;
        host_outputCharDirect(c);
    }
}

void host_sleep(long ms) {
    delay(ms);
}

void host_digitalWrite(int pin, int state) {
    digitalWrite(pin, state ? HIGH : LOW);
}

int host_digitalRead(int pin) {
    return digitalRead(pin);
}

int host_analogRead(int pin) {
    return analogRead(pin);
}

void host_pinMode(int pin, int mode) {
    pinMode(pin, mode);
}

void host_play(char *str) {
    #ifdef HOST_DEBUG
    Serial.print(F("host_play: "));
    Serial.println(str);
    #endif
    play(str);
}

static void dir_callback(const char* name, uint16_t size) {
    host_outputString((char*)name);
    host_outputProgMemString(DIR_SIZE_OPEN);
    host_outputInt(size);
    host_outputProgMemString(DIR_SIZE_CLOSE);
    host_newLine();
}

static bool dir_found = false;

static void dir_check_callback(const char* name, uint16_t size) {
    dir_found = true;
}

bool host_loadExtEEPROM(const char* filename) {
    if (!ensure_eeprom()) return false;
    if (!filename || strlen(filename) == 0) return false;

    extern bool load_text_program(const uint8_t* buffer, size_t size);

    size_t size = 0;

    if (!eeprom_load_file(filename, temp_buffer, &size)) {
        host_outputProgMemString(EEPROM_FILE_NOT_FOUND);
        host_newLine();
        host_showBuffer();
        return false;
    }

    if (size > MEMORY_SIZE) {
        host_outputProgMemString(EEPROM_PROGRAM_TOO_BIG);
        host_newLine();
        return false;
    }

    // Загружаем текст и токенизируем с автоматическими номерами строк
    if (!load_text_program(temp_buffer, size)) {
        host_outputProgMemString(EEPROM_LOAD_FAILED);
        host_newLine();
        host_showBuffer();
        return false;
    }

    host_outputProgMemString(EEPROM_LOAD_OK);
    host_newLine();
    host_showBuffer();

    return true;
}

bool host_removeExtEEPROM(const char* filename) {
    return eeprom_delete_file(filename);
}

void host_dirExtEEPROM() {
    dir_found = false;
    eeprom_list_files(dir_check_callback);
    
    if (dir_found) {
        eeprom_list_files(dir_callback);
    } else {
        host_outputProgMemString(EEPROM_NO_FILES);
        host_newLine();
    }
    
    host_showBuffer();
}

void host_extEEPROM_format() {
    eeprom_format();
    eeprom_purge_cache();
    
    host_outputProgMemString(EEPROM_FORMAT_COMPLETE);
    host_newLine();
    
    host_showBuffer();
}

bool host_saveExtEEPROM(const char* filename) {
    if (!ensure_eeprom()) return false;
    if (!filename || strlen(filename) == 0) return false;

    extern unsigned char mem[];
    extern int sysPROGEND;
    extern int program_to_text(uint8_t* buffer, int buffer_size);

    // Конвертируем токенизированную программу в текст
    uint8_t text_buffer[MEMORY_SIZE];
    int text_size = program_to_text(text_buffer, sizeof(text_buffer));

    if (text_size == 0 && sysPROGEND > 0) {
        host_outputProgMemString(EEPROM_SAVE_FAILED);
        return false;
    }

    bool result = eeprom_save_file(filename, text_buffer, text_size);

    if (result) {
        eeprom_sync();
    } else {
        host_outputProgMemString(EEPROM_SAVE_FAILED);
    }

    return result;
}
void lcd_cursor() {
    lcd.cursor();
}

void lcd_noCursor() {
    lcd.noCursor();
}

void lcd_blink() {
    lcd.blink();
}

void lcd_noBlink() {
    lcd.noBlink();
}

void lcd_clear() {
    lcd.clear();
}

// Добавить в host.cpp
void lcd_setCursor(int x, int y) {
    lcd.setCursor(x, y);
}

void lcd_print(const char* str) {
    lcd.print(str);
}

void lcd_printInt(int num) {
    lcd.print(num);
}

void lcd_write(char c) {
    lcd.write(c);
}