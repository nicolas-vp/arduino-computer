#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <Arduino.h>
#include "../config.h"

// Команды (соответствуют клавишам FN+1..4)
#define LAUNCH_CMD_NEW   0  // FN+1
#define LAUNCH_CMD_EDIT  1  // FN+2
#define LAUNCH_CMD_DEL   2  // FN+3
#define LAUNCH_CMD_RUN   3  // FN+4

// Специальные команды
#define LAUNCH_CMD_FORMAT 10
#define LAUNCH_CMD_ASCII   11
#define LAUNCH_CMD_MUSIC   12

// Инициализация лаунчера
void launcher_init();

// Запуск лаунчера (показывает список программ)
void launcher_run();

// Выполнить команду над программой
void launcher_execute_command(int program_index, int command);

// Выполнить специальную команду
void launcher_execute_special_command(uint8_t cmd);

#endif