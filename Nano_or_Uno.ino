#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <VL53L0X.h>
#include <OneButton.h>

int adjustedDepth;              // Variable to store the calculated depth value
bool zeroPointCaptured = false; // Flag to indicate if the zero point has been captured

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C // I2C address for OLED (check via I2C scanner if needed)
#define OLED_RESET -1

#define PIN_BUTTON1 10 // Button for "+"
#define PIN_BUTTON2 11 // Button for "-"
#define PIN_BUTTON3 12 // Button for "Confirm"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED definition
VL53L0X sensor;                                                           // Laser sensor definition

OneButton button1(PIN_BUTTON1, true);
OneButton button2(PIN_BUTTON2, true);
OneButton button3(PIN_BUTTON3, true);

// Define positions and dimensions for UI rectangles
int rectWidth = 120;  // Rectangle width
int rectHeight = 16;  // Rectangle height

int rect1X = 4;       // Rectangle 1 X position
int rect1Y = 4;       // Rectangle 1 Y position

int rect2X = 4;       // Rectangle 2 X position
int rect2Y = 24;      // Rectangle 2 Y position

int rect3X = 4;       // Rectangle 3 X position
int rect3Y = 40;      // Rectangle 3 Y position

// Finite State Machine (FSM) setup
enum Mode { SET_DEPTH, SET_ZERO, SET_DEADZONE, RUN, SET_DONE };
Mode currentMode = SET_DEPTH;

int targetDepth = 50; // Target drilling depth (mm)
int zeroPoint = 0;    // Zero point reference value (mm)
int deadZone = 5;     // Deadzone/Safety buffer (mm)

String sent = "sent"; // Message string to send to the Client via Serial
String wifi = "WiFi"; // WiFi connection status string

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display();
  delay(600); // Delay for screen refresh (longer delay = longer boot logo time)
  display.clearDisplay();

  // Initialize VL53L0X sensor
  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println(F("Failed to detect and initialize sensor!"));
    while (1);
  }

  // Set measurement timing budget. Higher budget = higher accuracy but lower frequency.
  sensor.setMeasurementTimingBudget(100000);

  // Initialize buttons
  button1.attachClick(increaseValue);
  button2.attachClick(decreaseValue);
  button2.attachLongPressStart(longPress);
  button3.attachClick(confirm);
  button3.attachLongPressStart(returnButton);

  // Initial display update
  displayParameters();
}

void loop() {
  // Check button states. Avoid heavy blocking logic in loop to maintain sensitivity.
  button1.tick();
  button2.tick();
  button3.tick();
  
  // Read WiFi status from Serial if available
  if (Serial.available()) {
    wifi = Serial.readStringUntil('\n');
  }

  // Real-time depth monitoring (Only runs in RUN mode)
  if (currentMode == RUN) {
    displayCurrentDepth();
    displayParameters();

    // Trigger stop signal if target depth is reached (minus deadzone)
    if(adjustedDepth >= targetDepth - deadZone)
    {
      Serial.println(sent);
      delay(100);
    }
  }
}

// --- Button Functions ---

void increaseValue() // Function for "+" button
{
  switch (currentMode)
  {
    case SET_DEPTH:
      targetDepth += 1;
      break;
    case SET_DEADZONE:
      deadZone += 1;
      break;
    case SET_ZERO:
      zeroPoint += 1;
    default:
      break;
  }
  displayParameters();
}

void decreaseValue() // Function for "-" button
{
  switch (currentMode)
  {
    case SET_DEPTH:
      targetDepth -= 1;
      break;
    case SET_DEADZONE:
      deadZone -= 1;
      break;
    case SET_ZERO:
      zeroPoint -= 1;
    default:
      break;
  }
  displayParameters();
}

void confirm() // Function for "Confirm" button
{
  switch(currentMode)
  {
    case SET_DEPTH:
      currentMode = SET_DEADZONE; // Transition to next state
      break;
    case SET_DEADZONE:
      zeroPoint = sensor.readRangeSingleMillimeters(); // Capture zero point
      zeroPointCaptured = true; // Set flag
      if(zeroPointCaptured == true)
      {
        Serial.println("zeropoint get");
      }
      currentMode = SET_ZERO;
      break;
    case SET_ZERO:
      Serial.println(zeroPoint); // Update zero point
      currentMode = SET_DONE;
      break;
    case SET_DONE:
      currentMode = RUN;
      break;
    case RUN:
      resetProgram(); // Reset all parameters and return to setup
      break;
    default:
      break;
  }
  displayParameters(); // Update display
}

void returnButton(){
  switch(currentMode)
  {
    case RUN:
      currentMode = SET_DONE; // Go back to previous state
      break;
    case SET_DONE:
      zeroPoint = sensor.readRangeSingleMillimeters(); // Recapture zero point
      zeroPointCaptured = true;
      if(zeroPointCaptured == true)
      {
        Serial.println("zeropoint get");
      }
      currentMode = SET_ZERO;
      break;
    case SET_ZERO:
      currentMode = SET_DEADZONE;
      break;
    case SET_DEADZONE:
      currentMode = SET_DEPTH;
      break;
    default:
      break;
  }
  displayParameters(); // Update display
}

void displayParameters() 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch(currentMode)
  {
    case SET_DEPTH:
    { 
      // Draw Rectangle 1: x, y, width, height, color
      display.drawRect(rect1X, rect1Y, rectWidth, rectHeight, SSD1306_WHITE);
      display.setCursor(rect1X + 3, rect1Y + 5);
      display.print(F("Set Depth: "));
      display.print(targetDepth);
      display.println(F(" mm"));

      display.setCursor(8,30);
      display.print(F("Deadzone:  "));
      display.print(deadZone);
      display.println(F(" mm"));

      display.setCursor(8,50);
      display.print(F("ZeroPoint: "));
      display.print(zeroPoint);
      display.println(F(" mm"));
      break;
    }
    
    case SET_DEADZONE:
    {
      display.setCursor(8,8);
      display.print(F("Depth: "));
      display.print(targetDepth);
      display.println(F(" mm"));

      // Draw Rectangle 2
      display.drawRect(rect2X, rect2Y, rectWidth, rectHeight, SSD1306_WHITE);
      display.setCursor(rect2X + 3, rect2Y + 5);
      display.print(F("Set Deadzone: "));
      display.print(deadZone);
      display.println(F(" mm"));

      display.setCursor(8,50);
      display.print(F("ZeroPoint: "));
      display.print(zeroPoint);
      display.println(F(" mm"));
      break;
    }
    case SET_ZERO:
    {
      display.setCursor(8,5);
      display.print(F("Depth: "));
      display.print(targetDepth);
      display.println(F(" mm"));

      display.setCursor(8,23);
      display.print(F("Deadzone: "));
      display.print(deadZone);
      display.println(F(" mm"));

      // Draw Rectangle 3
      display.drawRect(rect3X, rect3Y, rectWidth, rectHeight, SSD1306_WHITE);
      display.setCursor(rect3X + 3, rect3Y + 5);
      display.print(F("Set Zero: "));
      display.print(zeroPoint);
      display.println(F(" mm"));
      break;
    }
    case SET_DONE: // Confirmation Screen
    {
      // Draw Rectangle 4
      display.drawRect(rect1X, rect1Y, 122, 60, SSD1306_WHITE);

      display.setCursor(37, rect1Y + 3);
      display.println(F("<Confirm>"));
      display.setCursor(8,20);
      display.print(F("Depth:     "));
      display.print(targetDepth);
      display.println(F(" mm"));

      display.setCursor(8,34);
      display.print(F("Deadzone:   "));
      display.print(deadZone);
      display.println(F(" mm"));

      display.setCursor(8,49);
      display.print(F("ZeroPoint:  "));
      display.print(zeroPoint);
      display.println(F(" mm"));
      break;
    }
    case RUN: // Run Mode: Display real-time sensor data and WiFi status
    {
      // Draw Rectangle 5
      display.drawRect(2, rect1Y, 124, 36, SSD1306_WHITE);

      display.setCursor(7,9);
      display.println(F("Current Depth: "));

      display.setTextSize(2);
      display.setCursor(9,23);
      display.print(adjustedDepth);
      display.println(F(" mm"));

      display.setTextSize(1); // Reset text size
      
      // Display WiFi connection status
      wifiConnect(); // Cannot put heavy logic here due to OLED buffer limits, calling function instead

      break;
    }
  }
  display.display(); // Render content to screen
}

void displayCurrentDepth()
{
  int currentDepth = sensor.readRangeSingleMillimeters();
  adjustedDepth = zeroPoint - currentDepth;

  // Ensure depth value is non-negative
  if (adjustedDepth < 0) {
    adjustedDepth = 0;
  }
}

void resetProgram() {
  // Reset all parameters and return to SET_DEPTH mode
  targetDepth = 50;
  zeroPoint = 0;
  deadZone = 5;
  currentMode = SET_DEPTH;
  displayParameters();
}

void longPress()
{
  // Long press functionality to improve button sensitivity during RUN mode
  switch(currentMode)
  {
    case RUN:
      resetProgram(); 
      break;
    default:
      break;
  }
  displayParameters();
}

void wifiConnect()
{
  // Draw Rectangle 6
  display.drawRect(2, 44, 124, rectHeight, SSD1306_WHITE);
  display.setCursor(9, 49);
  
  // NOTE: Do not use 'while' loops here; it will block OLED refresh.
  if(wifi == "WiFi"){
    display.println(F("Connected"));
  }
  else if(wifi == "Disc") // String length limit check
  {
    display.println(F("Wait")); // Buffer is full, avoid adding too many characters to prevent OLED blackout
  }
}