#include "russian_strings.h"
#include <avr/pgmspace.h>

// ------------------------------------------------------------------
// Сообщения об ошибках BASIC в CP866
// ------------------------------------------------------------------
const char ERR_OK[] PROGMEM = "\x8E\x8A"; // "ОК"
const char ERR_BAD_NUMBER[] PROGMEM = "\x8E\xE8\xA8\xA1\xAA\xA0\x20\xE7\xA8\xE1\xAB\xA0"; // "Ошибка числа"
const char ERR_LINE_TOO_LONG[] PROGMEM = "\x91\xE2\xE0\xAE\xAA\xA0\x20\xE1\xAB\xA8\xE8\xAA\xAE\xAC\x20\xA4\xAB\xA8\xAD\xAD\xA0\xEF"; // "Строка слишком длинная"
const char ERR_UNEXPECTED_INPUT[] PROGMEM = "\x8D\xA5\xAE\xA6\xA8\xA4\xA0\xAD\xAD\xEB\xA9\x20\xA2\xA2\xAE\xA4"; // "Неожиданный ввод"
const char ERR_UNTERMINATED_STRING[] PROGMEM = "\x8D\xA5\xA7\xA0\xAA\xE0\xEB\xE2\xA0\xEF\x20\xE1\xE2\xE0\xAE\xAA\xA0"; // "Незакрытая строка"
const char ERR_MISSING_BRACKET[] PROGMEM = "\x8E\xE2\xE1\xE3\xE2\xE1\xE2\xA2\xE3\xA5\xE2\x20\xE1\xAA\xAE\xA1\xAA\xA0"; // "Отсутствует скобка"
const char ERR_ERROR_IN_EXPR[] PROGMEM = "\x8E\xE8\xA8\xA1\xAA\xA0\x20\xA2\x20\xA2\xEB\xE0\xA0\xA6\xA5\xAD\xA8\xA8"; // "Ошибка в выражении"
const char ERR_NUMERIC_EXPR_EXPECTED[] PROGMEM = "\x8E\xA6\xA8\xA4\xA0\xA5\xE2\xE1\xEF\x20\xE7\xA8\xE1\xAB\xAE"; // "Ожидается число"
const char ERR_STRING_EXPR_EXPECTED[] PROGMEM = "\x8E\xA6\xA8\xA4\xA0\xA5\xE2\xE1\xEF\x20\xE1\xE2\xE0\xAE\xAA\xA0"; // "Ожидается строка"
const char ERR_LINE_NUMBER_TOO_BIG[] PROGMEM = "\x8D\xAE\xAC\xA5\xE0\x20\xE1\xE2\xE0\xAE\xAA\xA8\x20\xE1\xAB\xA8\xE8\xAA\xAE\xAC\x20\xA2\xA5\xAB\xA8\xAA"; // "Номер строки слишком велик"
const char ERR_OUT_OF_MEMORY[] PROGMEM = "\x8D\xA5\xE2\x20\xAF\xA0\xAC\xEF\xE2\xA8"; // "Нет памяти"
const char ERR_DIV_BY_ZERO[] PROGMEM = "\x84\xA5\xAB\xA5\xAD\xA8\xA5\x20\xAD\xA0\x20\xAD\xAE\xAB\xEC"; // "Деление на ноль"
const char ERR_VARIABLE_NOT_FOUND[] PROGMEM = "\x8F\xA5\xE0\xA5\xAC\xA5\xAD\xAD\xA0\xEF\x20\xAD\xA5\x20\xAD\xA0\xA9\xA4\xA5\xAD\xA0"; // "Переменная не найдена"
const char ERR_BAD_COMMAND[] PROGMEM = "\x8D\xA5\xA2\xA5\xE0\xAD\xA0\xEF\x20\xAA\xAE\xAC\xA0\xAD\xA4\xA0"; // "Неверная команда"
const char ERR_BAD_LINE_NUMBER[] PROGMEM = "\x8D\xA5\xA2\xA5\xE0\xAD\xEB\xA9\x20\xAD\xAE\xAC\xA5\xE0\x20\xE1\xE2\xE0\xAE\xAA\xA8"; // "Неверный номер строки"
const char ERR_BREAK_PRESSED[] PROGMEM = "\x8F\xE0\xA5\xE0\xEB\xA2\xA0\xAD\xA8\xA5"; // "Прерывание"
const char ERR_NEXT_WITHOUT_FOR[] PROGMEM = "\x4E\x45\x58\x54\x20\xA1\xA5\xA7\x20\x46\x4F\x52"; // "NEXT без FOR"
const char ERR_STOP_STATEMENT[] PROGMEM = "\x8A\xAE\xAC\xA0\xAD\xA4\xA0\x20\x53\x54\x4F\x50"; // "Команда STOP"
const char ERR_MISSING_THEN[] PROGMEM = "\x8E\xE2\xE1\xE3\xE2\xE1\xE2\xA2\xE3\xA5\xE2\x20\x54\x48\x45\x4E\x20\xA2\x20\x49\x46"; // "Отсутствует THEN в IF"
const char ERR_RETURN_WITHOUT_GOSUB[] PROGMEM = "\x52\x45\x54\x55\x52\x4E\x20\xA1\xA5\xA7\x20\x47\x4F\x53\x55\x42"; // "RETURN без GOSUB"
const char ERR_WRONG_ARRAY_DIMS[] PROGMEM = "\x8D\xA5\xA2\xA5\xE0\xAD\xA0\xEF\x20\xE0\xA0\xA7\xAC\xA5\xE0\xAD\xAE\xE1\xE2\xEC\x20\xAC\xA0\xE1\xE1\xA8\xA2\xA0"; // "Неверная размерность массива"
const char ERR_BAD_ARRAY_INDEX[] PROGMEM = "\x8D\xA5\xA2\xA5\xE0\xAD\xEB\xA9\x20\xA8\xAD\xA4\xA5\xAA\xE1\x20\xAC\xA0\xE1\xE1\xA8\xA2\xA0"; // "Неверный индекс массива"
const char ERR_BAD_STRING_INDEX[] PROGMEM = "\x8D\xA5\xA2\xA5\xE0\xAD\xEB\xA9\x20\xA8\xAD\xA4\xA5\xAA\xE1\x20\xE1\xE2\xE0\xAE\xAA\xA8"; // "Неверный индекс строки"
const char ERR_ERROR_IN_VAL[] PROGMEM = "\x8E\xE8\xA8\xA1\xAA\xA0\x20\xA2\x20\x56\x41\x4C"; // "Ошибка в VAL"
const char ERR_BAD_PARAMETER[] PROGMEM = "\x8D\xA5\xA2\xA5\xE0\xAD\xEB\xA9\x20\xAF\xA0\xE0\xA0\xAC\xA5\xE2\xE0"; // "Неверный параметр"
const char ERR_OUT_OF_DATA[] PROGMEM = "\x87\xAD\xA0\xE7\xA5\xAD\xA8\xA5\x20\xA2\xAD\xA5\x20\xA3\xE0\xA0\xAD\xA8\xE6\xEB\x20\x44\x41\x54\x41";  // "Значение вне границы DATA"

// Массив указателей на сообщения об ошибках
const char* const errorTable[] PROGMEM = {
    ERR_OK,
    ERR_BAD_NUMBER,
    ERR_LINE_TOO_LONG,
    ERR_UNEXPECTED_INPUT,
    ERR_UNTERMINATED_STRING,
    ERR_MISSING_BRACKET,
    ERR_ERROR_IN_EXPR,
    ERR_NUMERIC_EXPR_EXPECTED,
    ERR_STRING_EXPR_EXPECTED,
    ERR_LINE_NUMBER_TOO_BIG,
    ERR_OUT_OF_MEMORY,
    ERR_DIV_BY_ZERO,
    ERR_VARIABLE_NOT_FOUND,
    ERR_BAD_COMMAND,
    ERR_BAD_LINE_NUMBER,
    ERR_BREAK_PRESSED,
    ERR_NEXT_WITHOUT_FOR,
    ERR_STOP_STATEMENT,
    ERR_MISSING_THEN,
    ERR_RETURN_WITHOUT_GOSUB,
    ERR_WRONG_ARRAY_DIMS,
    ERR_BAD_ARRAY_INDEX,
    ERR_BAD_STRING_INDEX,
    ERR_ERROR_IN_VAL,
    ERR_BAD_PARAMETER,
    ERR_OUT_OF_DATA
};

// ------------------------------------------------------------------
// Сообщения для EEPROM
// ------------------------------------------------------------------
const char EEPROM_NOT_AVAILABLE[] PROGMEM = "EEPROM\x20\xAD\xA5\x20\xA4\xAE\xE1\xE2\xE3\xAF\xAD\xA0"; // "EEPROM не доступна"
const char EEPROM_NOT_FOUND[] PROGMEM = "EEPROM\x20\xAD\xA5\x20\xAD\xA0\xA9\xA4\xA5\xAD\xA0"; // "EEPROM не найдена"
const char EEPROM_NEEDS_FORMAT[] PROGMEM = "EEPROM\x20\xE2\xE0\xA5\xA1\xE3\xA5\xE2\x20\xE4\xAE\xE0\xAC\xA0\xE2\xA8\xE0\xAE\xA2\xA0\xAD\xA8\xEF"; // "EEPROM требует форматирования"
const char EEPROM_FORMAT_COMPLETE[] PROGMEM = "EEPROM\x20\xAE\xE2\xE4\xAE\xE0\xAC\xA0\xE2\xA8\xE0\xAE\xA2\xA0\xAD\xA0"; // "EEPROM отформатирована"
const char EEPROM_SAVE_OK[] PROGMEM = "\x91\xAE\xE5\xE0\xA0\xAD\xA5\xAD\xAE"; // "Сохранено"
const char EEPROM_SAVE_FAILED[] PROGMEM = "\x8E\xE8\xA8\xA1\xAA\xA0\x20\xE1\xAE\xE5\xE0\xA0\xAD\xA5\xAD\xA8\xEF"; // "Ошибка сохранения"
const char EEPROM_LOAD_OK[] PROGMEM = "\x87\xA0\xA3\xE0\xE3\xA6\xA5\xAD\xAE";  // "Загружено"
const char EEPROM_LOAD_FAILED[] PROGMEM = "\x8E\xE8\xA8\xA1\xAA\xA0\x20\xA7\xA0\xA3\xE0\xA3\xA7\xAA\xA8"; // "Ошибка загрузки"
const char EEPROM_FILE_NOT_FOUND[] PROGMEM = "\x94\xA0\xA9\xAB\x20\xAD\xA5\x20\xAD\xA0\xA9\xA4\xA5\xAD"; // "Файл не найден"
const char EEPROM_PROGRAM_TOO_BIG[] PROGMEM = "\x8F\xE0\xAE\xA3\xE0\xA0\xAC\xAC\xA0\x20\xE1\xAB\xA8\xE8\xAA\xAE\xAC\x20\xA2\xA5\xAB\xA8\xAA\xA0"; // "Программа слишком велика"
const char EEPROM_NO_FILES[] PROGMEM = "\x8D\xA5\xE2\x20\xE4\xA0\xA9\xAB\xAE\xA2"; // "Нет файлов"
const char EEPROM_DELETE_OK[] PROGMEM = "\x93\xA4\xA0\xAB\xA5\xAD"; // "Удален"

// ------------------------------------------------------------------
// Сообщения для загрузки/сохранения
// ------------------------------------------------------------------
const char LOADING[] PROGMEM = "\x87\xA0\xA3\xE0\xE3\xA7\xAA\xA0\x20"; // "Загрузка "
const char SAVING[] PROGMEM = "\x91\xAE\xE5\xE0\xA0\xAD\xA5\xAD\xA8\xA5\x20"; // "Сохранение "
const char SIZE_BYTES[] PROGMEM = "\x20\xA1\xA0\xA9\xE2"; // " байт"
const char FIRST_BYTES[] PROGMEM = "\x8F\xA5\xE0\xA2\xEB\xA5\x20\xA1\xA0\xA9\xE2\xEB\x3A\x20"; // "Первые байты: "
const char COPIED_TO_MEMORY[] PROGMEM = "\x91\xAA\xAE\xAF\xA8\xE0\xAE\xA2\xA0\xAD\xAE\x20\xA2\x20\xAF\xA0\xAC\xEF\xE2\xEC\x20\x30\x2D"; // "Скопировано в память 0-"

// ------------------------------------------------------------------
// Сообщения для DIR
// ------------------------------------------------------------------
const char DIR_SIZE_OPEN[] PROGMEM = "(";
const char DIR_SIZE_CLOSE[] PROGMEM = "\x20\xA1\xA0\xA9\xE2\x29"; // " байт)"

// ------------------------------------------------------------------
// Сообщения для инициализации
// ------------------------------------------------------------------
const char INIT_MESSAGE[] PROGMEM = "\x9D\x82\x8C\x20\xA3\xAE\xE2\xAE\xA2\xA0\x2C\x20\x8F\x87\x93\x3A";  // "ЭВМ готова, ПЗУ:"
const char BYTES_FREE[] PROGMEM = "\xA1\xA0\xA9\xE2\x20\xE1\xA2\xAE\xA1\xAE\xA4\xAD\xAE"; // "байт свободно"

const char LAUNCHER_HELP_NEW[] PROGMEM = "\x46\x31\x20\x8D\xAE\xA2\xEB\xA9";     // "F1 Новый
const char LAUNCHER_HELP_EDIT[] PROGMEM = "\x46\x32\x20\x90\xA5\xA4\xA0\xAA";    // "F2 Редак
const char LAUNCHER_HELP_DELETE[] PROGMEM = "\x46\x33\x20\x93\xA4\xA0\xAB\x2E";  // "F3 Удал.
const char LAUNCHER_HELP_RUN[] PROGMEM = "\x46\x34\x20\x91\xE2\xA0\xE0\xE2";     // "F4 Старт

