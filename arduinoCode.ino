#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo lockServo;
const int SERVO_PIN  = A1;
const int LOCK_POS   = 0;
const int UNLOCK_POS = 90;
const int GREEN_LED = A2;
const int RED_LED   = A3;
const int BUZZER_PIN = A4;
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {A0, 13, 10, 9};   
byte colPins[COLS] = {8, 7, 6};         
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char password[5] = "1234";  
char input[5];
byte idx = 0;
int wrongAttempts = 0;
const int MAX_ATTEMPTS = 3;
const unsigned long LOCKOUT_MS = 15000;
unsigned long lockoutUntil = 0;

enum State {
  STATE_ENTER_CODE,
  STATE_GRANTED_MENU,
  STATE_RESET_NEW1,
  STATE_RESET_NEW2
};
State state = STATE_ENTER_CODE;
char newPass1[5];
char newPass2[5];
byte passIdx = 0;
void setLed(int pin, bool on) { digitalWrite(pin, on ? HIGH : LOW); }
void beep(int freq, int ms) {
  tone(BUZZER_PIN, freq);
  delay(ms);
  noTone(BUZZER_PIN);
}
void successBeep() {
  beep(1200, 120); delay(60);
  beep(1600, 120);
}
void errorBeep() {
  for (int i = 0; i < 3; i++) { beep(2000, 150); delay(100); }
}
void lockSafe() {
  lockServo.write(LOCK_POS);
  delay(400);
}
void unlockSafe() {
  lockServo.write(UNLOCK_POS);
  delay(400);
}
void showPromptEnter() {
  lcd.clear();
  lcd.print("Enter Code:");
  lcd.setCursor(0, 1);
}
void showGrantedMenu() {
  lcd.clear();
  lcd.print("5:Reset Pass");
  lcd.setCursor(0, 1);
  lcd.print("#:Lock/Exit");
}
void showResetStep1() {
  lcd.clear();
  lcd.print("New Pass:");
  lcd.setCursor(0, 1);
}

void showResetStep2() {
  lcd.clear();
  lcd.print("Again:");
  lcd.setCursor(0, 1);
}

void clearEntryBuffers() {
  idx = 0;
  input[0] = '\0';
}

void clearResetBuffers() {
  passIdx = 0;
  newPass1[0] = '\0';
  newPass2[0] = '\0';
}

void lockoutMessage() {
  lcd.clear();
  lcd.print("LOCKED!");
  lcd.setCursor(0, 1);
  lcd.print("Wait 15 sec");
}


bool handle4DigitEntry(char* target, const char* stepLabel) {
  char key = keypad.getKey();
  if (!key) return false; 

  if (key == '*') {
    passIdx = 0;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    return false;
  }

  if (key == '#') {
 
    clearResetBuffers();
    state = STATE_GRANTED_MENU;
    showGrantedMenu();
    return false;
  }

  if (key >= '0' && key <= '9') {
    if (passIdx < 4) {
      target[passIdx++] = key;
      lcd.print('*');
    }
    if (passIdx == 4) {
      target[4] = '\0';
      passIdx = 0; 
      return true;
    }
  }

  return false;
}

void checkCode() {
  input[idx] = '\0';
  lcd.clear();

  if (strcmp(input, password) == 0) {
    wrongAttempts = 0;

    lcd.print("Access Granted");
    successBeep();
    setLed(GREEN_LED, true);

    unlockSafe();
    delay(1200);

    setLed(GREEN_LED, false);

    state = STATE_GRANTED_MENU;
    showGrantedMenu();
  } else {
    wrongAttempts++;

    lcd.print("Wrong Password");
    lockSafe();
    setLed(RED_LED, true);
    errorBeep();
    delay(1200);
    setLed(RED_LED, false);

    if (wrongAttempts >= MAX_ATTEMPTS) {
      lockoutUntil = millis() + LOCKOUT_MS;
      lockoutMessage();
      beep(1000, 800); delay(400);
      beep(800, 800);
    } else {
      lcd.clear();
      lcd.print("Try again");
      lcd.setCursor(0, 1);
      lcd.print("Attempts: ");
      lcd.print(wrongAttempts);
      delay(1200);
      showPromptEnter();
    }
  }

  clearEntryBuffers();
}

void handleEnterCode() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == '*') {
    clearEntryBuffers();
    showPromptEnter();
    return;
  }

  if (key == '#') {
    if (idx > 0) checkCode();
    return;
  }

  if (key >= '0' && key <= '9') {
    if (idx < 4) {
      input[idx++] = key;
      lcd.print('*');
    }
    if (idx == 4) {
      checkCode();
    }
  }
}

void handleGrantedMenu() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == '5') {
    // Start reset flow
    clearResetBuffers();
    state = STATE_RESET_NEW1;
    showResetStep1();
    return;
  }

  if (key == '#') {
    // Lock and exit
    lcd.clear();
    lcd.print("Locking...");
    lockSafe();
    delay(600);

    state = STATE_ENTER_CODE;
    showPromptEnter();
    return;
  }


  if (key == '*') {
    showGrantedMenu();
    return;
  }
}

void handleResetStep1() {
  // Enter new password first time
  if (handle4DigitEntry(newPass1, "New Pass")) {
    // Move to confirm step
    state = STATE_RESET_NEW2;
    showResetStep2();
  }
}

void handleResetStep2() {
  // Re-enter new password
  if (handle4DigitEntry(newPass2, "Again")) {
    // Compare
    lcd.clear();

    if (strcmp(newPass1, newPass2) == 0) {
      strcpy(password, newPass1);
      lcd.print("Password Saved");
      successBeep();
      delay(1200);

      state = STATE_GRANTED_MENU;
      showGrantedMenu();
    } else {
      lcd.print("Not Match!");
      errorBeep();
      delay(1200);

      // Restart reset flow
      clearResetBuffers();
      state = STATE_RESET_NEW1;
      showResetStep1();
    }
  }
}

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLed(GREEN_LED, false);
  setLed(RED_LED, false);

  lcd.begin(16, 2);
  showPromptEnter();

  lockServo.attach(SERVO_PIN);
  lockSafe();
}

void loop() {
  // lockout handling
  if (millis() < lockoutUntil) return;
  if (lockoutUntil != 0 && millis() >= lockoutUntil) {
    lockoutUntil = 0;
    wrongAttempts = 0;
    state = STATE_ENTER_CODE;
    showPromptEnter();
  }

  switch (state) {
    case STATE_ENTER_CODE:
      handleEnterCode();
      break;

    case STATE_GRANTED_MENU:
      handleGrantedMenu();
      break;

    case STATE_RESET_NEW1:
      handleResetStep1();
      break;

    case STATE_RESET_NEW2:
      handleResetStep2();
      break;
  }
}
