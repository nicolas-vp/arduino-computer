#include "executor.h"
#include "../eeprom/eeprom.h"
#include "../host/host.h"
#include "../lang/russian_strings.h"
#include "../keyboard/keyboard.h"
#include "../config.h"

extern unsigned char mem[];
extern int sysPROGEND;
extern uint16_t lineNumber;
extern const char *const errorTable[];

static int loaded_lines = 0;
static int error_line = 0;

static void reset_stats() {
    loaded_lines = 0;
    error_line = 0;
}

static bool parse_and_load(const uint8_t* buffer, size_t size) {
    reset_stats();

    extern bool load_text_program(const uint8_t* buffer, size_t size);

    if (!load_text_program(buffer, size)) {
        return false;
    }

    // Подсчитываем количество строк
    loaded_lines = 0;
    int pos = 0;
    while (pos < (int)size) {
        if (buffer[pos] == '\n') loaded_lines++;
        pos++;
    }
    if (loaded_lines == 0 && size > 0) loaded_lines = 1;

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

    host_cls();

    unsigned char run_token[2];
    run_token[0] = TOKEN_RUN;
    run_token[1] = 0;

    int ret = processInput(run_token);

    host_outputProgMemString(PRESS_SPACE);
    host_showBuffer();

    unsigned char key;
    do {
        key = readKey();
        delay(10);
    } while (key != ' ');

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
        host_newLine();
    }

    host_showBuffer();

    return true;
}
