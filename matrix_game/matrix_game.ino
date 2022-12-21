#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

// ------------- LCD -----------------
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 3;
const byte d6 = 5;
const byte d7 = 4;
const byte LCDbrightnessPin = 6;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//-------------- MATRIX -----------------
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte size = 8;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

//------------ JOYSTICK -----------------
const byte pinSW = 2;
const byte pinX = A0;
const byte pinY = A1;

bool swState = LOW;
bool lastSwState = LOW;
byte state = 0;
byte switchState = HIGH;

int xValue = 0;
int yValue = 0;

const String menuOptions[5] = {
  "Start game",
  "Leaderboard",
  "Settings",
  "Instructions",
  "About"
};

const String settingsOptions[5] = {
  "Player name",
  "LCD bright.",
  "Matrix bright.",
  "Difficulty",
  "Reset HighScore"
};

char lettersOfName[4] = { 'X', 'X', 'X', 'X' };

byte arrows[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100
};

const byte smile[8] = {
  0b00000000,
  0b0000000,
  0b00100100,
  0b00100100,
  0b00000000,
  0b11111111,
  0b01000010,
  0b00111100
};

byte matrix[size][size] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte namePos = 0;
byte menuNow = 0;

byte brightnessMatrix = 1;
byte scrollingAboutText = 0;
byte scrollingIntructionsText = 0;

byte submenuOption = 0;

bool joyX = LOW;
bool joyY = LOW;

const int minThreshold = 400;
const int maxThreshold = 600;

unsigned long lastDebounceTime = 0;
unsigned long lastScrollTime = 0;
const byte debounceDelay = 50;

byte score = 0;

byte foodRowNow = 0;
byte foodColumnNow = 0;
byte foodRowLast = 0;
byte foodColumnLast = 0;

byte textScrolling = 0;

byte leaderboard = 0;
byte leaderboardPos = 0;

byte settings = 0;
byte settingsPos = 0;
byte hsBeaten = 0;

byte startGame = 0;

const String aboutText = " Roxana Enasoaie. github:@roxanaenasoaie ";
const String messageHTP = "Eat the food to increase score. The game ends when the snake hits itself.";

byte LCDbrightness;
byte difficulty;

unsigned long lastBlink = 0;
byte blink = LOW;
const int blinkInterval = 250;
unsigned long lastMoved = 0;
unsigned long lastLetterBlink = 0;

byte highscores[5];
String highscoreNames[5] = { "", "", "", "", "" };

byte snakeRow = 0;
byte snakeColumn = 1;
byte snakeLength = 2;
int snake[30][2];
int updateInterval;
unsigned long lastGameUpdate = 0;
int directionRow = 0;
int directionColumn = 1;


// -------------- GAME FUNCTIONS -----------------------
void gameDifficulty() {
  if (difficulty == 1)
    updateInterval = 750;
  else if (difficulty == 2) {
    updateInterval = 400;
  } else if (difficulty == 3){
      updateInterval = 250;
    }
}

bool ateTheFood(byte row, byte column) {
  for (int i = 0; i < snakeLength; ++i)
    if (snake[i][0] == row && snake[i][1] == column)
      return true;
  return false;
}

void generateFood() {
  foodRowLast = foodRowNow;
  foodColumnLast = foodColumnNow;

  foodRowNow = random(0, size);
  foodColumnNow = random(0, size);

  while (ateTheFood(foodRowNow, foodColumnNow)) {
    foodRowNow = random(0, size);
    foodColumnNow = random(0, size);
  }
  matrix[foodRowLast][foodColumnLast] = 0;
  matrix[foodRowNow][foodColumnNow] = 1;
}

void update() {
  for (int row = 0; row < size; row++) {
    for (int col = 0; col < size; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

void reset() {
  for (int row = 0; row < size; row++) {
    for (int col = 0; col < size; col++) {
      matrix[row][col] = 0;
    }
  }
}

void resetToMenu() {
  lcd.clear();
  lcd.print("MENU");
  lcd.setCursor(15, 0);
  lcd.write(byte(0));
  lcd.setCursor(0, 1);
  lcd.print("->");
  lcd.print(menuOptions[menuNow]);
  lc.clearDisplay(0);
}

int positionUpdate() {
  int snakeRowNew = snakeRow + directionRow;
  int snakeColumnNew = snakeColumn + directionColumn;

  if (snakeRowNew == size) {
    snakeRowNew = 0;
  } else if (snakeRowNew == -1) {
    snakeRowNew = size - 1;
  } else if (snakeColumnNew == size) {
    snakeColumnNew = 0;
  } else if (snakeColumnNew == -1) {
    snakeColumnNew = size - 1;
  }

  for (int i = 0; i < snakeLength; ++i) {
    if (snake[i][0] == snakeRowNew && snake[i][1] == snakeColumnNew) {
      exitGame();
      return -1;
    }
  }

  if (snakeRowNew == foodRowNow && snakeColumnNew == foodColumnNow) {
    score++;
    snakeLength++;

    if (difficulty == 2 || difficulty == 3) {
      snakeLength++;
      score++;
    }

    lcd.setCursor(8, 1);
    lcd.print(score);
    generateFood();

  } else {
    for (int i = 1; i < snakeLength; ++i) {
      snake[i - 1][0] = snake[i][0];
      snake[i - 1][1] = snake[i][1];
    }
  }

  snake[snakeLength - 1][0] = snakeRowNew;
  snake[snakeLength - 1][1] = snakeColumnNew;

  snakeRow = snakeRowNew;
  snakeColumn = snakeColumnNew;

  matrix[snake[0][0]][snake[0][1]] = 0;
  matrix[snakeRow][snakeColumn] = 1;
  return 0;
}

void game() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (xValue < minThreshold && directionRow == 0) {
    directionRow = 1;
    directionColumn = 0;
  }

  if (xValue > maxThreshold && directionRow == 0) {
    directionRow = -1;
    directionColumn = 0;
  }

  if (yValue < minThreshold && directionColumn == 0) {
    directionRow = 0;
    directionColumn = -1;
  }
  if (yValue > maxThreshold && directionColumn == 0) {
    directionRow = 0;
    directionColumn = 1;
  }

  if (millis() - lastBlink > blinkInterval && difficulty != 3) {
    matrix[foodRowNow][foodColumnNow] = !matrix[foodRowNow][foodColumnNow];
    lastBlink = millis();
  } else if (difficulty == 3) {
    if (matrix[foodRowNow][foodColumnNow] == 1) {
      if (millis() - lastBlink > blinkInterval) {
        matrix[foodRowNow][foodColumnNow] = 0;
        lastBlink = millis();
      }
    } else {
      if (millis() - lastBlink > 3 * blinkInterval) {
        matrix[foodRowNow][foodColumnNow] = 1;
        lastBlink = millis();
      }
    }
  }
  
  if (millis() - lastGameUpdate > updateInterval) {
    lastGameUpdate = millis();
    int result = positionUpdate();
    if (result == -1)
      return;
  }
  update();
}

void exitGame() {
  for (int row = 0; row < size; row++) {
    for (int col = 0; col < size; col++) {
      lc.setLed(0, row, col, true);
    }
  }

  directionRow = 0;
  directionColumn = 1;
  lastGameUpdate = 0;
  snakeRow = 0;
  snakeColumn = 1;
  snakeLength = 2;
  
  for (int i = 0; i < 40; ++i) {
    snake[i][0] = -1;
    snake[i][1] = -1;
  }

  reset();
  snake[0][0] = 0;
  snake[0][1] = 0;
  snake[1][0] = 0;
  snake[1][1] = 1;
  lcd.clear();

  menuNow = 0;
  submenuOption = 0;
  startGame = 0;

  lcd.print(lettersOfName[0]);
  lcd.print(lettersOfName[1]);
  lcd.print(lettersOfName[2]);
  lcd.print(lettersOfName[3]);
  lcd.print(" DEAD!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);

  delay(500);

  String newName = "";
  newName += lettersOfName[0];
  newName += lettersOfName[1];
  newName += lettersOfName[2];
  newName += lettersOfName[3];
  updateHighscore(score, newName);
  getData();

  score = 0;
  delay(2000);

  if (hsBeaten != 0) {
    for (int row = 0; row < size; row++) {
      lc.setRow(0, row, smile[row]);
    }

    lcd.clear();
    lcd.print("Congrats! You're now");
    lcd.setCursor(0, 1);
    lcd.print("#");
    lcd.print(hsBeaten);
    lcd.print(" on leaderboard!");
    hsBeaten = 0;

    state = 4;
  } else {
    state = 1;
    resetToMenu();
  }
}


// ---------- EEPROM RELATED ----------

// memory layout:
// 1 - Matrix brightness
// 2 - LCD brightness
// 3 - First highscore
// 4 -> 9 - First highscore name
// 10 - Second highscore
// 11 -> 16 - Second highscore name
// 17 - Third highscore
// 18 -> 23 - Third highscore name
// 24 - Fourth highscore
// 25 -> 30 - Fourth highscore name
// 31 - Fifth highscore
// 32 -> 37 - Fifth highscore name
// 38 - Difficulty

void startingHighscore() {
  for (int i = 1; i <= 5; ++i) {
    EEPROM.update(7 * i - 4, 0);
    EEPROM.update(7 * i - 3, 'X');
    EEPROM.update(7 * i - 2, 'X');
    EEPROM.update(7 * i - 1, 'X');
    EEPROM.update(7 * i, 'X');
  }
}

void updateHighscore(int newScore, String newName) {
  if (newScore > highscores[0]) {
    hsBeaten = 1;
    EEPROM.update(31, EEPROM.read(24));
    EEPROM.update(32, EEPROM.read(25));
    EEPROM.update(33, EEPROM.read(26));
    EEPROM.update(34, EEPROM.read(27));
    EEPROM.update(35, EEPROM.read(28));

    EEPROM.update(24, EEPROM.read(17));
    EEPROM.update(25, EEPROM.read(18));
    EEPROM.update(26, EEPROM.read(19));
    EEPROM.update(27, EEPROM.read(20));
    EEPROM.update(28, EEPROM.read(21));

    EEPROM.update(17, EEPROM.read(10));
    EEPROM.update(18, EEPROM.read(11));
    EEPROM.update(19, EEPROM.read(12));
    EEPROM.update(20, EEPROM.read(13));
    EEPROM.update(21, EEPROM.read(14));

    EEPROM.update(10, EEPROM.read(3));
    EEPROM.update(11, EEPROM.read(4));
    EEPROM.update(12, EEPROM.read(5));
    EEPROM.update(13, EEPROM.read(6));
    EEPROM.update(14, EEPROM.read(7));

    EEPROM.update(3, newScore);
    EEPROM.update(4, newName[0]);
    EEPROM.update(5, newName[1]);
    EEPROM.update(6, newName[2]);
    EEPROM.update(7, newName[3]);
  } else if (newScore > highscores[1]) {
    hsBeaten = 2;
    EEPROM.update(31, EEPROM.read(24));
    EEPROM.update(32, EEPROM.read(25));
    EEPROM.update(33, EEPROM.read(26));
    EEPROM.update(34, EEPROM.read(27));
    EEPROM.update(35, EEPROM.read(28));

    EEPROM.update(24, EEPROM.read(17));
    EEPROM.update(25, EEPROM.read(18));
    EEPROM.update(26, EEPROM.read(19));
    EEPROM.update(27, EEPROM.read(20));
    EEPROM.update(28, EEPROM.read(21));

    EEPROM.update(17, EEPROM.read(10));
    EEPROM.update(18, EEPROM.read(11));
    EEPROM.update(19, EEPROM.read(12));
    EEPROM.update(20, EEPROM.read(13));
    EEPROM.update(21, EEPROM.read(14));


    EEPROM.update(10, newScore);
    EEPROM.update(11, newName[0]);
    EEPROM.update(12, newName[1]);
    EEPROM.update(13, newName[2]);
    EEPROM.update(14, newName[3]);
  } else if (newScore > highscores[2]) {
    hsBeaten = 3;
    EEPROM.update(31, EEPROM.read(24));
    EEPROM.update(32, EEPROM.read(25));
    EEPROM.update(33, EEPROM.read(26));
    EEPROM.update(34, EEPROM.read(27));
    EEPROM.update(35, EEPROM.read(28));

    EEPROM.update(24, EEPROM.read(17));
    EEPROM.update(25, EEPROM.read(18));
    EEPROM.update(26, EEPROM.read(19));
    EEPROM.update(27, EEPROM.read(20));
    EEPROM.update(28, EEPROM.read(21));

    EEPROM.update(17, newScore);
    EEPROM.update(18, newName[0]);
    EEPROM.update(19, newName[1]);
    EEPROM.update(20, newName[2]);
    EEPROM.update(21, newName[3]);
  } else if (newScore > highscores[3]) {
    hsBeaten = 4;
    EEPROM.update(31, EEPROM.read(24));
    EEPROM.update(32, EEPROM.read(25));
    EEPROM.update(33, EEPROM.read(26));
    EEPROM.update(34, EEPROM.read(27));
    EEPROM.update(35, EEPROM.read(28));

    EEPROM.update(24, newScore);
    EEPROM.update(25, newName[0]);
    EEPROM.update(26, newName[1]);
    EEPROM.update(27, newName[2]);
    EEPROM.update(28, newName[3]);
  } else if (newScore > highscores[4]) {
    hsBeaten = 5;
    EEPROM.update(31, newScore);
    EEPROM.update(32, newName[0]);
    EEPROM.update(33, newName[1]);
    EEPROM.update(34, newName[2]);
    EEPROM.update(35, newName[3]);
  }
}

void getData() {
  brightnessMatrix = EEPROM.read(1);
  LCDbrightness = EEPROM.read(2);
  difficulty = EEPROM.read(38);

  for (int i = 1; i <= 5; ++i) {
    highscores[i - 1] = EEPROM.read(7 * i - 4);
    String name;
    int addr = 7 * i - 3;
    EEPROM.get(addr, name);
    highscoreNames[i - 1] = name;
  }
}

// ------------ JOYSTICK FUNCTIONS ------------
void button() {
  if (swState != lastSwState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (swState != switchState) {
      switchState = swState;
      if (switchState == LOW) {
        if (state == 0 && startGame == 0 || state == 4) {
          state = 1;
          lcd.clear();
          resetToMenu();

        } else if (submenuOption == 1) {
          startGame = 1;

          lcd.clear();
          lcd.print("PLAYER: ");
          lcd.print(lettersOfName[0]);
          lcd.print(lettersOfName[1]);
          lcd.print(lettersOfName[2]);
          lcd.print(lettersOfName[3]);
          lcd.setCursor(0, 1);
          lcd.print("SCORE: ");
          lcd.print(score);
          lc.clearDisplay(0);
          generateFood();
        } else if (state == 3 && startGame == 0 && settings == 1 && settingsPos == 0) {
          state = 2;
          lcd.clear();
          lcd.print("SETTINGS");
          lcd.setCursor(15, 0);
          lcd.write(byte(0));
          lcd.setCursor(0, 1);
          lcd.print("->");
          lcd.print(settingsOptions[settingsPos]);
          lc.clearDisplay(0);
        }

      } else if (state == 3 && startGame == 0 && settings == 1 && settingsPos == 5) {
        startingHighscore();
        getData();
        state = 2;
        lcd.clear();
        lcd.print("SETTINGS");
        lcd.setCursor(15, 0);
        lcd.write(byte(0));
        lcd.setCursor(0, 1);
        lcd.print("->");
        lcd.print(settingsOptions[settingsPos]);
        lc.clearDisplay(0);
      }
    }
  }
  lastSwState = swState;
}

void upDown() {
  if (yValue > maxThreshold && joyY == LOW && state == 1 && startGame == 0) {  
    state = 2;
    if (menuNow == 0) {
      submenuOption = 1;
      lcd.clear();
      lcd.print("<- START GAME");
      lcd.setCursor(0, 1);
      lcd.print("Press to start");

    } else if (menuNow == 1) {
      lcd.clear();
      lcd.print("<- LEADERBOARD");
      lcd.setCursor(15, 0);
      lcd.write(byte(0));
      lcd.setCursor(0, 1);
      lcd.setCursor(0, 1);
      lcd.print(leaderboardPos + 1);
      lcd.print(".");
      lcd.print(highscoreNames[leaderboardPos]);
      lcd.print(" - ");
      lcd.print(highscores[leaderboardPos]);
      lcd.print(" pct");

      leaderboard = 1;
    } else if (menuNow == 2) {
      lcd.clear();
      lcd.print("<- SETTINGS");
      settings = 1;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(settingsOptions[settingsPos]);

    } else if (menuNow == 3) {
      lcd.clear();
      lcd.print("<- INSTRUCTIONS");
      lcd.setCursor(0, 1);
      scrollingIntructionsText = 1;

    } else if (menuNow == 4) {
      lcd.clear();
      lcd.print("<- ABOUT");
      lcd.setCursor(0, 1);
      scrollingAboutText = 1;
    }
    joyY = HIGH;
  } else if (yValue < minThreshold && joyY == LOW && state == 2 && startGame == 0) {  
    state = 1;
    scrollingAboutText = 0;
    scrollingIntructionsText = 0;
    textScrolling = 0;
    leaderboardPos = 0;
    submenuOption = 0;
    leaderboard = 0;
    settingsPos = 0;
    settings = 0;
    lcd.clear();
    resetToMenu();

    joyY = HIGH;
  } else if (yValue > maxThreshold && joyY == LOW && state == 2 && startGame == 0 && settings == 1) {  
    state = 3;
    lcd.clear();
    if (settingsPos == 0) {
      lcd.print("NAME  Press to");
      lcd.setCursor(0, 1);
      lcd.print("   ");

      lcd.print(lettersOfName[0]);
      lcd.print(lettersOfName[1]);
      lcd.print(lettersOfName[2]);
      lcd.print(lettersOfName[3]);

      lcd.print("    SAVE");

    } else if (settingsPos == 1) {
      lcd.print("<- Level: ");
      lcd.setCursor(4, 1);
      lcd.print("-");
      lcd.setCursor(10, 1);
      lcd.print("+");

    } else if (settingsPos == 2) {
      for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
          lc.setLed(0, row, col, true);
        }
      }
      lcd.print("<- MATRIX BRIGHT.");
      lcd.setCursor(4, 1);
      lcd.print("-");
      lcd.setCursor(10, 1);
      lcd.print("+");

    } else if (settingsPos == 3) {
      lcd.print("<- DIFFICULTY");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print(" ");
      if (difficulty == 1) {
        lcd.print("BEGGINER");
      } else if (difficulty == 2) {
        lcd.print("INTERMEDIATE");
      } else if (difficulty == 3) {
        lcd.print("EXPERT");
      }

    } else if (settingsPos == 4) {
      lcd.print("<- RESET HighScore");
      lcd.setCursor(0, 1);
      lcd.print("Press to reset");
    }
    joyY = HIGH;
  } else if (yValue < minThreshold && joyY == LOW && state == 3 && startGame == 0 && settings == 1 and settingsPos != 0) {  
    state = 2;
    lcd.clear();
    lcd.print("<- SETTINGS");
    lcd.setCursor(15, 0);
    lcd.write(byte(0));
    lcd.setCursor(0, 1);
    lcd.print("->");
    lcd.print(settingsOptions[settingsPos]);
    lc.clearDisplay(0);


    joyY = HIGH;
  } else if (yValue < minThreshold && joyY == LOW && state == 3 && startGame == 0 && settings == 1 and settingsPos == 0) {  
    if (namePos > 0) {
      lcd.setCursor(namePos + 4, 1);
      lcd.print(lettersOfName[namePos]);
      namePos--;
    }
    joyY = HIGH;
  } else if (yValue > maxThreshold && joyY == LOW && state == 3 && startGame == 0 && settings == 1 and settingsPos == 0) { 
    if (namePos < 2) {
      lcd.setCursor(namePos + 4, 1);
      lcd.print(lettersOfName[namePos]);
      namePos++;
    }
    joyY = HIGH;
  } else if (joyY == HIGH && yValue < maxThreshold && yValue > minThreshold && startGame == 0) {
    joyY = LOW;
  }
}

void leftRight() {
  if (xValue < minThreshold && joyX == LOW && state == 1 && startGame == 0 && leaderboard == 0) {  
    if (menuNow < 4) {
      menuNow++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(menuOptions[menuNow]);
      lc.clearDisplay(0);


    } else if (menuNow == 4) {
      menuNow = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(menuOptions[menuNow]);
      lc.clearDisplay(0);
    }
    joyX = HIGH;
  } else if (xValue > maxThreshold && joyX == LOW && state == 1 && startGame == 0 && leaderboard == 0) {  
    if (menuNow > 0) {
      menuNow--;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(menuOptions[menuNow]);
      lc.clearDisplay(0);


    } else if (menuNow == 0) {
      menuNow = 4;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(menuOptions[menuNow]);
      lc.clearDisplay(0);
    }
    joyX = HIGH;
  } else if (xValue < minThreshold && joyX == LOW && state == 2 && startGame == 0 && leaderboard == 1) {  
    if (leaderboardPos < 4) {
      leaderboardPos++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(leaderboardPos + 1);
      lcd.print(".");
      lcd.print(highscoreNames[leaderboardPos]);
      lcd.print(" - ");
      lcd.print(highscores[leaderboardPos]);
      lcd.print(" pct");
    }
    joyX = HIGH;
  } else if (xValue > maxThreshold && joyX == LOW && state == 2 && startGame == 0 && leaderboard == 1) {  
    if (leaderboardPos > 0) {
      leaderboardPos--;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(leaderboardPos + 1);
      lcd.print(".");
      lcd.print(highscoreNames[leaderboardPos]);
      lcd.print(" - ");
      lcd.print(highscores[leaderboardPos]);
      lcd.print(" pct");
    }
    joyX = HIGH;
  } else if (xValue < minThreshold && joyX == LOW && state == 2 && startGame == 0 && settings == 1) {  
    if (settingsPos < 5) {
      settingsPos++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(settingsOptions[settingsPos]);

    } else if (settingsPos == 5) {
      settingsPos = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(settingsOptions[settingsPos]);
    }
    joyX = HIGH;
  } else if (xValue > maxThreshold && joyX == LOW && state == 2 && startGame == 0 && settings == 1) { 
    if (settingsPos > 0) {
      settingsPos--;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(settingsOptions[settingsPos]);

    } else if (settingsPos == 0) {
      settingsPos = 5;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("->");
      lcd.print(settingsOptions[settingsPos]);
    }
    joyX = HIGH;
  } else if (xValue < minThreshold && joyX == LOW && state == 3 && startGame == 0 && settings == 1) {  
    if (settingsPos == 0) {                                                                           
      if (lettersOfName[namePos] > 'A') {
        lettersOfName[namePos]--;
        lcd.setCursor(namePos + 4, 1);
        lcd.print(lettersOfName[namePos]);
      } else if (lettersOfName[namePos] == 'A') {
        lettersOfName[namePos] = 'Z';
        lcd.setCursor(namePos + 4, 1);
        lcd.print(lettersOfName[namePos]);
      }
    } else if (settingsPos == 1) {  
      if (LCDbrightness > 1) {
        LCDbrightness--;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        lcd.setCursor(10, 1);
        lcd.print("+");
        analogWrite(LCDbrightnessPin, LCDbrightness * 51);
        EEPROM.update(2, LCDbrightness);
      }
    } else if (settingsPos == 2) {  
      if (brightnessMatrix > 1) {
        brightnessMatrix--;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        lcd.setCursor(10, 1);
        lcd.print("+");
        lc.setIntensity(0, 3 * brightnessMatrix);
        EEPROM.update(1, brightnessMatrix);
      }
    } else if (settingsPos == 3) {  
      if (difficulty > 1) {
        difficulty--;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.print(" ");
        if (difficulty == 1) {
          lcd.print("BEGGINER");
        } else if (difficulty == 2) {
          lcd.print("INTERMEDIATE");
        } else if (difficulty == 3) {
          lcd.print("EXPERT");
        }
        EEPROM.update(38, difficulty);
      }
    }

    joyX = HIGH;
  } else if (xValue > maxThreshold && joyX == LOW && state == 3 && startGame == 0 && settings == 1) { 
    if (settingsPos == 0) {
      if (lettersOfName[namePos] < 'Z') {
        lettersOfName[namePos]++;
        lcd.setCursor(namePos + 4, 1);
        lcd.print(lettersOfName[namePos]);
      } else if (lettersOfName[namePos] == 'Z') {
        lettersOfName[namePos] = 'A';
        lcd.setCursor(namePos + 4, 1);
        lcd.print(lettersOfName[namePos]);
      }
    } else if (settingsPos == 1) {
      if (LCDbrightness < 5) {
        LCDbrightness++;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        lcd.setCursor(10, 1);
        lcd.print("+");
        analogWrite(LCDbrightnessPin, LCDbrightness * 51);
        EEPROM.update(2, LCDbrightness);
      }
    } else if (settingsPos == 2) {
      if (brightnessMatrix < 5) {
        brightnessMatrix++;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(4, 1);
        lcd.print("-");
        lcd.setCursor(10, 1);
        lcd.print("+");
        lc.setIntensity(0, 3 * brightnessMatrix);
        EEPROM.update(1, brightnessMatrix);
      }
    } else if (settingsPos == 3) {
      if (difficulty < 3) {
        difficulty++;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.print(" ");
        if (difficulty == 1) {
          lcd.print("BEGGINER");
        } else if (difficulty == 2) {
          lcd.print("INTERMEDIATE");
        } else if (difficulty == 3) {
          lcd.print("EXPERT");
        }
        EEPROM.update(38, difficulty);
      }
    }
    joyX = HIGH;
  } else if (joyX == HIGH && xValue < maxThreshold && xValue > minThreshold && startGame == 0) {
    joyX = LOW;
  }
}


void setup() {
  getData();
  gameDifficulty();

  analogWrite(LCDbrightnessPin, LCDbrightness * 51);

  pinMode(pinSW, INPUT_PULLUP);
  
  randomSeed(analogRead(A5));
  lcd.begin(16, 2);
  lcd.print("HI :)");
  lcd.setCursor(0, 1);
  lcd.print(" Press to begin");

  lcd.createChar(0, arrows);
  
  lc.shutdown(0, false);
  lc.setIntensity(0, 3 * brightnessMatrix);
  lc.clearDisplay(0);
  for (int row = 0; row < size; row++) {
    lc.setRow(0, row, smile[row]);
  }

  snake[0][0] = 0;
  snake[0][1] = 0;
  snake[1][0] = 0;
  snake[1][1] = 1;
}

void loop() {
  if (startGame == 1) {
    gameDifficulty();
    game();
  } else {
    blinking();
    scrollAbout();
    scrollInstructions();

    swState = digitalRead(pinSW);
    xValue = analogRead(pinX);
    yValue = analogRead(pinY);

    button();
    upDown();
    leftRight();
  }
}



void blinking() {
  if (state == 3 && settings == 1 && settingsPos == 0) {
    if (millis() - lastLetterBlink > 400) {
      lastLetterBlink = millis();
      blink = !blink;
    }
    lcd.setCursor(namePos + 4, 1);
    if (blink == HIGH) {
      lcd.print(lettersOfName[namePos]);
    } else {
      lcd.print(" ");
    }
  }
}

// SCROLLING TEXTS
void scrollAbout() {
  if (scrollingAboutText == 1 && textScrolling < aboutText.length() - 15) {
    lcd.setCursor(0, 1);
    if (millis() - lastScrollTime >= 400) {
      lastScrollTime = millis();
      lcd.setCursor(0, 1);
      lcd.print(aboutText.substring(textScrolling, 16 + textScrolling));
      textScrolling++;
    }
  }
}

void scrollInstructions() {
  if (scrollingIntructionsText == 1 && textScrolling < messageHTP.length() - 15) {
    lcd.setCursor(0, 0);
    lcd.print("<- INSTRUCTIONS");
    lcd.setCursor(0, 1);
    if (millis() - lastScrollTime >= 400) {
      lastScrollTime = millis();
      lcd.setCursor(0, 1);
      lcd.print(messageHTP.substring(textScrolling, 16 + textScrolling));
      textScrolling++;
    }
  }
}