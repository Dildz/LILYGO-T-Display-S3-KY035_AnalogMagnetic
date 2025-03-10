/*********************************************************************************************************
 * LILYGO T-Display-S3 KY035 Analogue Hall Magnetic Sensor Project
 *
 * Description:
 *   This code reads the analog output of a KY035 Analogue Hall Magnetic Sensor Module connected to a
 *   LilyGO T-Display-S3 microcontroller board. It displays the sensor readings using a analog meter
 *   using the TFT_eSPI and TFT_eWidget libraries. The project demonstrates embedded programming concepts,
 *   including:
 *     - Analog Inputs: Reading sensor data from an analog pin.
 *     - Variables: Storing and manipulating data, such as the sensor reading and averaged values.
 *     - Functions: Placing code into reusable blocks for better organization and modularity.
 *     - TFT_eSPI and TFT_eWidget Libraries: Utilizing these libraries for direct display control and
 *        rendering an analog meter based off of the Meters library example.
 *     - Data Averaging: Taking multiple readings and averaging them to help smooth the reading.
 *     - Real-Time Meter Display: Visualizing sensor data as a needle on an analog meter.
 *
 * How It Works:
 *   1. Sensor Reading: The code reads the analog output of the KY035 sensor, which varies with the
 *       strength of the magnetic field.
 *   2. Averaging: Multiple readings are taken and averaged to provide a more stable value.
 *   3. Display Update: The averaged sensor reading is displayed as a needle on an analog meter, with the
 *       raw ADC value also shown on the screen.
 *   4. TFT_eWidget Meter: The project utilizes the TFT_eWidget library to render an analog meter with
 *       zones for visual feedback.
 *
 * Pin Connections:
 *   - Sensor (S)   -> A0 (Analog Input 0)
 *   - Ground (-)   -> GND
 *   - VCC (middle) -> 3.3V
 *
 * Notes:
 *   - The KY035 sensor outputs an analog signal proportional to the magnetic field strength.
 *   - The TFT_eSPI and TFT_eWidget libraries are configured to work with the LilyGO T-Display-S3,
 *      providing an easy way to display information on the built-in screen.
 *   - The analog meter dynamically updates based on the sensor readings, providing real-time feedback.
 * 
 * KY035 Specifications:
 *   - Operating Voltage: 2.7V to 6V
 *   - Power Consumption: ~ 6mA
 *   - Sensitivity: 1.4 to 2.0mV/GS
 *   - Operating Temperature: -40 °C to 85 °C [-40 °F to 185 °F]
 *********************************************************************************************************/


/*************************************************************
******************* INCLUDES & DEFINITIONS *******************
**************************************************************/

#include <TFT_eSPI.h>
#include <TFT_eWidget.h>

// Define the pin for the KY035 sensor
#define SENSOR_PIN A0 // analog pin 0 (GPIO01)

// TFT_eSPI & TFT_eWidget related declarations
TFT_eSPI tft = TFT_eSPI();
MeterWidget volts = MeterWidget(&tft);

// Display update time in ms
#define LOOP_PERIOD 50

// Screen dimensions
#define SCREEN_WIDTH 340
#define SCREEN_HEIGHT 170

// Meter dimensions (based on the size of the meter outline in Meter.cpp)
#define METER_WIDTH 239
#define METER_HEIGHT 126

// Meter positioning
#define METER_X ((SCREEN_WIDTH - METER_WIDTH) / 2 - 10) // center horizontally (needed 10 extra pixels to the left)
#define METER_Y ((SCREEN_HEIGHT - METER_HEIGHT) - 10)   // align to the bottom (with extra 10px padding)

// Float for the mapped value used by the needle
float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax) {
  return tomin + (((tomax - tomin) * (ip - ipmin)) / (ipmax - ipmin));
}


/*************************************************************
*********************** HELPER FUNCTIONS *********************
**************************************************************/

// Function to read and map the sensor value with smoothing
float readAndMapSensor(int &aveValue) { // Pass aveValue by reference
  const int numReadings = 50; // number of readings to average
  int total = 0;              // initialise a total value

  // Take multiple readings and sum them
  for (int i = 0; i < numReadings; i++) {
    total += analogRead(SENSOR_PIN);
    delay(1); // small delay between readings
  }

  // Calculate the average ave value
  aveValue = total / numReadings;

  // Normalize aveValue for when no voltage is on A0 (prevents the needle bouncing)
  if (aveValue <= 30) {
    aveValue = 0;
  }

  // Map to voltage range (0V to 3.3V)
  return mapValue(aveValue, 0, 4095, 0.0, 3.3);
}

// Function to update the meter's needle
void updateMeter(float voltage) {
  volts.updateNeedle(voltage, 0); // update the needle position
}

// Function to display the ave sensor value
void displayaveValue(int aveValue) {
  //static int textOffsetX = -3; // horizontal offset for the "ADC value" text

  // Convert the ave sensor value to a string
  char aveValueStr[8];                  // buffer to hold the string version of the ave value
  dtostrf(aveValue, 5, 0, aveValueStr); // convert aveValue to a string with 0 decimal places

  // Display the ave sensor value in the bottom left corner
  tft.setTextColor(TFT_BLACK, TFT_WHITE);                                // set text color (black on white background)
  tft.drawRightString(aveValueStr, METER_X + 50, METER_Y + 119 - 20, 2); // adjust position as needed

  // Overwrite the bottom right text with "ADC value" (originally displayed the unit 'V')
  tft.setTextColor(TFT_BLACK, TFT_WHITE);                                                   // set text color (black on white background)
  tft.drawString("ADC value", METER_X + 5 + 230 - 40, METER_Y + 119 - 20, 2); // adjust position with textOffsetX
}


/*************************************************************
*********************** MAIN FUNCTIONS ***********************
**************************************************************/

// SETUP
void setup(void) {
  tft.init();
  tft.setRotation(1); // adjust rotation (0 & 2 portrait | 1 & 3 landscape)

  // Set up meter zones
  volts.setZones(0, 100, 25, 75, 0, 0, 40, 60); // Red, Orange, Yellow, Green

  // Draw the meter
  tft.fillScreen(TFT_BLACK); // clear the screen
  volts.analogMeter(METER_X, METER_Y, 3.3, "Volts", "0V", "0.82", "1.65", "2.47", "3.3");

  // Add the heading at the top center of the screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // set text color (white on black background)
  tft.setTextSize(1);                     // set text size
  tft.setTextDatum(TC_DATUM);             // set text alignment to top center

  // Draw the heading
  tft.drawString("KY035 Analog Hall Magnetic Sensor Module", SCREEN_WIDTH / 2 - 10, 5, 2); // needed 10 extra pixels to the left
}

// MAIN LOOP
void loop() {
  static uint32_t updateTime = 0;

  if (millis() - updateTime >= LOOP_PERIOD) {
    updateTime = millis();

    // Initialise the averaged sensor value
    int aveValue;

    // Read and map the sensor value, and get the aveValue
    float voltage = readAndMapSensor(aveValue);

    // Update the analog meter's needle
    updateMeter(voltage);

    // Display the ave sensor value
    displayaveValue(aveValue); // pass aveValue to the displayaveValue function
  }
}