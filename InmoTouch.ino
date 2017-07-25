#include <EEPROM.h>
#include "pitches.h"

#define STARTER_PIN 2
#define ENGINE_PIN 3
#define TOUCH_PIN 4
#define ACC_PIN 5
#define RED_LED_PIN 6
#define GREEN_LED_PIN 7
#define SPEAKER_PIN 13

#define ACC_OFF 1
#define ACC_ON 0

#define VALID 1
#define INVALID 2
#define NO_CARD 3
#define CLONE 3
#define CONTACT 4

const int TRACE = 0;
const int DEBUG = 1;
const int ERROR_ = 2;
const int WARN = 3;
const int INFO = 4;
const int LOG_LEVEL = DEBUG;

const int WORKSHOP_MEM = 0;
const int WORKSHOP_ON = 1;
const int WORKSHOP_OFF = 0;

const int INIT = 0;
const int LOCKED = 1;
const int WORKSHOP = 2;
const int UNLOCKED = 3;
const int UNLOCKED_WAITING = 4;

int MAIN_STATUS = INIT;
int TIMER = 0;
unsigned long IDLE_TIME = 60000;//milliseconds

unsigned long TIME_TOUCHS = 500;//milliseconds
const int START_TOUCHS = 3;
const int STOP_TOUCHS = 6;
const int START_TOUCHS_WORKSHOP_TOGGLE = 10;
const int CHEATS = 15;

bool RED_STATUS = false;
bool GREEN_STATUS = false;
bool NO_TOUCH = true;
bool TOUCH = false;

int OLD_STATUS = -1;
int OLD_TOUCH_STATUS = NO_TOUCH;

//========== Melodies definition: access, welcome and rejection
int access_melody[] = {NOTE_FS7, NOTE_DS7};
int access_noteDurations[] = {2, 2};

int fail_melody[] = {NOTE_DS7, NOTE_DS7, NOTE_DS7};
int fail_noteDurations[] = {8, 8, 8};

int clone_melody[] = {NOTE_B6, NOTE_DS7, NOTE_FS7};
int clone_noteDurations[] = {8, 8, 8};

int contact_melody[] = {NOTE_DS7, NOTE_FS7};
int contact_noteDurations[] = {8, 8};

//notes in the melody:
const int tetris_melody[] = {
  NOTE_E5, NOTE_E3, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5,
  NOTE_B4, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A3, NOTE_D5,
  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_D5, NOTE_E3, NOTE_E5,
  NOTE_E3, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_B2,
  NOTE_C3, NOTE_D3, NOTE_D5, NOTE_F5, NOTE_A5, NOTE_C5, NOTE_C5, NOTE_G5,
  NOTE_F5, NOTE_E5, NOTE_C3, 0, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_G4, NOTE_D5,
  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_G4, NOTE_E5,
  NOTE_G4, NOTE_C5, NOTE_E4, NOTE_A4, NOTE_E3, NOTE_A4, 0,
  NOTE_E5, NOTE_E3, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5,
  NOTE_B4, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_A3, NOTE_D5,
  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_D5, NOTE_E3, NOTE_E5,
  NOTE_E3, NOTE_C5, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_A4, NOTE_A3, NOTE_B2,
  NOTE_C3, NOTE_D3, NOTE_D5, NOTE_F5, NOTE_A5, NOTE_C5, NOTE_C5, NOTE_G5,
  NOTE_F5, NOTE_E5, NOTE_C3, 0, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_G4, NOTE_D5,
  NOTE_C5, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_G4, NOTE_E5,
  NOTE_G4, NOTE_C5, NOTE_E4, NOTE_A4, NOTE_E3, NOTE_A4, 0,
};
//note durations: 4 = quarter note, 8 = eighth note, etc
const int tetris_noteDurations[] = {
  8, 8, 8, 8, 8, 16, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 4, 8, 8, 16, 16, 8, 8, 8, 8, 8, 8, 8, 16, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4,
  8, 8, 8, 8, 8, 16, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 4, 8, 8, 16, 16, 8, 8, 8, 8, 8, 8, 8, 16, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4,
};

//========== Function to play the access granted or denied tunes ==========
void playTune(int Scan) {
  pinMode(SPEAKER_PIN, OUTPUT);
  if (Scan == 1) // A Good card Read
  {
    for (int i = 0; i < 2; i++)    //loop through the notes
    {
      int access_noteDuration = 1000 / access_noteDurations[i];
      tone(SPEAKER_PIN, access_melody[i], access_noteDuration);
      int access_pauseBetweenNotes = access_noteDuration * 1.30;
      delay(access_pauseBetweenNotes);
      noTone(SPEAKER_PIN);
    }
  }
  else if (Scan == 2) { // A Bad card read
    for (int i = 0; i < 3; i++)    //loop through the notes
    {
      int fail_noteDuration = 1000 / fail_noteDurations[i];
      tone(SPEAKER_PIN, fail_melody[i], fail_noteDuration);
      int fail_pauseBetweenNotes = fail_noteDuration * 1.30;
      delay(fail_pauseBetweenNotes);
      noTone(SPEAKER_PIN);
    }
  }
  else if (Scan == 3) {
    for (int i = 0; i < 3; i++)    //loop through the notes
    {
      int clone_noteDuration = 1000 / clone_noteDurations[i];
      tone(SPEAKER_PIN, clone_melody[i], clone_noteDuration);
      int clone_pauseBetweenNotes = clone_noteDuration * 1.30;
      delay(clone_pauseBetweenNotes);
      noTone(SPEAKER_PIN);
    }
  }
  else if (Scan == 4) {
    for (int i = 0; i < 2; i++)    //loop through the notes
    {
      int contact_noteDuration = 1000 / contact_noteDurations[i];
      tone(SPEAKER_PIN, contact_melody[i], contact_noteDuration);
      int contact_pauseBetweenNotes = contact_noteDuration * 1.30;
      delay(contact_pauseBetweenNotes);
      noTone(SPEAKER_PIN);
    }
  } else if (Scan == CHEATS) { //Tetris Bitch!
    Serial.println("Cheat time!");
    for (int thisNote = 0; thisNote < 128; thisNote++) {
      /*
        to calculate the note duration, take one second divided by the note type.
        e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      */
      int tetris_noteDuration = 1000 / tetris_noteDurations[thisNote];
      tone(SPEAKER_PIN, tetris_melody[thisNote], tetris_noteDuration);
      /*
        to distinguish the notes, set a minimum time between them.
        the note's duration + 30% seems to work well:
      */
      int pauseBetweenNotes = tetris_noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(SPEAKER_PIN); //stop the tone playing:
    }
  }
  delay(50);
  pinMode(SPEAKER_PIN, INPUT);
}

//========== Security Car API ==========
void blinkRed() {
  RED_STATUS = !RED_STATUS;
  digitalWrite(RED_LED_PIN, RED_STATUS);
}

void blinkGreen() {
  GREEN_STATUS = !GREEN_STATUS;
  digitalWrite(GREEN_LED_PIN, GREEN_STATUS);

}

void onOrange() {
  digitalWrite(RED_LED_PIN, true);
  digitalWrite(GREEN_LED_PIN, true);
}

void onRed() {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
}

void onGreen() {
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
}

void offRed() {
  digitalWrite(RED_LED_PIN, LOW);
}

void offGreen() {
  digitalWrite(GREEN_LED_PIN, LOW);
}

void soundOK() {
  playTune(VALID);
}

void soundKO() {
  playTune(INVALID);
}

void soundCloned() {
  playTune(CLONE);
}

void soundOK_and_Contact() {
  playTune(CONTACT);
}

void soundCheat() {
  playTune(CHEATS);
}

void startEngine() {
  Serial.println("Starting Engine is allowed");
  digitalWrite(ENGINE_PIN, LOW);
  digitalWrite(STARTER_PIN, LOW);
}

void stopEngine() {
  Serial.println("Stopping Engine");
  digitalWrite(ENGINE_PIN, HIGH);
  digitalWrite(STARTER_PIN, HIGH);
}

int readTouchSensor(unsigned ms_between_touchs) {
  bool touch_status = NO_TOUCH;
  unsigned long curr_timer = 0;
  int touchs = 0;
  curr_timer = millis() + ms_between_touchs;
  while (curr_timer > millis()) {
    touch_status = digitalRead(TOUCH_PIN);
    //Flanco ascendente
    if ((touch_status != OLD_TOUCH_STATUS) &&
        (touch_status == TOUCH)) {
      Serial.println("Touch detected in time!");
      touchs++;
      curr_timer = millis() + ms_between_touchs;
    }
    OLD_TOUCH_STATUS = touch_status;
  }
  if (touchs > 1) {
    Serial.print("Touchs detected: ");
    Serial.println(touchs);
  }
  return touchs;
}

//========== Events ==========
void on_init() {
  stopEngine();
  onOrange();
  if (OLD_STATUS != INIT) {
    OLD_STATUS = MAIN_STATUS;
    Serial.println("New Status is INIT");
  }

  if (EEPROM.read(WORKSHOP_MEM) == WORKSHOP_ON) {
    MAIN_STATUS = WORKSHOP;
    Serial.println("EEPROM contains WORKSHOP mode");
  } else {
    MAIN_STATUS = LOCKED;
    Serial.println("EEPROM NO WORKSHOP mode");
  }
}

void on_locked() {
  if (OLD_STATUS != LOCKED) {
    OLD_STATUS = MAIN_STATUS;
    onRed();
    stopEngine();
    Serial.println("New Status is LOCKED");
  }

  if (START_TOUCHS == readTouchSensor(TIME_TOUCHS)) {
    MAIN_STATUS = UNLOCKED_WAITING;
  }

}

void on_unlocked() {
  if (OLD_STATUS != UNLOCKED) {
    OLD_STATUS = MAIN_STATUS;
    onGreen();
    startEngine();
    EEPROM.update(WORKSHOP_MEM, WORKSHOP_OFF);
    Serial.println("New Status is UNLOCKED");
    soundOK_and_Contact();
  }
  bool acc = ACC_ON;
  while (acc == ACC_ON) {
    acc = digitalRead(ACC_PIN);
    int touchs_read = readTouchSensor(TIME_TOUCHS);
    if (START_TOUCHS_WORKSHOP_TOGGLE == touchs_read) {
      MAIN_STATUS = WORKSHOP;
      return;
    } else if (touchs_read == CHEATS) {
      soundCheat();
    } else if (touchs_read == STOP_TOUCHS) {
      MAIN_STATUS = LOCKED;
      return;
    }
  }
  MAIN_STATUS = UNLOCKED_WAITING;
}

void on_unlocked_waiting() {
  if (OLD_STATUS != UNLOCKED_WAITING) {
    OLD_STATUS = MAIN_STATUS;
    onGreen();
    startEngine();
    Serial.println("New Status is UNLOCKED_WAITING");

    unsigned long timer = millis() + IDLE_TIME;
    bool acc = digitalRead(ACC_PIN);
    while ((acc = digitalRead(ACC_PIN))== ACC_OFF) {
      if (timer <= millis()) {
        Serial.println("Max IDLE time! Locking engine");
        MAIN_STATUS = LOCKED;
        soundKO();
        return;
      }
      int touchs_read = readTouchSensor(TIME_TOUCHS);
      if (START_TOUCHS_WORKSHOP_TOGGLE == touchs_read) {
        MAIN_STATUS = WORKSHOP;
        return;
      } else if (touchs_read == CHEATS) {
        soundCheat();
      } else if (touchs_read == STOP_TOUCHS) {
        MAIN_STATUS = LOCKED;
        return;
      }
      blinkRed();
      delay(250);
    }
    MAIN_STATUS = UNLOCKED;
  }
}

void on_workshop() {
  if (OLD_STATUS != WORKSHOP) {
    OLD_STATUS = MAIN_STATUS;
    Serial.println("New Status is WORKSHOP");
    EEPROM.update(WORKSHOP_MEM, WORKSHOP_ON);
    startEngine();
    soundCloned();
  }
  onOrange();

  int touchs_read = readTouchSensor(TIME_TOUCHS);
  if (START_TOUCHS_WORKSHOP_TOGGLE == touchs_read) {
    MAIN_STATUS = UNLOCKED;
    return;
  } else if (touchs_read == CHEATS) {
    soundCheat();
  }
}

//========== MAIN PROGRAM ==========
void setup() {
  Serial.begin(9600); // Iniciar serial
  pinMode(STARTER_PIN, OUTPUT);
  pinMode(ENGINE_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
}

void loop() {

  if (MAIN_STATUS == INIT) {
    on_init();
  }


  else if (MAIN_STATUS == LOCKED) {
    on_locked();
  }

  else if (MAIN_STATUS == WORKSHOP) {
    on_workshop();
  }

  else if (MAIN_STATUS == UNLOCKED) {
    on_unlocked();
  }

  else if (MAIN_STATUS == UNLOCKED_WAITING) {
    on_unlocked_waiting();
  }
}

