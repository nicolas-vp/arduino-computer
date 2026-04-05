#include "editor.h"
#include "../host/host.h"
#include "../eeprom/eeprom.h"
#include "../keyboard/keyboard.h"
#include <string.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

#define EDITOR_SCREEN_HEIGHT 3
#define INFO_LINE 3
#define SCREEN_WIDTH 20

static TextFile current_file;
static int file_pos = 0;
static int screen_offset = 0;
static int screen_col_offset = 0;
static int cursor_screen_x = 0;
static int cursor_screen_y = 0;

// Раскладка клавиатуры
extern bool ruMode;
extern void switchLayout();
extern unsigned char mapLayout(unsigned char c);

// Таймер исчезновения индикатора языка
static unsigned long lang_indicator_time = 0;
unsigned char mapLcdChar(unsigned char c);

extern void lcd_cursor();
extern void lcd_noCursor();
extern void lcd_blink();
extern void lcd_noBlink();
extern void lcd_setCursor(int x, int y);
extern void lcd_print(const char* str);
extern unsigned char mapLcdChar(unsigned char c);

extern LiquidCrystal lcd;

static void update_screen_offset() {
    if (current_file.size == 0) {
        screen_offset = 0;
        screen_col_offset = 0;
        return;
    }
    
    int line_num = 0;
    int pos = 0;
    
    while (pos < file_pos && pos < current_file.size) {
        if (current_file.data[pos] == '\n') line_num++;
        pos++;
    }
    
    int screen_start_line = 0;
    pos = 0;
    
    for (int i = 0; i < screen_offset && pos < current_file.size; i++) {
        if (current_file.data[pos] == '\n') screen_start_line++;
        pos++;
    }
    
    if (line_num < screen_start_line) {
        screen_start_line = line_num;
        pos = 0;
        int current_line = 0;
        while (pos < current_file.size && current_line < screen_start_line) {
            if (current_file.data[pos] == '\n') current_line++;
            pos++;
        }
        screen_offset = pos;
    }
    else if (line_num >= screen_start_line + EDITOR_SCREEN_HEIGHT) {
        screen_start_line = line_num - EDITOR_SCREEN_HEIGHT + 1;
        pos = 0;
        int current_line = 0;
        while (pos < current_file.size && current_line < screen_start_line) {
            if (current_file.data[pos] == '\n') current_line++;
            pos++;
        }
        screen_offset = pos;
    }
    
    if (screen_offset >= current_file.size) {
        screen_offset = current_file.size - 1;
        if (screen_offset < 0) screen_offset = 0;
    }
    
    int col_in_line = 0;
    pos = file_pos;
    while (pos > 0 && current_file.data[pos - 1] != '\n') {
        col_in_line++;
        pos--;
    }
    
    if (col_in_line < screen_col_offset) {
        screen_col_offset = col_in_line;
    }
    else if (col_in_line >= screen_col_offset + SCREEN_WIDTH) {
        screen_col_offset = col_in_line - SCREEN_WIDTH + 1;
    }
    
    if (screen_col_offset < 0) screen_col_offset = 0;
}

void editor_display() {
    lcd.clear();
    delay(5);

    int file_idx = screen_offset;
    int current_screen_line = 0;

    while (current_screen_line < EDITOR_SCREEN_HEIGHT) {
        lcd.setCursor(0, current_screen_line);
        delay(1);

        if (file_idx >= current_file.size) {
            // Пустая строка — пробелы
            for (int x = 0; x < SCREEN_WIDTH; x++) lcd.print(' ');
            current_screen_line++;
            continue;
        }

        // Пропускаем символы до screen_col_offset
        int skipped = 0;
        int temp_idx = file_idx;
        while (skipped < screen_col_offset &&
               temp_idx < current_file.size &&
               current_file.data[temp_idx] != '\n') {
            skipped++;
            temp_idx++;
        }

        if (skipped < screen_col_offset) {
            // Вся строка до offset — пробелы
            for (int x = 0; x < SCREEN_WIDTH; x++) lcd.print(' ');
            // Переходим к следующему \n
            while (file_idx < current_file.size && current_file.data[file_idx] != '\n') file_idx++;
            if (file_idx < current_file.size) file_idx++; // skip \n
            current_screen_line++;
            continue;
        }

        file_idx = temp_idx;
        int x = 0;

        while (file_idx < current_file.size &&
               current_file.data[file_idx] != '\n' &&
               x < SCREEN_WIDTH) {
            unsigned char c = (unsigned char)current_file.data[file_idx];
            if (c >= 32) {
                lcd.write(mapLcdChar(c));
            } else {
                lcd.write('.');
            }
            file_idx++;
            x++;
        }

        while (x < SCREEN_WIDTH) {
            lcd.print(' ');
            x++;
        }

        if (file_idx < current_file.size && current_file.data[file_idx] == '\n') {
            file_idx++;
        }
        current_screen_line++;
    }
    
    while (current_screen_line < EDITOR_SCREEN_HEIGHT) {
        lcd.setCursor(0, current_screen_line);
        delay(1);
        for (int x = 0; x < SCREEN_WIDTH; x++) lcd.print(' ');
        current_screen_line++;
    }

    lcd.setCursor(0, INFO_LINE);
    delay(1);

    int name_len = strlen(current_file.name);
    for (int i = 0; i < 8 && i < name_len; i++) lcd.print(current_file.name[i]);
    if (name_len > 8) lcd.print('+');
    if (current_file.modified) lcd.print('*');

    int line_num = 1;
    int col_num = 1;
    for (int i = 0; i < file_pos && i < current_file.size; i++) {
        if (current_file.data[i] == '\n') {
            line_num++;
            col_num = 1;
        } else {
            col_num++;
        }
    }

    lcd.setCursor(14, INFO_LINE);
    delay(1);
    char pos_buf[10];
    sprintf(pos_buf, "%d:%d", line_num, col_num);
    lcd.print(pos_buf);
    
    int cursor_file_pos = screen_offset;
    cursor_screen_x = 0;
    cursor_screen_y = 0;
    current_screen_line = 0;
    
    while (current_screen_line < EDITOR_SCREEN_HEIGHT && cursor_file_pos <= file_pos) {
        if (cursor_file_pos == file_pos) {
            cursor_screen_y = current_screen_line;
            
            int col = 0;
            int temp_pos = file_pos;
            
            while (temp_pos > 0 && current_file.data[temp_pos - 1] != '\n') {
                col++;
                temp_pos--;
            }
            
            cursor_screen_x = col - screen_col_offset;
            
            if (cursor_screen_x < 0) cursor_screen_x = 0;
            if (cursor_screen_x >= SCREEN_WIDTH) cursor_screen_x = SCREEN_WIDTH - 1;
            
            break;
        }
        
        if (cursor_file_pos < current_file.size && 
            current_file.data[cursor_file_pos] == '\n') {
            current_screen_line++;
            cursor_screen_x = 0;
            cursor_file_pos++;
        } else {
            cursor_file_pos++;
        }
    }
    
    if (cursor_screen_y < EDITOR_SCREEN_HEIGHT) {
        lcd.setCursor(cursor_screen_x, cursor_screen_y);
    } else {
        lcd.setCursor(0, EDITOR_SCREEN_HEIGHT - 1);
    }
}

static void editor_insert_char(char c) {
    if (current_file.size >= MAX_FILE_SIZE - 1) return;
    
    for (int i = current_file.size; i > file_pos; i--) {
        current_file.data[i] = current_file.data[i - 1];
    }
    
    current_file.data[file_pos] = c;
    current_file.size++;
    file_pos++;
    current_file.modified = true;
}

static void editor_backspace() {
    if (file_pos > 0) {
        if (current_file.data[file_pos - 1] == '\n') {
            file_pos--;
            for (int i = file_pos; i < current_file.size - 1; i++) {
                current_file.data[i] = current_file.data[i + 1];
            }
            current_file.size--;
        } else {
            for (int i = file_pos - 1; i < current_file.size - 1; i++) {
                current_file.data[i] = current_file.data[i + 1];
            }
            current_file.size--;
            file_pos--;
        }
        current_file.modified = true;
    }
}

static void editor_move_up() {
    if (file_pos == 0) return;
    
    int new_pos = file_pos - 1;
    while (new_pos >= 0 && current_file.data[new_pos] != '\n') new_pos--;
    
    if (new_pos >= 0) {
        new_pos--;
        while (new_pos >= 0 && current_file.data[new_pos] != '\n') new_pos--;
        new_pos++;
        
        int target_col = 0;
        for (int i = file_pos - 1; i >= 0 && current_file.data[i] != '\n'; i--) target_col++;
        
        file_pos = new_pos;
        int line_len = 0;
        int temp = file_pos;
        while (temp < current_file.size && current_file.data[temp] != '\n') {
            line_len++;
            temp++;
        }
        
        for (int i = 0; i < target_col && i < line_len; i++) file_pos++;
    }
}

static void editor_move_down() {
    if (file_pos >= current_file.size) return;
    
    int new_pos = file_pos;
    while (new_pos < current_file.size && current_file.data[new_pos] != '\n') new_pos++;
    
    if (new_pos < current_file.size) {
        new_pos++;
        
        int target_col = 0;
        for (int i = file_pos - 1; i >= 0 && current_file.data[i] != '\n'; i--) target_col++;
        
        file_pos = new_pos;
        int line_len = 0;
        int temp = file_pos;
        while (temp < current_file.size && current_file.data[temp] != '\n') {
            line_len++;
            temp++;
        }
        
        for (int i = 0; i < target_col && i < line_len; i++) file_pos++;
    }
}

static void editor_move_left() {
    if (file_pos > 0) {
        file_pos--;
        if (file_pos > 0 && current_file.data[file_pos] == '\n') {
            file_pos--;
            while (file_pos > 0 && current_file.data[file_pos] != '\n') file_pos--;
            if (current_file.data[file_pos] == '\n') {
                file_pos++;
                while (file_pos < current_file.size && current_file.data[file_pos] != '\n') {
                    file_pos++;
                }
            }
        }
    }
}

static void editor_move_right() {
    if (file_pos >= current_file.size) return;
    
    if (current_file.data[file_pos] == '\n') {
        file_pos++;
        return;
    }
    
    file_pos++;
    
    if (file_pos < current_file.size && current_file.data[file_pos] == '\n') {
        return;
    }
}

static void editor_save() {
    if (eeprom_save_file(current_file.name, (uint8_t*)current_file.data, current_file.size)) {
        current_file.modified = false;
        lcd_setCursor(0, INFO_LINE);
        lcd_print("Saved!        ");
        delay(500);
    } else {
        lcd_setCursor(0, INFO_LINE);
        lcd_print("Save failed!  ");
        delay(500);
    }
}

static bool editor_load(const char* filename) {
    strncpy(current_file.name, filename, 15);
    current_file.name[15] = 0;
    
    size_t size = 0;
    if (eeprom_load_file(filename, (uint8_t*)current_file.data, &size)) {
        current_file.size = size;
        current_file.modified = false;
        file_pos = 0;
        screen_offset = 0;
        screen_col_offset = 0;
        return true;
    }
    
    current_file.size = 0;
    current_file.modified = false;
    file_pos = 0;
    screen_offset = 0;
    screen_col_offset = 0;
    return false;
}

static bool editor_confirm_exit() {
    if (!current_file.modified) return true;
    
    lcd_setCursor(0, INFO_LINE);
    lcd_print("Save? Y/N     ");

    while (true) {
        unsigned char key = readKey();
        if (key == 'Y' || key == 'y') {
            editor_save();
            return true;
        }
        if (key == 'N' || key == 'n' || key == KEY_ESC) {
            return true;
        }
        delay(50);
    }
}

void editor_run(const char* filename) {
    if (!filename || filename[0] == 0) return;
    
    editor_load(filename);
    
    lcd_cursor();
    lcd_blink();
    
    update_screen_offset();
    editor_display();
    
    while (true) {
        unsigned char key = readKey();
        if (key == 0) {
            delay(10);
            continue;
        }

        unsigned long now = millis();
        // Debounce — игнорируем повтор того же символа
        static unsigned char prev_key = 0;
        static unsigned long prev_key_ms = 0;
        if (key == prev_key && (now - prev_key_ms) < 300) {
            delay(10);
            continue;
        }
        prev_key = key;
        prev_key_ms = now;

        switch(key) {
            case 0xB5:
                editor_move_up();
                break;
            case 0xB6:
                editor_move_down();
                break;
            case 0xB4:
                editor_move_left();
                break;
            case 0xB7:
                editor_move_right();
                break;
            case 175:
                switchLayout();
                lang_indicator_time = millis();
                break;
            case KEY_ENTER:
                editor_insert_char('\n');
                break;
            case 0x08:
            case 0x7F:
                editor_backspace();
                break;
            case 174:
                editor_save();
                break;
            case 0x1B:
                if (editor_confirm_exit()) {
                    lcd_noCursor();
                    lcd_noBlink();
                    return;
                }
                break;
            default:
                if (key >= 32 && key <= 126) {
                    key = mapLayout(key);
                    editor_insert_char(key);
                }
                break;
        }
        
        update_screen_offset();
        editor_display();

        // Показываем индикатор раскладки только 1 секунду после переключения
        if (lang_indicator_time > 0 && (now - lang_indicator_time) < 1000) {
            lcd.setCursor(SCREEN_WIDTH - 3, 0);
            if (ruMode) {
                lcd.print((char)146); lcd.print('P'); lcd.print((char)169);
            } else {
                lcd.print((char)147); lcd.print('E'); lcd.print('N');
            }
            // Возвращаем курсор на позицию
            if (cursor_screen_y < EDITOR_SCREEN_HEIGHT) {
                lcd.setCursor(cursor_screen_x, cursor_screen_y);
            } else {
                lcd.setCursor(0, EDITOR_SCREEN_HEIGHT - 1);
            }
        } else if (lang_indicator_time > 0 && (now - lang_indicator_time) >= 1000) {
            // Индикатор истёк — очистим и сбросим таймер
            lcd.setCursor(SCREEN_WIDTH - 3, 0);
            lcd.print(' '); lcd.print(' '); lcd.print(' ');
            // Возвращаем курсор на позицию
            if (cursor_screen_y < EDITOR_SCREEN_HEIGHT) {
                lcd.setCursor(cursor_screen_x, cursor_screen_y);
            } else {
                lcd.setCursor(0, EDITOR_SCREEN_HEIGHT - 1);
            }
            lang_indicator_time = 0;
        }
    }
}