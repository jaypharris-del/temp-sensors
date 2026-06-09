# Temperature Sensor Display

A multi-sensor temperature monitoring system that displays real-time readings from 3 DS18B20 temperature sensors on a 2.8" TFT display with alarm functionality.

## Features

- **3 DS18B20 Temperature Sensors** - All connected to D12
- **2.8" TFT Display** - Auto-detected using mcufriend_kbv, real-time temperature display in Fahrenheit
- **Bar Graph Visualization** - Visual representation of each sensor's temperature
- **Adjustable Limits** - Set temperature thresholds for each sensor via serial commands
- **Alarm System** - Buzzer alarm on D13 when temperature exceeds limit
- **Serial Control** - Adjust limits via serial commands

## Hardware Requirements

- **Microcontroller**: ESP8266 (NodeMCU), Arduino, or ESP32
- **Temperature Sensors**: 3x DS18B20 1-Wire Temperature Sensors
- **Display**: 2.8" TFT Display (any controller - auto-detected)
- **Buzzer**: Active/Passive buzzer connected to D13
- **Pull-up Resistor**: 4.7kΩ resistor on D12 (OneWire data line)

## Pin Configuration

| Component | Pin |
|-----------|-----|
| DS18B20 Data | D12 |
| Buzzer | D13 |
| TFT (auto-detected) | Default SPI pins + RST, DC, CS |

## Wiring

### DS18B20 Sensors (All on D12)
- Connect all 3 sensors to D12 (data line)
- Add a 4.7kΩ pull-up resistor between D12 and 3.3V
- Connect all sensor GND pins together to GND
- Connect all sensor VCC pins together to 3.3V

### TFT Display
The mcufriend_kbv library uses default SPI pins:
- **ESP8266**: MOSI=D7, MISO=D6, SCK=D5, CS=D4, RST=D0, DC=D8
- **Arduino**: MOSI=11, MISO=12, SCK=13, plus RST, DC, CS (check your board)
- **ESP32**: MOSI=23, MISO=19, SCK=18, plus RST, DC, CS

Connect power:
- VCC → 3.3V
- GND → GND

### Buzzer
- Positive → D13
- Negative → GND

## Required Libraries

Install via Arduino IDE Library Manager:

```
- OneWire
- DallasTemperature
- MCUFRIEND_kbv
- Adafruit GFX Library
```

Or add to `platformio.ini`:
```ini
lib_deps =
    PaulStoffregen/OneWire@^2.3.6
    milesburton/DallasTemperature@^3.9.1
    prenticedba/MCUFRIEND_kbv@^3.0.3
    adafruit/Adafruit GFX Library@^1.11.9
```

## Installation

1. Install the required libraries
2. Upload the `temp_sensor_display.ino` sketch to your microcontroller
3. Open the Serial Monitor (115200 baud)
4. You should see the display ID detected and "Setup complete!" message

## Serial Commands

Control the temperature limits via serial monitor:

- `S1` - Select Sensor 1
- `S2` - Select Sensor 2
- `S3` - Select Sensor 3
- `+` - Increase temperature limit by 1°F
- `-` - Decrease temperature limit by 1°F

### Example Session

```
S1        // Select Sensor 1
+         // Increase limit to 76°F
+         // Increase limit to 77°F
-         // Decrease limit to 76°F
S2        // Switch to Sensor 2
```

## Display Layout

The TFT display shows:
- **Title**: Temperature Monitor
- **For Each Sensor**:
  - Sensor number
  - Current temperature in Fahrenheit (green if normal, red if over limit)
  - Temperature limit
  - Bar graph (visual indicator of temperature relative to max 120°F)
- **Instructions**: Serial command reference

## Alarm Behavior

When a sensor reading exceeds its set limit:
- Temperature value displays in **RED**
- Bar graph fills in **RED**
- Buzzer emits **2 short beeps**
- Status message sent to Serial Monitor
- Alarm continues until temperature drops below limit
- When cleared, confirmation message appears

Serial output shows real-time temperature readings and alarm status.

## Default Settings

- **Temperature Limits**: 75°F for all sensors
- **Temperature Range**: -50°F to 185°F (valid sensor range)
- **Update Interval**: 1 second for temperature readings
- **Display Refresh**: 500ms
- **Bar Graph Max**: 120°F

## Troubleshooting

### Display not detected
- Run the Arduino IDE example: File → Examples → MCUFRIEND_kbv → diagnose
- This will show your display ID
- Check SPI pin connections

### No temperature readings
- Check 4.7kΩ pull-up resistor on D12
- Verify sensor GND and VCC connections
- Check DS18B20 sensor addressing in serial output
- One sensor not detected: Check it's properly connected

### Buzzer not working
- Verify D13 connection
- Check for active vs passive buzzer (code uses HIGH for active)
- Test with: `digitalWrite(D13, HIGH); delay(500); digitalWrite(D13, LOW);`

### Display flickering
- Increase DISPLAY_UPDATE_INTERVAL in code
- Check power supply stability

## Serial Monitor Output

```
Temp Sensor Display Initializing...
Display ID: 0x9341
Display size: 320x240
Temp 1: 72.5F, Temp 2: 71.3F, Temp 3: 73.8F
Temp 1: 72.6F, Temp 2: 71.4F, Temp 3: 73.9F
ALARM: Sensor 1 exceeded limit! 76.2F > 75.0F
[buzzer beeps]
Alarm cleared for Sensor 1
```

## Customization

Edit these values in the code:

```cpp
float tempLimits[3] = {75, 75, 75};  // Default limits in Fahrenheit
#define TEMP_READ_INTERVAL 1000      // Temperature read frequency (ms)
#define DISPLAY_UPDATE_INTERVAL 500  // Display refresh rate (ms)
```

## Future Enhancements

- SD card data logging
- MQTT/WiFi integration
- Web dashboard
- Multiple alarm settings (low/high warnings)
- Sensor calibration menu
- EEPROM storage of limit settings
- Touch screen input

## License

Open source - Feel free to modify and use!

## Support

If you have issues:
1. Check Serial Monitor output for error messages
2. Run the MCUFRIEND_kbv diagnose sketch
3. Verify all sensor connections
4. Test each component independently
