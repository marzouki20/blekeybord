#include <BleKeyboard.h>

// ==== BLE Setup ====
BleKeyboard bleKeyboard("ESP32 Controller", "Manufacturer", 100);

// ==== Pin Definitions ====
const uint8_t BUTTON_PINS[] = {15, 18, 4, 5};  // Q, Z, D, S
#define JOYSTICK_X_PIN 35
#define JOYSTICK_Y_PIN 34
#define JOYSTICK_BTN_PIN 12  // Spacebar

// ==== Joystick Configuration ====
#define ANALOG_MAX 4095      // ESP32 ADC resolution
#define CENTER_X 2000        // Calibrate these values
#define CENTER_Y 2000
#define DEADZONE 300
#define SAMPLE_COUNT 5
#define POLL_RATE 20         // 50Hz (20ms)

// ==== Key Mappings ===-
const char BUTTON_KEYS[] = {'q', 'z', 'd', 's'};
const uint8_t ARROW_KEYS[] = {
  KEY_UP_ARROW,
  KEY_DOWN_ARROW,
  KEY_LEFT_ARROW,
  KEY_RIGHT_ARROW
};

// ==== State Tracking ====
bool buttonStates[4] = {HIGH, HIGH, HIGH, HIGH};
bool lastJoyButtonState = HIGH;
bool currentArrows[4] = {false};  // UP, DOWN, LEFT, RIGHT
unsigned long lastJoystickRead = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize inputs
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP);

  // Start BLE
  bleKeyboard.begin();
  Serial.println("Waiting for BLE connection...");
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
  if (millis() - lastJoystickRead < POLL_RATE) return;
  
  // Read averaged values
  int xVal = readAverage(JOYSTICK_X_PIN, SAMPLE_COUNT);
  int yVal = readAverage(JOYSTICK_Y_PIN, SAMPLE_COUNT);
  
  bool newArrows[4] = {false};
  
  // Horizontal axis
  if (xVal < CENTER_X - DEADZONE) {
    newArrows[3] = true;  // LEFT
  } else if (xVal > CENTER_X + DEADZONE) {
    newArrows[2] = true;  // RIGHT
  }
  
  // Vertical axis (invert if needed)
  if (yVal < CENTER_Y - DEADZONE) {
    newArrows[0] = true;  // UP
  } else if (yVal > CENTER_Y + DEADZONE) {
    newArrows[1] = true;  // DOWN
  }

  // Update arrow keys
  for (uint8_t i = 0; i < 4; i++) {
    if (newArrows[i] != currentArrows[i]) {
      if (newArrows[i]) {
        bleKeyboard.press(ARROW_KEYS[i]);
      } else {
        bleKeyboard.release(ARROW_KEYS[i]);
      }
      currentArrows[i] = newArrows[i];
    }
  }

  lastJoystickRead = millis();
}

int readAverage(int pin, int samples) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delayMicroseconds(100);
  }
  return total / samples;
}
