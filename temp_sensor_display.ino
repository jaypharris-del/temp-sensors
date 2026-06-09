#include <OneWire.h>
#include <DallasTemperature.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

// Pin definitions
#define ONE_WIRE_PIN D12           // DS18B20 data pin
#define BUZZER_PIN D13            // Buzzer alarm pin

// Display object (mcufriend_kbv auto-detects)
MCUFRIEND_kbv tft;

// OneWire and temperature sensor setup
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1, sensor2, sensor3;

// Temperature thresholds and readings
float tempReadings[3] = {0, 0, 0};
float tempLimits[3] = {75, 75, 75};  // Default limits in Fahrenheit
bool alarmTriggered[3] = {false, false, false};

// UI state
int selectedSensor = 0;  // 0, 1, or 2
bool adjustingLimit = false;
unsigned long lastTempRead = 0;
unsigned long lastDisplay = 0;
const unsigned long TEMP_READ_INTERVAL = 1000;    // Read temp every 1 second
const unsigned long DISPLAY_UPDATE_INTERVAL = 500; // Update display every 500ms

// Color definitions
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_GREEN 0x07E0
#define COLOR_RED 0xF800
#define COLOR_YELLOW 0xFFE0
#define COLOR_CYAN 0x07FF
#define COLOR_BLUE 0x001F

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\nTemp Sensor Display Initializing...");
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize TFT display (mcufriend_kbv auto-detects)
  uint16_t ID = tft.readID();
  Serial.print("Display ID: 0x");
  Serial.println(ID, HEX);
  
  tft.begin(ID);
  tft.setRotation(1);  // Landscape mode
  tft.fillScreen(COLOR_BLACK);
  
  Serial.print("Display size: ");
  Serial.print(tft.width());
  Serial.print("x");
  Serial.println(tft.height());
  
  // Initialize temperature sensors
  sensors.begin();
  
  // Get addresses of sensors
  if (!sensors.getAddress(sensor1, 0)) {
    Serial.println("Unable to find address for Device 0");
  }
  if (!sensors.getAddress(sensor2, 1)) {
    Serial.println("Unable to find address for Device 1");
  }
  if (!sensors.getAddress(sensor3, 2)) {
    Serial.println("Unable to find address for Device 2");
  }
  
  // Set resolution to 12 bits
  sensors.setResolution(sensor1, 12);
  sensors.setResolution(sensor2, 12);
  sensors.setResolution(sensor3, 12);
  
  // Draw initial screen
  drawBootScreen();
  delay(2000);
  
  // Request first temperature read
  sensors.requestTemperatures();
  lastTempRead = millis();
  
  Serial.println("Setup complete!");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read temperatures at interval
  if (currentMillis - lastTempRead >= TEMP_READ_INTERVAL) {
    readTemperatures();
    sensors.requestTemperatures();  // Request next reading
    lastTempRead = currentMillis;
  }
  
  // Update display at interval
  if (currentMillis - lastDisplay >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplay = currentMillis;
  }
  
  // Check for alarms
  checkAlarms();
  
  // Handle serial input
  if (Serial.available()) {
    handleSerialInput();
  }
}

void readTemperatures() {
  // Convert Celsius to Fahrenheit
  float c1 = sensors.getTempC(sensor1);
  float c2 = sensors.getTempC(sensor2);
  float c3 = sensors.getTempC(sensor3);
  
  // Only update if valid reading
  if (c1 > -127 && c1 < 85) tempReadings[0] = (c1 * 9.0 / 5.0) + 32.0;
  if (c2 > -127 && c2 < 85) tempReadings[1] = (c2 * 9.0 / 5.0) + 32.0;
  if (c3 > -127 && c3 < 85) tempReadings[2] = (c3 * 9.0 / 5.0) + 32.0;
  
  Serial.print("Temp 1: ");
  Serial.print(tempReadings[0], 1);
  Serial.print("F, Temp 2: ");
  Serial.print(tempReadings[1], 1);
  Serial.print("F, Temp 3: ");
  Serial.print(tempReadings[2], 1);
  Serial.println("F");
}

void updateDisplay() {
  tft.fillScreen(COLOR_BLACK);
  
  // Draw title
  tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Temperature Monitor");
  
  // Draw horizontal line
  tft.drawLine(0, 35, tft.width(), 35, COLOR_CYAN);
  
  // Draw temperature display for each sensor
  int startY = 50;
  int sensorSpacing = 60;
  
  for (int i = 0; i < 3; i++) {
    drawSensorDisplay(i, 10, startY + (i * sensorSpacing));
  }
  
  // Draw control instructions at bottom
  drawControlPanel();
}

void drawSensorDisplay(int sensorNum, int x, int y) {
  // Sensor label
  tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print("Sensor ");
  tft.println(sensorNum + 1);
  
  // Temperature reading
  uint16_t tempColor = (tempReadings[sensorNum] > tempLimits[sensorNum]) ? COLOR_RED : COLOR_GREEN;
  tft.setTextColor(tempColor, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setCursor(x, y + 20);
  tft.print(tempReadings[sensorNum], 1);
  tft.println("F");
  
  // Limit indicator
  tft.setTextColor(COLOR_YELLOW, COLOR_BLACK);
  tft.setTextSize(1);
  tft.setCursor(x + 140, y);
  tft.print("Limit: ");
  tft.println(tempLimits[sensorNum], 1);
  
  // Bar graph
  drawBarGraph(sensorNum, x + 140, y + 15);
}

void drawBarGraph(int sensorNum, int x, int y) {
  int barWidth = 60;
  int barHeight = 20;
  float maxTemp = 120.0;
  
  // Draw bar background
  tft.drawRect(x, y, barWidth, barHeight, COLOR_WHITE);
  
  // Calculate bar fill
  float percentage = tempReadings[sensorNum] / maxTemp;
  if (percentage > 1.0) percentage = 1.0;
  if (percentage < 0) percentage = 0;
  
  int fillWidth = (int)(barWidth * percentage);
  uint16_t barColor = (tempReadings[sensorNum] > tempLimits[sensorNum]) ? COLOR_RED : COLOR_GREEN;
  
  // Draw filled portion
  if (fillWidth > 0) {
    tft.fillRect(x + 1, y + 1, fillWidth - 1, barHeight - 2, barColor);
  }
}

void drawControlPanel() {
  tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, tft.height() - 20);
  tft.println("Serial: S1/S2/S3=select, +/-=adjust limit");
}

void drawBootScreen() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_CYAN, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setCursor(40, 100);
  tft.println("Temperature");
  tft.setCursor(60, 140);
  tft.println("Monitor");
  tft.setTextSize(2);
  tft.setCursor(50, 200);
  tft.println("Initializing...");
}

void checkAlarms() {
  for (int i = 0; i < 3; i++) {
    if (tempReadings[i] > tempLimits[i]) {
      if (!alarmTriggered[i]) {
        alarmTriggered[i] = true;
        triggerAlarm();
        Serial.print("ALARM: Sensor ");
        Serial.print(i + 1);
        Serial.print(" exceeded limit! ");
        Serial.print(tempReadings[i], 1);
        Serial.print("F > ");
        Serial.print(tempLimits[i], 1);
        Serial.println("F");
      }
    } else {
      if (alarmTriggered[i]) {
        alarmTriggered[i] = false;
        digitalWrite(BUZZER_PIN, LOW);
        Serial.print("Alarm cleared for Sensor ");
        Serial.println(i + 1);
      }
    }
  }
}

void triggerAlarm() {
  // Buzzer pattern: beep beep
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

void handleSerialInput() {
  char cmd = Serial.read();
  
  if (cmd == 'S' || cmd == 's') {
    // Skip whitespace
    while (Serial.available() && Serial.peek() == ' ') Serial.read();
    
    int sensor = Serial.parseInt();
    if (sensor >= 1 && sensor <= 3) {
      selectedSensor = sensor - 1;
      adjustingLimit = true;
      Serial.print("Selected Sensor ");
      Serial.println(sensor);
      Serial.println("Use + to increase limit, - to decrease limit");
    }
  }
  else if (cmd == '+' && adjustingLimit) {
    tempLimits[selectedSensor] += 1.0;
    Serial.print("New limit for Sensor ");
    Serial.print(selectedSensor + 1);
    Serial.print(": ");
    Serial.print(tempLimits[selectedSensor], 1);
    Serial.println("F");
  }
  else if (cmd == '-' && adjustingLimit) {
    tempLimits[selectedSensor] -= 1.0;
    Serial.print("New limit for Sensor ");
    Serial.print(selectedSensor + 1);
    Serial.print(": ");
    Serial.print(tempLimits[selectedSensor], 1);
    Serial.println("F");
  }
}
