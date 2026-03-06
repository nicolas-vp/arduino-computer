#include "executor.h"
#include "../eeprom/eeprom.h"
#include "../host/host.h"
#include "../lang/russian_strings.h"
#include "../keyboard/keyboard.h"
#include "../config.h"
#include <string.h>

// Временно отключаем отладку
#define EXECUTOR_DEBUG 0

extern unsigned char mem[];
extern int sysPROGEND;
extern uint16_t lineNumber;
extern const char *const errorTable[];
extern int doProgLine(uint16_t lineNumber, unsigned char *tokenPtr, int tokensLength);

static int loaded_lines = 0;
static int error_line = 0;

static void reset_stats() {
    loaded_lines = 0;
    error_line = 0;
}

static bool load_line(const char* line, int line_num) {
    unsigned char token_buf[128];
    
    int ret = tokenize((unsigned char*)line, token_buf, sizeof(token_buf));
    
    if (ret != ERROR_NONE) {
        error_line = line_num;
        return false;
    }
    
    if (!doProgLine(line_num, token_buf, strlen((char*)token_buf))) {
        error_line = line_num;
        return false;
    }
    
    loaded_lines++;
    return true;
}

static bool parse_and_load(const uint8_t* buffer, size_t size) {
    int line_num = 1;
    int pos = 0;
    char line_buffer[128];
    int line_pos = 0;
    
    reset_stats();
    
    while (pos < size) {
        char c = buffer[pos++];
        
        if (c == '\n' || c == '\r' || pos == size) {
            if (pos == size && c != '\n' && c != '\r') {
                if (line_pos < sizeof(line_buffer) - 1) {
                    line_buffer[line_pos++] = c;
                }
            }
            
            if (line_pos < sizeof(line_buffer)) {
                line_buffer[line_pos] = 0;
            } else {
                line_buffer[sizeof(line_buffer) - 1] = 0;
            }
            
            if (line_pos > 0) {
                host_outputString(line_buffer);
                delay(1000);

                if (!load_line(line_buffer, line_num)) {
                    return false;
                }
                line_num++;
            }
            
            line_pos = 0;
            
            if (c == '\r' && pos < size && buffer[pos] == '\n') {
                pos++;
            }
        } else {
            if (line_pos < sizeof(line_buffer) - 1) {
                line_buffer[line_pos++] = c;
            }
        }
    }
    
    if (loaded_lines == 0 && line_num > 1) {
        loaded_lines = 1;
    }
    
    return true;
}

bool executor_run_program(const char* filename) {
    if (!filename || filename[0] == 0) return false;
    
    uint8_t buffer[MAX_FILE_SIZE];
    size_t file_size = 0;
    
    if (!eeprom_load_file(filename, buffer, &file_size)) {
        host_cls();
        host_moveCursor(0, 1);
        host_outputProgMemString(PSTR("Load failed!"));
        host_showBuffer();
        delay(1000);
        return false;
    }
    
    return executor_run_buffer(buffer, file_size);
}

bool executor_run_buffer(const uint8_t* buffer, size_t size) {
    if (size == 0) {
        host_cls();
        host_moveCursor(0, 1);
        host_outputProgMemString(PSTR("Empty file"));
        host_showBuffer();
        delay(1000);
        return false;
    }
    
    reset();
    
    if (!parse_and_load(buffer, size)) {
        host_cls();
        host_moveCursor(0, 1);
        host_outputProgMemString(PSTR("Error in line "));
        host_outputInt(error_line);
        host_showBuffer();
        delay(1000);
        return false;
    }


    #if EXECUTOR_DEBUG
    Serial.print(F("After parse_and_load: sysPROGEND="));
    Serial.println(sysPROGEND);
    Serial.print(F("First bytes of program: "));
    for (int i = 0; i < 10 && i < sysPROGEND; i++) {
        Serial.print(mem[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
    #endif
    
    host_cls();
    host_outputProgMemString(PSTR("Program loaded"));
    host_newLine();
    host_outputInt(loaded_lines);
    host_outputProgMemString(PSTR(" lines"));
    host_newLine();
    host_outputProgMemString(PSTR("Running..."));
    host_showBuffer();
    delay(500);

    unsigned char run_token[2];
    run_token[0] = TOKEN_RUN;
    run_token[1] = 0;
    
    int ret = processInput(run_token);
    
    host_cls();
    if (ret != ERROR_NONE) {
        host_moveCursor(0, 0);
        host_outputString("Error: ");
        if (lineNumber != 0) {
            host_outputInt(lineNumber);
            host_outputChar('-');
        }
        host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
        host_newLine();
    } else {
        host_outputString("program-ended");
        // host_outputProgMemString(PSTR("Program ended"));
        host_newLine();
    }
    
    // host_outputProgMemString(PSTR("Press any key..."));
    host_showBuffer();
    
    while (readKey() == 0) delay(10);
    
    return true;
}

int executor_get_line_count() {
    return loaded_lines;
}

int executor_get_error_line() {
    return error_line;
}