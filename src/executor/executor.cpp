#include "executor.h"
#include "../eeprom/eeprom.h"
#include "../host/host.h"
#include "../lang/russian_strings.h"
#include "../keyboard/keyboard.h"
#include "../config.h"
#include <string.h>

// Включаем отладку
#define EXECUTOR_DEBUG 1

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

    Serial.print("LINE ");
    Serial.print(line_num);
    Serial.print(": \"");
    Serial.print(line);
    Serial.println("\"");

    int ret = tokenize((unsigned char*)line, token_buf, sizeof(token_buf));

    if (ret != ERROR_NONE) {
        Serial.print("Tokenize error: ");
        Serial.println(ret);
        error_line = line_num;
        return false;
    }

    Serial.print("Tokens: ");
    unsigned char *p = token_buf;
    while (*p != TOKEN_EOL) {
        Serial.print("[");
        Serial.print(*p, DEC);
        Serial.print("] ");
        p++;
    }
    Serial.println();

    if (!doProgLine(line_num, token_buf, strlen((char*)token_buf))) {
        Serial.println("doProgLine failed");
        error_line = line_num;
        return false;
    }

    Serial.print("OK, memory now ");
    Serial.println(sysPROGEND);

    loaded_lines++;
    return true;
}

static bool parse_and_load(const uint8_t* buffer, size_t size) {
    int line_num = 1;
    int pos = 0;
    char line_buffer[128];
    int line_pos = 0;

    reset_stats();

    Serial.print("File size: ");
    Serial.println(size);

    Serial.print("Hex: ");
    for (size_t i = 0; i < size; i++) {
        if (buffer[i] < 0x10) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    Serial.print("Text: \"");
    for (size_t i = 0; i < size; i++) {
        char c = buffer[i];
        if (c == '\r') Serial.print("\\r");
        else if (c == '\n') Serial.print("\\n");
        else if (c >= 32 && c <= 126) Serial.print(c);
        else {
            Serial.print("[");
            Serial.print(c, HEX);
            Serial.print("]");
        }
    }
    Serial.println("\"");

    while (pos < (int)size) {
        char c = buffer[pos++];

        if (c == '\n' || c == '\r' || pos == (int)size) {
            if (pos == (int)size && c != '\n' && c != '\r') {
                if (line_pos < (int)(sizeof(line_buffer) - 1)) {
                    line_buffer[line_pos++] = c;
                }
            }

            if (line_pos < (int)sizeof(line_buffer)) {
                line_buffer[line_pos] = 0;
            }

            if (line_pos > 0) {
                if (!load_line(line_buffer, line_num)) {
                    return false;
                }
                line_num++;
            }

            line_pos = 0;

            if (c == '\r' && pos < (int)size && buffer[pos] == '\n') {
                pos++;
            }
        } else {
            if (line_pos < (int)(sizeof(line_buffer) - 1)) {
                line_buffer[line_pos++] = c;
            }
        }
    }

    Serial.print("Loaded ");
    Serial.print(loaded_lines);
    Serial.println(" lines");

    return true;
}

bool executor_run_program(const char* filename) {
    if (!filename || filename[0] == 0) return false;

    uint8_t buffer[MAX_FILE_SIZE];
    size_t file_size = 0;

    Serial.print("Loading program: ");
    Serial.println(filename);

    if (!eeprom_load_file(filename, buffer, &file_size)) {
        Serial.println("Load failed!");
        return false;
    }

    Serial.print("Load OK, size=");
    Serial.println(file_size);

    return executor_run_buffer(buffer, file_size);
}

bool executor_run_buffer(const uint8_t* buffer, size_t size) {
    if (size == 0) {
        Serial.println("Empty file");
        return false;
    }

    reset();

    if (!parse_and_load(buffer, size)) {
        Serial.print("Parse error in line ");
        Serial.println(error_line);
        return false;
    }

    Serial.println("Creating RUN token");

    unsigned char run_token[2];
    run_token[0] = TOKEN_RUN;
    run_token[1] = 0;

    Serial.print("Calling processInput with token ");
    Serial.println(run_token[0], DEC);

    int ret = processInput(run_token);

    Serial.print("processInput returned: ");
    Serial.println(ret);

    return true;
}

int executor_get_line_count() {
    return loaded_lines;
}

int executor_get_error_line() {
    return error_line;
}