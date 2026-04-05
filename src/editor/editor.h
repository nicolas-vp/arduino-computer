#ifndef EDITOR_H
#define EDITOR_H

#include <Arduino.h>
#include "../executor/executor.h"  // Теперь MAX_FILE_SIZE определен здесь

struct TextFile {
    char data[MAX_FILE_SIZE];
    int size;
    char name[16];
    bool modified;
};

void editor_run(const char* filename);
void editor_display();

#endif