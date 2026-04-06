#include "eeprom.h"
#include <Arduino.h>
#include <Wire.h>
#include <string.h>

static FileEntry catalog[MAX_FILES];
static bool catalog_dirty = false;
bool eeprom_initialized = false;

#define LED_PIN 10  // Встроенный светодиод на Arduino

// Проверка валидности имени файла
static bool is_valid_filename(const char* name) {
    if (!name || !name[0] || strlen(name) >= FILENAME_LEN) return false;
    
    for (int i = 0; i < FILENAME_LEN && name[i]; i++) {
        char c = name[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != ' ') 
            return false;
    }
    return true;
}

// Проверка валидности записи каталога
static bool is_valid_entry(FileEntry* entry) {
    if (!(entry->flags & 0x01)) return false;
    
    if (!is_valid_filename(entry->name)) return false;
    if (entry->offset < CATALOG_SIZE || entry->offset >= EEPROM_SIZE) return false;
    // Разрешаем size = 0 для пустых файлов
    if (entry->size > MAX_PROGRAM_SIZE) return false;
    if (entry->offset + entry->size > EEPROM_SIZE) return false;
    
    return true;
}

// Проверка связи с устройством
static bool eeprom_check() {
    Wire.beginTransmission((uint8_t)EEPROM_I2C_ADDR);
    Wire.write(0);
    Wire.write(0);
    if (Wire.endTransmission() != 0) return false;
    
    delay(5);
    Wire.requestFrom((uint8_t)EEPROM_I2C_ADDR, (uint8_t)1);
    return Wire.available() > 0;
}

// Чтение буфера
bool eeprom_read_buffer(uint16_t addr, uint8_t* buffer, size_t length) {
    if (length == 0) return false;
    
    size_t remaining = length;
    uint16_t current_addr = addr;
    uint8_t* ptr = buffer;
    
    while (remaining > 0) {
        uint8_t chunk = (remaining > 32) ? 32 : (uint8_t)remaining;
        
        Wire.beginTransmission((uint8_t)EEPROM_I2C_ADDR);
        Wire.write((uint8_t)((current_addr >> 8) & 0xFF));
        Wire.write((uint8_t)(current_addr & 0xFF));
        
        if (Wire.endTransmission() != 0) return false;
        
        delay(1);
        
        uint8_t bytes_received = Wire.requestFrom((uint8_t)EEPROM_I2C_ADDR, chunk);
        
        if (bytes_received != chunk) return false;
        
        for (uint8_t i = 0; i < chunk; i++) {
            ptr[i] = Wire.read();
        }
        
        ptr += chunk;
        current_addr += chunk;
        remaining -= chunk;
        
        delay(1);
    }
    
    return true;
}

// Базовая функция записи страницы
bool eeprom_write_page(uint16_t addr, const uint8_t* data, uint8_t length) {
    if (length == 0 || length > EEPROM_PAGE_SIZE) return false;
    // Ограничиваем 30 байтами: 2 байта адреса + 30 данных = 32 (TWI буфер)
    if (length > 30) return false;

    uint8_t page_offset = addr % EEPROM_PAGE_SIZE;
    if (page_offset + length > EEPROM_PAGE_SIZE) return false;

    Wire.beginTransmission((uint8_t)EEPROM_I2C_ADDR);
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));

    for (uint8_t i = 0; i < length; i++) {
        Wire.write(data[i]);
    }

    int8_t status = Wire.endTransmission();
    if (status != 0) return false;

    delay(10);
    return true;
}

// Функция записи с верификацией
bool eeprom_write_page_verified(uint16_t addr, const uint8_t* data, uint8_t length) {
    if (!eeprom_write_page(addr, data, length)) {
        return false;
    }

    delay(20);

    // Верифицируем побайтово, без большого буфера
    for (uint8_t i = 0; i < length; i++) {
        uint8_t read_back = eeprom_read_byte(addr + i);
        if (read_back != data[i]) {
            return false;
        }
    }

    return true;
}

// Специальная функция для записи данных программы
bool eeprom_write_data_safe(uint16_t addr, const uint8_t* data, size_t size) {
    size_t remaining = size;
    uint16_t current_addr = addr;
    const uint8_t* ptr = data;
    
    while (remaining > 0) {
        uint8_t chunk = (remaining > 32) ? 32 : (uint8_t)remaining;
        
        for (uint8_t i = 0; i < chunk; i++) {
            if (!eeprom_write_byte(current_addr + i, ptr[i])) {
                return false;
            }
            delay(5);
        }
        
        for (uint8_t i = 0; i < chunk; i++) {
            uint8_t read_back = eeprom_read_byte(current_addr + i);
            if (read_back != ptr[i]) {
                return false;
            }
        }
        
        ptr += chunk;
        current_addr += chunk;
        remaining -= chunk;
        
        delay(10);
    }
    
    return true;
}

// ИНИЦИАЛИЗАЦИЯ EEPROM
int eeprom_init() {
    delay(50);
    
    if (!eeprom_check()) {
        eeprom_initialized = false;
        return EEPROM_ERR_NOT_FOUND;
    }
    
    bool read_success = false;
    bool has_valid_entries = false;
    
    for (int attempt = 0; attempt < 3; attempt++) {
        memset(catalog, 0, sizeof(catalog));
        bool has_error = false;
        has_valid_entries = false;
        
        for (int i = 0; i < MAX_FILES; i++) {
            uint16_t addr = i * sizeof(FileEntry);
            if (!eeprom_read_buffer(addr, (uint8_t*)&catalog[i], sizeof(FileEntry))) {
                has_error = true;
                break;
            }
            
            if ((catalog[i].flags & 0x01) && is_valid_entry(&catalog[i])) {
                has_valid_entries = true;
            }
            
            if ((catalog[i].flags & 0x01) && !is_valid_entry(&catalog[i])) {
                memset(&catalog[i], 0, sizeof(FileEntry));
            }
        }
        
        if (!has_error) {
            read_success = true;
            break;
        }
        delay(30);
    }
    
    if (!read_success) {
        eeprom_initialized = false;
        return EEPROM_ERR_READ_FAIL;
    }
    
    eeprom_initialized = true;
    return has_valid_entries ? EEPROM_OK : EEPROM_OK_EMPTY;
}

// Запись каталога
static bool flush_catalog() {
    if (!catalog_dirty || !eeprom_initialized) return true;
    
    bool all_success = true;
    
    for (int i = 0; i < MAX_FILES; i++) {
        uint16_t addr = i * sizeof(FileEntry);
        
        const uint8_t* data = (const uint8_t*)&catalog[i];
        uint8_t remaining = sizeof(FileEntry);
        uint16_t current_addr = addr;
        
        while (remaining > 0) {
            uint8_t page_offset = current_addr % EEPROM_PAGE_SIZE;
            uint8_t chunk = EEPROM_PAGE_SIZE - page_offset;
            if (chunk > remaining) chunk = remaining;
            
            if (!eeprom_write_data_safe(current_addr, data, chunk)) {
                all_success = false;
                break;
            }
            
            current_addr += chunk;
            data += chunk;
            remaining -= chunk;
            delay(10);
        }
    }
    
    if (all_success) {
        catalog_dirty = false;
    }
    
    return all_success;
}

bool eeprom_sync() {
    return flush_catalog();
}

static int find_free_slot() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!(catalog[i].flags & 0x01)) return i;
    }
    return -1;
}

static int find_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if ((catalog[i].flags & 0x01) && 
            strcasecmp(catalog[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

static uint16_t find_free_space(uint16_t size) {
    if (!eeprom_initialized) return 0;

    uint16_t max_offset = CATALOG_SIZE;

    for (int i = 0; i < MAX_FILES; i++) {
        if (catalog[i].flags & 0x01) {
            uint16_t end = catalog[i].offset + catalog[i].size;
            if (end > max_offset) {
                max_offset = end;
            }
            // Всегда продвигаемся за начало существующего файла
            // даже если он пустой (size=0)
            if (catalog[i].offset > max_offset) {
                max_offset = catalog[i].offset;
            }
        }
    }

    if (size == 0) {
        return max_offset;
    }

    if (max_offset + size > EEPROM_SIZE) {
        return 0;
    }

    return max_offset;
}

static void blink_error(int count) {
    pinMode(LED_PIN, OUTPUT);
    for (int i = 0; i < count * 2; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        delay(150);
    }
    delay(500);
}

bool eeprom_save_file(const char* filename, const uint8_t* data, size_t size) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    if (!eeprom_initialized) {
        blink_error(1);
        return false;
    }
    if (!filename || !data) {
        blink_error(2);
        return false;
    }
    if (size > MAX_PROGRAM_SIZE) {
        blink_error(3);
        return false;
    }

    // Удаляем старый файл с таким именем
    eeprom_delete_file(filename);

    int slot = find_free_slot();
    if (slot < 0) {
        blink_error(4);
        return false;
    }

    uint16_t offset = find_free_space((uint16_t)size);
    if (offset == 0) {
        blink_error(5);
        return false;
    }

    // Записываем данные только если size > 0
    if (size > 0) {
        size_t i = 0;
        while (i < size) {
            uint8_t page_offset = (offset + i) % EEPROM_PAGE_SIZE;
            uint8_t chunk = EEPROM_PAGE_SIZE - page_offset;
            if (chunk > (size - i)) chunk = (uint8_t)(size - i);
            // Ограничиваем 30 байтами: 2 байта адреса + 30 данных = 32 (TWI буфер)
            if (chunk > 30) chunk = 30;

            if (!eeprom_write_page_verified((uint16_t)(offset + i), &data[i], chunk)) {
                blink_error(6);
                return false;
            }
            i += chunk;
        }
    }

    // Обновляем каталог
    char safe_name[FILENAME_LEN];
    strncpy(safe_name, filename, FILENAME_LEN-1);
    safe_name[FILENAME_LEN-1] = '\0';

    strncpy(catalog[slot].name, safe_name, FILENAME_LEN-1);
    catalog[slot].name[FILENAME_LEN-1] = '\0';
    catalog[slot].offset = offset;
    catalog[slot].size = (uint16_t)size;
    catalog[slot].flags = 0x01;
    catalog_dirty = true;

    if (!flush_catalog()) {
        blink_error(7);
        return false;
    }

    digitalWrite(LED_PIN, LOW);
    return true;
}

bool eeprom_load_file(const char* filename, uint8_t* buffer, size_t* size) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    bool result = false;
    
    if (!eeprom_initialized || !filename || !buffer || !size) {
        digitalWrite(LED_PIN, LOW);
        return false;
    }
    
    int slot = find_file(filename);
    if (slot < 0) {
        digitalWrite(LED_PIN, LOW);
        return false;
    }
    
    if (catalog[slot].offset < CATALOG_SIZE) {
        digitalWrite(LED_PIN, LOW);
        return false;
    }
    
    if (catalog[slot].size == 0) {
        *size = 0;
        result = true;
        digitalWrite(LED_PIN, LOW);
        return result;
    }

    if (!eeprom_read_buffer(catalog[slot].offset, buffer, catalog[slot].size)) {
        digitalWrite(LED_PIN, LOW);
        return false;
    }

    *size = catalog[slot].size;
    result = true;

    digitalWrite(LED_PIN, LOW);
    return result;
}


bool eeprom_delete_file(const char* filename) {
    if (!eeprom_initialized) return false;
    
    int slot = find_file(filename);
    if (slot < 0) return false;
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    memset(&catalog[slot], 0, sizeof(FileEntry));
    catalog_dirty = true;
    flush_catalog();
    eeprom_refresh_catalog();
    
    digitalWrite(LED_PIN, LOW);
    return true;
}

bool eeprom_list_files(void (*callback)(const char* name, uint16_t size)) {
    if (!eeprom_initialized || !callback) return false;
    
    bool found = false;
    for (int i = 0; i < MAX_FILES; i++) {
        if (catalog[i].flags & 0x01) {
            if (is_valid_entry(&catalog[i])) {
                callback(catalog[i].name, catalog[i].size);
                found = true;
            }
        }
    }
    return found;
}

bool eeprom_file_exists(const char* filename) {
    return find_file(filename) >= 0;
}

// Низкоуровневые функции
bool eeprom_write_byte(uint16_t addr, uint8_t data) {
    Wire.beginTransmission((uint8_t)EEPROM_I2C_ADDR);
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

uint8_t eeprom_read_byte(uint16_t addr) {
    Wire.beginTransmission((uint8_t)EEPROM_I2C_ADDR);
    Wire.write((uint8_t)((addr >> 8) & 0xFF));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.endTransmission();
    
    Wire.requestFrom((uint8_t)EEPROM_I2C_ADDR, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0xFF;
}

// Функции управления каталогом
void eeprom_refresh_catalog() {
    if (!eeprom_initialized) return;
    
    for (int i = 0; i < MAX_FILES; i++) {
        uint16_t addr = i * sizeof(FileEntry);
        FileEntry temp;
        
        if (eeprom_read_buffer(addr, (uint8_t*)&temp, sizeof(FileEntry))) {
            if ((temp.flags & 0x01) && is_valid_entry(&temp)) {
                memcpy(&catalog[i], &temp, sizeof(FileEntry));
            } else {
                memset(&catalog[i], 0, sizeof(FileEntry));
            }
        } else {
            memset(&catalog[i], 0, sizeof(FileEntry));
        }
    }
    
    catalog_dirty = false;
}

void eeprom_purge_cache() {
    memset(catalog, 0, sizeof(catalog));
    catalog_dirty = true;
    flush_catalog();
    eeprom_refresh_catalog();
}

void eeprom_format() {
    // Включаем светодиод на время работы
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    uint8_t zero_buffer[EEPROM_PAGE_SIZE] = {0};
    
    memset(catalog, 0, sizeof(catalog));
    
    for (int addr = 0; addr < CATALOG_SIZE; addr += EEPROM_PAGE_SIZE) {
        uint8_t chunk = (CATALOG_SIZE - addr > EEPROM_PAGE_SIZE) ? 
                         EEPROM_PAGE_SIZE : (uint8_t)(CATALOG_SIZE - addr);
        
        if (!eeprom_write_data_safe(addr, zero_buffer, chunk)) {
            digitalWrite(LED_PIN, LOW);
            return;
        }
    }
    
    eeprom_refresh_catalog();
    catalog_dirty = false;
    eeprom_initialized = true;
    
    // Выключаем светодиод
    digitalWrite(LED_PIN, LOW);
}

int eeprom_find_file(const char* filename) {
    return find_file(filename);
}