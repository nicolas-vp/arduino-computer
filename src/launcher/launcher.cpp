#include "launcher.h"
#include "../host/host.h"
#include "../eeprom/eeprom.h"
#include "../basic/basic.h"
#include "../lang/russian_strings.h"
#include "../keyboard/keyboard.h"
#include "../editor/editor.h"
#include "../executor/executor.h"  // Добавляем подключение executor.h

// Структура для хранения информации о программе
struct ProgramInfo {
    char name[17];
    uint16_t size;
    uint8_t valid;
};

#define MAX_PROGRAMS 30
static ProgramInfo programs[MAX_PROGRAMS];
static int program_count = 0;

static int selected_program = 0;
static int scroll_offset = 0;

#define VISIBLE_LINES 4
#define FILE_LENGHT 6

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
    
    if (program_count > 0) {
        selected_program = 0;
        scroll_offset = 0;
    } else {
        selected_program = -1;
    }
    
    while (true) {
        host_cls();
        
        host_moveCursor(0, 0);
        
        host_moveCursor(12, 0);
        host_outputProgMemDirectString(LAUNCHER_HELP_NEW);
        host_moveCursor(12, 1);
        host_outputProgMemDirectString(LAUNCHER_HELP_EDIT);
        host_moveCursor(12, 2);
        host_outputProgMemDirectString(LAUNCHER_HELP_DELETE);
        host_moveCursor(12, 3);
        host_outputProgMemDirectString(LAUNCHER_HELP_RUN);

        host_moveCursor(11, 0); host_outputChar(240);
        host_moveCursor(11, 1); host_outputChar(242);
        host_moveCursor(11, 2); host_outputChar(242);
        host_moveCursor(11, 3); host_outputChar(241);
        
        if (program_count == 0) {
            host_moveCursor(0, 1);
            host_outputProgMemString(PSTR("No files"));
            host_moveCursor(0, 2);
            host_outputProgMemString(PSTR("Press 1 for NEW"));
        } else {
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
                    
                    char name_buf[FILE_LENGHT+1];
                    strncpy(name_buf, programs[idx].name, FILE_LENGHT);
                    name_buf[FILE_LENGHT] = 0;
                    host_outputString(name_buf);
                    host_outputChar('(');
                    host_outputInt(programs[idx].size);
                    host_outputChar(')');
                } else {
                    host_outputString("          ");
                }
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
                        host_outputProgMemString(PSTR("New name:"));
                        host_showBuffer();
                        
                        char* new_name = host_readLine();
                        if (strlen(new_name) > 0 && strlen(new_name) <= 16) {
                            uint8_t empty_prog[1] = {0};
                            if (eeprom_save_file(new_name, empty_prog, 0)) {
                                host_cls();
                                host_moveCursor(0, 1);
                                host_outputProgMemString(PSTR("Created!"));
                                delay(800);
                                
                                program_count = 0;
                                eeprom_list_files(collect_program_callback);
                                if (program_count > 0) selected_program = 0;
                            }
                        }
                    }
                    break;
                    
                case 1: // 2 - EDIT
                    if (program_count > 0) {
                        launcher_execute_command(selected_program, LAUNCH_CMD_EDIT);
                        program_count = 0;
                        eeprom_list_files(collect_program_callback);
                    }
                    break;
                    
                case 2: // 3 - DELETE
                    if (program_count > 0) {
                        launcher_execute_command(selected_program, LAUNCH_CMD_DEL);
                        program_count = 0;
                        eeprom_list_files(collect_program_callback);
                        if (selected_program >= program_count) {
                            selected_program = program_count - 1;
                        }
                        if (program_count == 0) selected_program = -1;
                    }
                    break;
                    
                case 3: // 4 - RUN
                    if (program_count > 0) {
                        launcher_execute_command(selected_program, LAUNCH_CMD_RUN);
                        program_count = 0;
                        eeprom_list_files(collect_program_callback);
                    }
                    break;
            }
            continue;
        }
        
        // Если нет программ, только NEW работает
        if (program_count == 0) {
            if (key == KEY_ESC) {
                host_cls();
                return;
            }
            continue;
        }
        
        // Навигация по списку
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
            host_outputProgMemString(PSTR("Delete "));
            host_outputString((char*)prog_name);
            host_outputProgMemString(PSTR("?"));
            host_moveCursor(0, 2);
            host_outputProgMemString(PSTR("Y/any key to cancel"));
            host_showBuffer();
            
            char* confirm = host_readLine();
            if (confirm[0] == 'Y' || confirm[0] == 'y') {
                if (eeprom_delete_file(prog_name)) {
                    host_cls();
                    host_moveCursor(0, 1);
                    host_outputProgMemString(PSTR("Deleted"));
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
                host_cls();
                host_moveCursor(0, 0);
                host_outputProgMemString(PSTR("Running: "));
                host_outputString((char*)prog_name);
                host_showBuffer();
                delay(500);

                // Минимальная отладка - одну строку
                // Serial.println(F("RUN"));

                extern bool executor_run_program(const char* filename);
                executor_run_program(prog_name);
            }
            break;
        }
    }
}