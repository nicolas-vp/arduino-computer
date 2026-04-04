#ifndef RUSSIAN_STRINGS_H
#define RUSSIAN_STRINGS_H

#include <avr/pgmspace.h>

// Сообщения об ошибках BASIC (в CP866)
extern const char ERR_OK[];
extern const char ERR_BAD_NUMBER[];
extern const char ERR_LINE_TOO_LONG[];
extern const char ERR_UNEXPECTED_INPUT[];
extern const char ERR_UNTERMINATED_STRING[];
extern const char ERR_MISSING_BRACKET[];
extern const char ERR_ERROR_IN_EXPR[];
extern const char ERR_NUMERIC_EXPR_EXPECTED[];
extern const char ERR_STRING_EXPR_EXPECTED[];
extern const char ERR_LINE_NUMBER_TOO_BIG[];
extern const char ERR_OUT_OF_MEMORY[];
extern const char ERR_DIV_BY_ZERO[];
extern const char ERR_VARIABLE_NOT_FOUND[];
extern const char ERR_BAD_COMMAND[];
extern const char ERR_BAD_LINE_NUMBER[];
extern const char ERR_BREAK_PRESSED[];
extern const char ERR_NEXT_WITHOUT_FOR[];
extern const char ERR_STOP_STATEMENT[];
extern const char ERR_MISSING_THEN[];
extern const char ERR_RETURN_WITHOUT_GOSUB[];
extern const char ERR_WRONG_ARRAY_DIMS[];
extern const char ERR_BAD_ARRAY_INDEX[];
extern const char ERR_BAD_STRING_INDEX[];
extern const char ERR_ERROR_IN_VAL[];
extern const char ERR_BAD_PARAMETER[];
extern const char ERR_OUT_OF_DATA[];

// Массив указателей на сообщения об ошибках
extern const char* const errorTable[];

// Сообщения для EEPROM
extern const char EEPROM_NOT_AVAILABLE[];
extern const char EEPROM_NOT_FOUND[];
extern const char EEPROM_NEEDS_FORMAT[];
extern const char EEPROM_FORMAT_COMPLETE[];
extern const char EEPROM_SAVE_OK[];
extern const char EEPROM_SAVE_FAILED[];
extern const char EEPROM_LOAD_OK[];
extern const char EEPROM_LOAD_FAILED[];
extern const char EEPROM_FILE_NOT_FOUND[];
extern const char EEPROM_PROGRAM_TOO_BIG[];
extern const char EEPROM_NO_FILES[];
extern const char EEPROM_DELETE_OK[];

// Сообщения для загрузки/сохранения
extern const char LOADING[];
extern const char SAVING[];
extern const char SIZE_BYTES[];
extern const char FIRST_BYTES[];
extern const char COPIED_TO_MEMORY[];

// Сообщения для DIR
extern const char DIR_SIZE_OPEN[];
extern const char DIR_SIZE_CLOSE[];

// Сообщения для инициализации
extern const char INIT_MESSAGE[];
extern const char BYTES_FREE[];

extern const char LAUNCHER_HELP_NEW[];
extern const char LAUNCHER_HELP_EDIT[];
extern const char LAUNCHER_HELP_DELETE[];
extern const char LAUNCHER_HELP_RUN[];

extern const char NEW_NAME[];
#endif