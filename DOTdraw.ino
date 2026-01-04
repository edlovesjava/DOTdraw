// DOTdraw - Simple drawing app for DOTplatform
// Controls:
//   LEFT button:  Rotate direction (up → right → down → left)
//   RIGHT button: Move cursor one step
//   CHORD:        Toggle draw mode
//   CHORD hold:   Clear screen (2 seconds)

// MAX7219 Register Addresses
#define MAX7219_REG_DECODE_MODE 0x09
#define MAX7219_REG_INTENSITY 0x0A
#define MAX7219_REG_SCAN_LIMIT 0x0B
#define MAX7219_REG_SHUTDOWN 0x0C
#define MAX7219_REG_DISPLAY_TEST 0x0F

// Pin definitions
const uint8_t DIN = 0;
const uint8_t CLK = 1;
const uint8_t CS = 2;
const uint8_t RIGHT_BUTTON = 3;
const uint8_t LEFT_BUTTON = 4;

// Direction constants
#define DIR_UP    0
#define DIR_RIGHT 1
#define DIR_DOWN  2
#define DIR_LEFT  3

// Canvas (what's actually drawn)
uint8_t canvas[8];

// Cursor state
int8_t cursorX = 3;
int8_t cursorY = 3;
uint8_t direction = DIR_UP;  // 0=up, 1=right, 2=down, 3=left
bool drawMode = false;

// Button state tracking
bool lastLeftPressed = false;
bool lastRightPressed = false;
bool chordActive = false;
bool chordedThisPress = false;  // True if chord was activated during this button hold
uint16_t chordHoldTime = 0;
const uint16_t CLEAR_HOLD_TIME = 2000;  // 2 seconds to clear

// Timing
uint32_t lastBlinkTime = 0;
bool cursorVisible = true;
const uint16_t CURSOR_BLINK_SHORT = 100;  // Short phase duration
const uint16_t CURSOR_BLINK_LONG = 300;   // Long phase duration

// Direction indicator
bool showDirectionBlip = false;
uint32_t blipStartTime = 0;
const uint16_t BLIP_DURATION = 150;

// Send a byte via bit-banged SPI
void sendByte(uint8_t b) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(CLK, LOW);
    digitalWrite(DIN, (b >> i) & 1);
    digitalWrite(CLK, HIGH);
  }
}

// Send command to MAX7219
void sendCmd(uint8_t addr, uint8_t data) {
  digitalWrite(CS, LOW);
  sendByte(addr);
  sendByte(data);
  digitalWrite(CS, HIGH);
}

// Update display from canvas + cursor overlay
void updateDisplay() {
  uint32_t now = millis();

  // Handle cursor blink - context-aware timing
  // Unlit pixel: long OFF, short ON (flash to show cursor)
  // Lit pixel: long ON, short OFF (dim to show cursor)
  bool pixelLit = canvas[cursorY] & (1 << cursorX);
  uint16_t blinkDuration = cursorVisible ?
    (pixelLit ? CURSOR_BLINK_LONG : CURSOR_BLINK_SHORT) :
    (pixelLit ? CURSOR_BLINK_SHORT : CURSOR_BLINK_LONG);

  if (now - lastBlinkTime >= blinkDuration) {
    lastBlinkTime = now;
    cursorVisible = !cursorVisible;
  }

  // Handle direction blip timeout
  if (showDirectionBlip && (now - blipStartTime >= BLIP_DURATION)) {
    showDirectionBlip = false;
  }

  // Build display buffer
  for (uint8_t row = 0; row < 8; row++) {
    uint8_t rowData = canvas[row];

    // Add cursor (blink or solid based on draw mode)
    if (row == cursorY) {
      if (drawMode) {
        // In draw mode: always show cursor
        rowData |= (1 << cursorX);
      } else {
        // Not drawing: context-aware cursor blink
        bool cursorPixelLit = canvas[cursorY] & (1 << cursorX);
        if (cursorPixelLit) {
          // Lit pixel: mostly ON, briefly OFF to show cursor
          if (!cursorVisible) {
            rowData &= ~(1 << cursorX);  // Turn OFF to show cursor
          }
        } else {
          // Unlit pixel: mostly OFF, briefly ON to show cursor
          if (cursorVisible) {
            rowData |= (1 << cursorX);  // Turn ON to show cursor
          }
        }
      }
    }

    // Add direction blip - toggle pixel to be visible on both lit and unlit
    if (showDirectionBlip) {
      int8_t blipX = cursorX;
      int8_t blipY = cursorY;
      switch (direction) {
        case DIR_UP:    blipY = (cursorY - 1 + 8) % 8; break;
        case DIR_DOWN:  blipY = (cursorY + 1) % 8; break;
        case DIR_LEFT:  blipX = (cursorX - 1 + 8) % 8; break;
        case DIR_RIGHT: blipX = (cursorX + 1) % 8; break;
      }
      if (row == blipY) {
        rowData ^= (1 << blipX);  // XOR to toggle - dims lit, lights unlit
      }
    }

    sendCmd(row + 1, rowData);
  }
}

// Move cursor in current direction
void moveCursor() {
  // If in draw mode, toggle pixel at current position before moving
  if (drawMode) {
    canvas[cursorY] ^= (1 << cursorX);  // Toggle pixel
  }

  // Move in current direction with wrap
  switch (direction) {
    case DIR_UP:
      cursorY = (cursorY - 1 + 8) % 8;
      break;
    case DIR_DOWN:
      cursorY = (cursorY + 1) % 8;
      break;
    case DIR_LEFT:
      cursorX = (cursorX - 1 + 8) % 8;
      break;
    case DIR_RIGHT:
      cursorX = (cursorX + 1) % 8;
      break;
  }
}

// Rotate direction clockwise
void rotateDirection() {
  direction = (direction + 1) % 4;

  // Show blip in new direction
  showDirectionBlip = true;
  blipStartTime = millis();
}

// Clear the canvas
void clearCanvas() {
  for (uint8_t i = 0; i < 8; i++) {
    canvas[i] = 0;
  }

  // Flash to confirm clear
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t row = 0; row < 8; row++) {
      sendCmd(row + 1, 0xFF);
    }
    delay(100);
    for (uint8_t row = 0; row < 8; row++) {
      sendCmd(row + 1, 0x00);
    }
    delay(100);
  }
}

void setup() {
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);

  // Initialize MAX7219
  sendCmd(MAX7219_REG_DISPLAY_TEST, 0x00);
  sendCmd(MAX7219_REG_DECODE_MODE, 0x00);
  sendCmd(MAX7219_REG_SCAN_LIMIT, 0x07);
  sendCmd(MAX7219_REG_INTENSITY, 0x08);
  sendCmd(MAX7219_REG_SHUTDOWN, 0x01);

  // Clear canvas
  for (uint8_t i = 0; i < 8; i++) {
    canvas[i] = 0;
  }
}

void loop() {
  // Read buttons
  bool leftPressed = (digitalRead(LEFT_BUTTON) == LOW);
  bool rightPressed = (digitalRead(RIGHT_BUTTON) == LOW);
  bool chordPressed = leftPressed && rightPressed;

  // Handle chord (both buttons)
  if (chordPressed) {
    if (!chordActive) {
      // Chord just started
      chordActive = true;
      chordedThisPress = true;  // Mark that we chorded during this press
      chordHoldTime = 0;
    } else {
      // Chord held - count time
      chordHoldTime++;
      if (chordHoldTime >= CLEAR_HOLD_TIME) {
        clearCanvas();
        chordHoldTime = 0;  // Reset to prevent repeated clears
      }
    }
  } else {
    // Check for chord release (toggle draw mode)
    if (chordActive && chordHoldTime < CLEAR_HOLD_TIME) {
      drawMode = !drawMode;
    }
    chordActive = false;
    chordHoldTime = 0;
  }

  // Handle single button releases (only if we didn't chord during this press)
  if (!chordPressed && !chordedThisPress) {
    // LEFT released - rotate
    if (lastLeftPressed && !leftPressed && !rightPressed) {
      rotateDirection();
    }
    // RIGHT released - move
    if (lastRightPressed && !rightPressed && !leftPressed) {
      moveCursor();
    }
  }

  // Reset chord flag when all buttons released
  if (!leftPressed && !rightPressed) {
    chordedThisPress = false;
  }

  // Save button states
  lastLeftPressed = leftPressed;
  lastRightPressed = rightPressed;

  // Update display
  updateDisplay();

  delay(1);
}
