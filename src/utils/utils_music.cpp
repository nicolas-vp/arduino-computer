#include <LiquidCrystal.h>
#include <Arduino.h>
#include "../keyboard/keyboard.h"

extern LiquidCrystal lcd;

const char* noteNames[12] = {
  "C", "C#", "D", "D#", "E", "F",
  "F#", "G", "G#", "A", "A#", "B"
};
// Частоты из твоего списка (начиная с B0 = индекс 0)
const int noteFreqs[] = {
  31,33,35,37,39,41,44,46,49,52,55,58,62,
  65,69,73,78,82,87,93,98,104,110,117,123,
  131,139,147,156,165,175,185,196,208,220,233,247,
  262,277,294,311,330,349,370,392,415,440,466,494,
  523,554,587,622,659,698,740,784,831,880,932,988,
  1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,
  2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,
  4186,4435,4699,4978
};
const int TOTAL_NOTES = sizeof(noteFreqs) / sizeof(noteFreqs[0]);


void playNote(int frequency) {
   int noteDuration = 1000 / 8;

    tone(8, frequency, noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;

    delay(pauseBetweenNotes);
    // noTone(8); // Убираем этот вызов, т.к. tone() сам отключает тон
}

int currentOctave = 4;
int selectedNote = 0;

int getGlobalIndex() {
  int index = (currentOctave * 12) + selectedNote - 12; 
  // -12 потому что список начинается с B0
  if (index < 0) index = 0;
  if (index >= TOTAL_NOTES) index = TOTAL_NOTES - 1;
  return index;
}

void printCell(int col, int row, int index) {
  lcd.setCursor(col * 6, row);

  if (index == selectedNote)
    lcd.print(">");
  else
    lcd.print(" ");

  lcd.print(noteNames[index]);
  lcd.print(currentOctave);

  lcd.print(" ");
}

void drawInterface() {
  lcd.clear();

  lcd.setCursor(19, 0);
  lcd.write(135);
  lcd.setCursor(19, 3);
  lcd.write(134);

  lcd.setCursor(19, 1);
  lcd.print(currentOctave);

  for (int i = 0; i < 12; i++) {
    int row = i / 3 ;
    int col = i % 3;

    printCell(col, row, i);
  }
}

void handleInput(unsigned char c) {
  if (c == KEY_RIGHT) {              // вправо
    if (selectedNote < 11) selectedNote++;
  }

  else if (c == KEY_LEFT) {         // влево
    if (selectedNote > 0) selectedNote--;
  }

  else if (c == KEY_UP) {         // вверх = октава +
    if (currentOctave < 8) currentOctave++;
  }

  else if (c == KEY_DOWN) {         // вниз = октава -
    if (currentOctave > 0) currentOctave--;
  }

  else if (c == ' ' or c == KEY_ENTER) {         // пробел = играть
    int index = getGlobalIndex();
    int freq = noteFreqs[index];
    playNote(freq);
  }

  drawInterface();
}

void musicPlayer() {
    lcd.clear();

    drawInterface();
    while (1) {
        unsigned char c = readKey();
        if (!c) continue;

        handleInput(c);

        // FN или ESC — выход
        if (c == 0x80 || c == 0x1B) {
            lcd.clear();
            return;
        }
    }
}

int parseNote(const char* noteStr) {

  char noteLetter = noteStr[0];   // C D E ...
  int index = -1;

  // базовые ноты
  switch (noteLetter) {
    case 'C': index = 0; break;
    case 'D': index = 2; break;
    case 'E': index = 4; break;
    case 'F': index = 5; break;
    case 'G': index = 7; break;
    case 'A': index = 9; break;
    case 'B': index = 11; break;
    default: return 0;
  }

  int pos = 1;

  // проверка на #
  if (noteStr[pos] == '#') {
    index++;
    pos++;
  }

  // читаем октаву
  int octave = noteStr[pos] - '0';

  // перевод в глобальный индекс
  int globalIndex = (octave * 12) + index - 12;

  if (globalIndex < 0 || globalIndex >= TOTAL_NOTES)
    return 0;

  return noteFreqs[globalIndex];
}

void play(const char* melody) {

  char buffer[8];          // максимум "A#10\0"
  int bufIndex = 0;

  for (int i = 0; ; i++) {

    char c = melody[i];

    if (c == ',' || c == '\0') {

      buffer[bufIndex] = '\0';

      int freq = parseNote(buffer);
      if (freq > 0) {
        playNote(freq);
        delay(300);
      }

      bufIndex = 0;

      if (c == '\0')
        break;
    }
    else {
      if (bufIndex < sizeof(buffer) - 1) {
        buffer[bufIndex++] = c;
      }
    }
  }
}
