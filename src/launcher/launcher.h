#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <Arduino.h>
#include "../config.h"

// Команды (соответствуют клавишам FN+1..4)
#define LAUNCH_CMD_NEW   0  // FN+1
#define LAUNCH_CMD_EDIT  1  // FN+2
#define LAUNCH_CMD_DEL   2  // FN+3
#define LAUNCH_CMD_RUN   3  // FN+4

// Инициализация лаунчера
void launcher_init();

// Запуск лаунчера (показывает список программ)
void launcher_run();

// Получение количества программ
int launcher_get_program_count();

// Получение имени программы по индексу
const char* launcher_get_program_name(int index);

// Получение размера программы по индексу
uint16_t launcher_get_program_size(int index);

// Выполнить команду над программой
void launcher_execute_command(int program_index, int command);

#endif