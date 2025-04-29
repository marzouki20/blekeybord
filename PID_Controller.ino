#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Controller", "Manufacturer", 100);

// Broches
const uint8_t BUTTON_PINS[] = {15, 18, 4, 5}; // Q, Z, D, S
#define JOYSTICK_X_PIN 35
#define JOYSTICK_BTN_PIN 12  // Spacebar

// Configuration joystick
#define CENTER_X 2000        // À calibrer avec les vraies valeurs
#define DEADZONE 1500         // Sensibilité
#define SAMPLE_COUNT 5
#define POLL_RATE 20         // 50Hz

// dsssawdsawdsssTouches
const char BUTTON_KEYS[] = {'q', 'z', 'd', 's'};

// États
bool buttonStates[4] = {HIGH, HIGH, HIGH, HIGH};
bool leftPressed = false;
bool rightPressed = false;

void setup() {
  Serial.begin(115200);
  
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP);
  
  bleKeyboard.begin();
  Serial.println("En attente de connexion BLE...");
}

void loop() {
  if (!bleKeyboard.isConnected()) {
    delay(100);
    return;
  }

  handleButtons();
  handleJoystick();
}

void handleButtons() {
  for (uint8_t i = 0; i < 4; i++) {
    bool currentState = digitalRead(BUTTON_PINS[i]);
    
    if (currentState != buttonStates[i]) {
      if (currentState == LOW) {
        bleKeyboard.press(BUTTON_KEYS[i]);
      } else {
        bleKeyboard.release(BUTTON_KEYS[i]);
      }
      buttonStates[i] = currentState;
    }
  }
}

void handleJoystick() {
  static unsigned long lastRead = 0;
  if (millis() - lastRead < POLL_RATE) return;
  
  int xVal = readAverage(JOYSTICK_X_PIN, SAMPLE_COUNT);

  // Gestion gauche
  if (xVal < (CENTER_X - DEADZONE)) {
    if (!leftPressed) {
      bleKeyboard.press(KEY_RIGHT_ARROW);
      leftPressed = true;
    }
  } else if (leftPressed) {
    bleKeyboard.release(KEY_RIGHT_ARROW);
    leftPressed = false;
  }

  // Gestion droite
  if (xVal > (CENTER_X + DEADZONE)) {
    if (!rightPressed) {
      bleKeyboard.press(KEY_LEFT_ARROW);
      rightPressed = true;
    }
  } else if (rightPressed) {
    bleKeyboard.release(KEY_LEFT_ARROW);
    rightPressed = false;
  }

  lastRead = millis();
}

int readAverage(int pin, int samples) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delayMicroseconds(100);
  }
  return total / samples;
}
