#include "launcher.h"
#include "../host/host.h"
#include "../eeprom/eeprom.h"
#include "../basic/basic.h"
#include "../lang/russian_strings.h"
#include "../keyboard/keyboard.h"
#include "../editor/editor.h"
#include "../executor/executor.h"
#include "../utils/utils_ascii.h"
#include "../utils/utils_music.h"

// Структура для хранения информации о программе
struct ProgramInfo {
    char name[17];
    uint16_t size;
    uint8_t valid;
    uint8_t special; // 1 = специальная команда
    uint8_t cmd;     // номер специальной команды
};

#define MAX_PROGRAMS 30
static ProgramInfo programs[MAX_PROGRAMS];
static int program_count = 0;

static int selected_program = 0;
static int scroll_offset = 0;

#define VISIBLE_LINES 4
#define FILE_LENGHT 6

// Специальные пункты (всегда в конце списка)
static const char* special_names[] = {"[\x94\x8E\x90\x8C\x80\x92]", "[\x91\x88\x8C\x82\x8E\x8B\x9B]", "[\x87\x82\x93\x8A\x88]"};
static const uint8_t special_cmds[] = {LAUNCH_CMD_FORMAT, LAUNCH_CMD_ASCII, LAUNCH_CMD_MUSIC};
#define SPECIAL_COUNT 3

static void add_special_items() {
    for (int i = 0; i < SPECIAL_COUNT; i++) {
        strncpy(programs[program_count].name, special_names[i], 16);
        programs[program_count].name[16] = 0;
        programs[program_count].size = 0;
        programs[program_count].valid = 1;
        programs[program_count].special = 1;
        programs[program_count].cmd = special_cmds[i];
        program_count++;
    }
}

extern void host_cls();
extern void host_moveCursor(int x, int y);
extern void host_outputChar(char c);
extern void host_outputString(char *str);
extern int host_outputInt(long num);
extern void host_outputProgMemString(const char *p);
extern char* host_readLine();

static bool collect_program_callback(const char* name, uint16_t size) {
    if (program_count < MAX_PROGRAMS) {
        strncpy(programs[program_count].name, name, 16);
        programs[program_count].name[16] = 0;
        programs[program_count].size = size;
        programs[program_count].valid = 1;
        programs[program_count].special = 0;
        programs[program_count].cmd = 0;
        program_count++;
    }
    return true;
}

void launcher_init() {
    program_count = 0;
    selected_program = 0;
    scroll_offset = 0;
}

void launcher_run() {
    host_cls();

    program_count = 0;
    eeprom_list_files(collect_program_callback);

    // Добавляем специальные пункты в конец
    add_special_items();

    selected_program = 0;
    scroll_offset = 0;

    while (true) {
        host_cls();

        host_moveCursor(0, 0);

        // Показываем подсказки только если есть программы
        if (program_count > SPECIAL_COUNT) {
            host_moveCursor(13, 0);
            host_outputProgMemDirectString(LAUNCHER_HELP_NEW);
            host_moveCursor(13, 1);
            host_outputProgMemDirectString(LAUNCHER_HELP_EDIT);
            host_moveCursor(13, 2);
            host_outputProgMemDirectString(LAUNCHER_HELP_DELETE);
            host_moveCursor(13, 3);
            host_outputProgMemDirectString(LAUNCHER_HELP_RUN);
            host_moveCursor(13, 3); host_outputChar(240);
        } else {
            host_moveCursor(13, 0);
            host_outputProgMemDirectString(LAUNCHER_HELP_NEW);
        }

        int start_idx = scroll_offset;

        for (int i = 0; i < VISIBLE_LINES; i++) {
            int idx = start_idx + i;
            int y = 0 + i;

            host_moveCursor(0, y);

            if (idx < program_count) {
                if (idx == selected_program) {
                    host_outputChar('>');
                } else {
                    host_outputChar(' ');
                }

                if (programs[idx].special) {
                    host_outputString((char*)programs[idx].name);
                } else {
                    char name_buf[FILE_LENGHT+1];
                    strncpy(name_buf, programs[idx].name, FILE_LENGHT);
                    name_buf[FILE_LENGHT] = 0;
                    host_outputString(name_buf);
                    host_outputChar('(');
                    host_outputInt(programs[idx].size);
                    host_outputChar(')');
                }
            } else {
                host_outputString("          ");
            }
        }

        host_showBuffer();

        unsigned char key = 0;
        while (key == 0) {
            key = readKey();
            delay(10);
        }

        // Обработка цифровых клавиш
        if (key >= '1' && key <= '4') {
            int cmd = key - '1';

            switch (cmd) {
                case 0: // 1 - NEW
                    if (program_count < MAX_PROGRAMS) {
                        host_cls();
                        host_moveCursor(0, 0);
                        host_outputProgMemDirectString(NEW_NAME);
                        host_showBuffer();

                        char* new_name = host_readLine();
                        if (strlen(new_name) > 0 && strlen(new_name) <= 16) {
                            uint8_t empty_prog[1] = {0};
                            if (eeprom_save_file(new_name, empty_prog, 0)) {
                                host_cls();
                                program_count = 0;
                                eeprom_list_files(collect_program_callback);
                                add_special_items();
                                selected_program = 0;
                            }
                        }
                    }
                    break;

                case 1: // 2 - EDIT
                    if (!programs[selected_program].special) {
                        launcher_execute_command(selected_program, LAUNCH_CMD_EDIT);
                        program_count = 0;
                        eeprom_list_files(collect_program_callback);
                        add_special_items();
                    }
                    break;

                case 2: // 3 - DELETE
                    if (!programs[selected_program].special) {
                        launcher_execute_command(selected_program, LAUNCH_CMD_DEL);
                        program_count = 0;
                        eeprom_list_files(collect_program_callback);
                        add_special_items();
                        if (selected_program >= program_count - SPECIAL_COUNT) {
                            selected_program = program_count - SPECIAL_COUNT - 1;
                        }
                        if (selected_program < 0) selected_program = SPECIAL_COUNT;
                    }
                    break;

                case 3: // 4 - RUN
                    launcher_execute_command(selected_program, LAUNCH_CMD_RUN);
                    program_count = 0;
                    eeprom_list_files(collect_program_callback);
                    add_special_items();
                    break;
            }
            continue;
        }

        // Если только специальные пункты (нет программ), только NEW работает
        if (key == KEY_UP) {
            if (selected_program > 0) {
                selected_program--;
                if (selected_program < scroll_offset) {
                    scroll_offset = selected_program;
                }
            }
        }
        else if (key == KEY_DOWN) {
            if (selected_program < program_count - 1) {
                selected_program++;
                if (selected_program >= scroll_offset + VISIBLE_LINES) {
                    scroll_offset = selected_program - VISIBLE_LINES + 1;
                }
            }
        }
        else if (key == KEY_ENTER) {
            launcher_execute_command(selected_program, LAUNCH_CMD_RUN);
            program_count = 0;
            eeprom_list_files(collect_program_callback);
            add_special_items();
        }
        else if (key == KEY_ESC) {
            host_cls();
            return;
        }
    }
}

void launcher_execute_command(int program_index, int command) {
    if (program_index < 0 || program_index >= program_count) return;
    
    const char* prog_name = programs[program_index].name;
    
    switch (command) {
        case LAUNCH_CMD_EDIT: {
            host_cls();
            extern void editor_run(const char* filename);
            editor_run(prog_name);
            host_cls();
            break;
        }
        
        case LAUNCH_CMD_DEL: {
            host_cls();
            host_moveCursor(0, 1);
            host_outputProgMemString(PSTR("\x93\xA4\xA0\xAB\xA8\xE2\xEC\x20"));
            host_outputString((char*)prog_name);
            host_outputProgMemString(PSTR("?"));
            host_moveCursor(0, 2);
            host_outputProgMemString(PSTR("Y/any key"));
            host_showBuffer();
            
            char* confirm = host_readLine();
            if (confirm[0] == 'Y' || confirm[0] == 'y') {
                if (eeprom_delete_file(prog_name)) {
                    host_cls();
                    host_moveCursor(0, 1);
                    host_outputProgMemString(PSTR("\x93\xA4\xA0\xAB\xA5\xAD"));
                } else {
                    host_cls();
                    host_moveCursor(0, 1);
                    host_outputProgMemString(PSTR("Failed"));
                }
                delay(1000);
            }
            break;
        }
        
        case LAUNCH_CMD_RUN: {
            if (program_count > 0) {
                // Проверяем, специальная ли это команда
                if (programs[program_index].special) {
                    launcher_execute_special_command(programs[program_index].cmd);
                } else {
                    host_cls();
                    host_moveCursor(0, 0);
                    host_outputProgMemString(PSTR("Running: "));
                    host_outputString((char*)prog_name);
                    host_showBuffer();
                    delay(500);

                    extern bool executor_run_program(const char* filename);
                    executor_run_program(prog_name);
                }
            }
            break;
        }
    }
}

void launcher_execute_special_command(uint8_t cmd) {
    switch (cmd) {
        case LAUNCH_CMD_FORMAT: {
            host_cls();
            host_moveCursor(0, 1);
            host_outputProgMemString(PSTR("\x94\xAE\xE0\xAC\xA0\xE2\xA8\xE0\xAE\xA2\xA0\xE2\xEC\x20\x45\x45\x50\x52\x4F\x4D\x3F"));
            host_moveCursor(0, 2);
            host_outputProgMemString(PSTR("Y/any key"));
            host_showBuffer();

            char* confirm = host_readLine();
            if (confirm[0] == 'Y' || confirm[0] == 'y') {
                eeprom_format();
                host_cls();
                host_moveCursor(0, 1);
                host_outputProgMemString(PSTR("\x94\xAE\xE0\xAC\xA0\xE2\x20\xA7\xA0\xA2\xA5\xE0\xE8\xA5\xAD"));
                host_showBuffer();
                delay(1000);
            }
            break;
        }

        case LAUNCH_CMD_ASCII: {
            host_cls();
            host_showBuffer();
            showAscii();
            break;
        }

        case LAUNCH_CMD_MUSIC: {
            host_cls();
            host_showBuffer();
            musicPlayer();
            break;
        }
    }
}