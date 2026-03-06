#define EEPROM_H

#include <stdint.h>
#include <stddef.h>

// Конфигурация AT24C256
#define EEPROM_I2C_ADDR 0x50
#define EEPROM_SIZE 32768
#define EEPROM_PAGE_SIZE 64
#define MAX_FILES 20
#define FILENAME_LEN 16
#define CATALOG_SIZE 256
#define MAX_PROGRAM_SIZE (EEPROM_SIZE - CATALOG_SIZE)

// Структура записи файла в каталоге
typedef struct {
    char name[FILENAME_LEN];
    uint16_t offset;
    uint16_t size;
    uint8_t flags;
} FileEntry;

// Коды возврата для eeprom_init
#define EEPROM_OK 0
#define EEPROM_OK_EMPTY 1
#define EEPROM_ERR_NOT_FOUND 2
#define EEPROM_ERR_READ_FAIL 3

// Инициализация
int eeprom_init();

// Операции с файлами
bool eeprom_save_file(const char* filename, const uint8_t* data, size_t size);
bool eeprom_load_file(const char* filename, uint8_t* buffer, size_t* size);
bool eeprom_delete_file(const char* filename);
bool eeprom_list_files(void (*callback)(const char* name, uint16_t size));
bool eeprom_file_exists(const char* filename);
bool eeprom_sync();

// Управление каталогом
void eeprom_format();
void eeprom_purge_cache();
void eeprom_refresh_catalog();
void eeprom_check_catalog();
void eeprom_debug_dump(const char* filename);
void eeprom_test_basic();

// Низкоуровневые операции
bool eeprom_write_byte(uint16_t addr, uint8_t data);
uint8_t eeprom_read_byte(uint16_t addr);
bool eeprom_write_page(uint16_t addr, const uint8_t* data, uint8_t length);
bool eeprom_write_page_verified(uint16_t addr, const uint8_t* data, uint8_t length);
bool eeprom_read_buffer(uint16_t addr, uint8_t* buffer, size_t length);

void eeprom_check_address(const char* filename);

extern bool eeprom_initialized;