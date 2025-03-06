/*********************************************************************************************************
 * LILYGO T-Display-S3 KY-035 Analogue Hall Magnetic Sensor Project
 *
 * Description:
 *   This code reads the analog output of a KY-035 Analogue Hall Magnetic Sensor Module connected to a
 *   LilyGO T-Display-S3 microcontroller board. It displays the sensor readings and a scrolling graph
 *   of the readings over time on the built-in screen using the TFT_eSPI library.
 *   The project demonstrates embedded programming concepts, including:
 *     - Analog Inputs: Reading sensor data from an analog pin.
 *     - Variables: Storing and manipulating data, such as the sensor reading and averaged values.
 *     - Enumerations (Enums): Defining named constants for operational modes, improving code readability.
 *     - Functions: Placing code into reusable blocks for better organization and modularity.
 *     - State Machines: Implementing a simple state machine to manage program behavior.
 *     - TFT_eSPI Library: Utilizing the TFT_eSPI graphics library for direct display control.
 *     - Data Averaging: Taking multiple readings and averaging them to reduce noise.
 *     - Real-Time Graphing: Displaying a scrolling graph of sensor readings over time.
 *
 * How It Works:
 *   1. Sensor Reading: The code reads the analog output of the KY-035 sensor, which varies with the
 *       strength of the magnetic field.
 *   2. Averaging: Multiple readings are taken and averaged to provide a more stable value.
 *   3. Display Update: The averaged sensor reading and a scrolling graph of recent readings are displayed
 *       on the TFT screen. The graph updates dynamically as new data is collected.
 *   4. TFT_eSPI Display: The project utilizes the TFT_eSPI library for displaying the sensor reading,
 *       graph, and current state directly on the screen.
 *
 * Core Concepts and Benefits:
 *   - Enums: Enhance code readability by using descriptive names for states.
 *   - State Machines: Simplify the management of program behavior.
 *   - Data Averaging: Reduces noise and provides more stable sensor readings.
 *   - Real-Time Graphing: Visualizes sensor data trends over time.
 *   - TFT_eSPI: Allows for direct control of the display, enabling real-time feedback and state
 *      information without the need for a serial monitor.
 *
 * Pin Connections:
 *   - KY-035 Sensor  -> A0 (Analog Input 0)
 *   - LCD Backlight  -> GPIO15
 *   - Ground         -> GND
 *
 * Notes:
 *   - The KY-035 sensor outputs an analog signal proportional to the magnetic field strength.
 *   - The TFT_eSPI library is configured to work with the LilyGO T-Display-S3, providing an easy way to
 *      display information on the built-in screen.
 *   - The code uses a state machine to manage the program's behavior, ensuring clean and maintainable logic.
 *   - The graph dynamically scales based on the minimum and maximum values of the sensor readings.
 * 
 * KY-035 Specifications:
 *   - Operating Voltage: 2.7V to 6V
 *   - Power Consumption: ~ 6mA
 *   - Sensitivity: 1.4 to 2.0mV/GS
 *   - Operating Temperature: -40 °C to 85 °C [-40 °F to 185 °F]
 *********************************************************************************************************/


/*************************************************************
******************* INCLUDES & DEFINITIONS *******************
**************************************************************/

#include <Arduino.h>
#include <TFT_eSPI.h>

// Define the state machine mode states
enum class State {
  SENSOR_READ, // read the sensor value
  WAIT,        // wait for the next reading
  AVERAGE      // take multiple readings and average them
};

// Global variables
State currentState = State::SENSOR_READ; // default to Sensor Mode
unsigned long lastReadTime = 0;          // timestamp of the last sensor reading
const unsigned long READ_INTERVAL = 50;  // interval between individual readings (in milliseconds)
const int NUM_READINGS = 10;             // number of readings to average
int readings[NUM_READINGS];              // array to store readings
int graphData[NUM_READINGS];             // array for graph data
int readIndex = 0;                       // index for storing readings
int averagedValue = 0;                   // averaged sensor value
int previousAveragedValue = -1;          // store the previous averaged value for comparison

// Graph scaling
const int GRAPH_X = 5;      // X start position of the graph
const int GRAPH_WIDTH = 150; // Width of graph
const int GRAPH_HEIGHT = 55; // Height of graph
int graphY;                  // Y start position of the graph (calculated dynamically)

// Define the pin for the KY-035 sensor
#define SENSOR_PIN A0 // analog pin for KY-035 sensor
#define PIN_LCD_BL 15 // backlight pin for T-Display S3

// TFT_eSPI related declarations
TFT_eSPI tft = TFT_eSPI();
bool redrawRequired = true; // flag to indicate if the screen needs to be updated

/*************************************************************
*********************** HELPER FUNCTION **********************
**************************************************************/

// Function to draw a simple scrolling graph
void drawGraph() {
  int minValue = 30, maxValue = 4095; // no voltage on A0 = ~30 | 3.3V on A0 = 4095

  // Determine the min and max values for scaling
  for (int i = 0; i < NUM_READINGS; i++) {
    if (graphData[i] < minValue) minValue = graphData[i];
    if (graphData[i] > maxValue) maxValue = graphData[i];
  }

  // Clear graph area
  tft.fillRect(GRAPH_X, graphY - GRAPH_HEIGHT, GRAPH_WIDTH, GRAPH_HEIGHT, TFT_BLACK);
  
  // Draw axes
  tft.drawFastHLine(GRAPH_X, graphY, GRAPH_WIDTH, TFT_WHITE); // X-axis
  tft.drawFastVLine(GRAPH_X, graphY - GRAPH_HEIGHT, GRAPH_HEIGHT, TFT_WHITE); // Y-axis

  // Plot values as a line graph
  for (int i = 1; i < NUM_READINGS; i++) {
    int x1 = GRAPH_X + ((i - 1) * (GRAPH_WIDTH / NUM_READINGS));
    int y1 = graphY - map(graphData[i - 1], minValue, maxValue, 0, GRAPH_HEIGHT);
    int x2 = GRAPH_X + (i * (GRAPH_WIDTH / NUM_READINGS));
    int y2 = graphY - map(graphData[i], minValue, maxValue, 0, GRAPH_HEIGHT);
    
    // Draw the sensor reading line
    tft.drawLine(x1, y1, x2, y2, TFT_GREEN);
  }
}

// Function to display status information
void displayStatus(int sensorValue) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Display the sensor reading
  tft.println("---------------------------");
  tft.println("  KY035 Sensor Module");
  tft.println("---------------------------");

  tft.println("\nSensor Value:");
  tft.println(sensorValue);

  tft.println("\nReadings over time:");

  // Calculate the Y position for the graph based on the text height
  graphY = tft.getCursorY() + 70; // add padding below the text

  // Draw the graph
  drawGraph();
}


/*************************************************************
*********************** MAIN FUNCTIONS ***********************
**************************************************************/

// SETUP
void setup() {
  // Initialize the sensor pin
  pinMode(SENSOR_PIN, INPUT);

  // Initialize the backlight pin
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH); // turn on the backlight

  // TFT_eSPI setup
  tft.init();
  tft.setRotation(0);                     // adjust rotation (0 & 2 portrait | 1 & 3 landscape)
  tft.fillScreen(TFT_BLACK);              // set screen background colour (black)
  tft.setTextFont(2);                     // set the font (you can experiment with different fonts)
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // set text colour (white) and background colour (black)
}

// MAIN LOOP
void loop() {
  unsigned long currentTime = millis(); // get the current time

  // State machine logic
  switch (currentState) {
    case State::SENSOR_READ: // take a single sensor reading
      {
        readings[readIndex] = analogRead(SENSOR_PIN); // store the reading
        readIndex++;                // move to the next position in the array
        lastReadTime = currentTime; // record the time of the last reading

        if (readIndex >= NUM_READINGS) {
          // All readings have been taken, transition to AVERAGE state
          currentState = State::AVERAGE;
        }
        else {
          // Wait for the next reading
          currentState = State::WAIT;
        }
      }
      break;

    case State::WAIT: // wait for the next reading
      if (currentTime - lastReadTime >= READ_INTERVAL) {
        currentState = State::SENSOR_READ; // transition back to the SENSOR_READ state
      }
      break;

    case State::AVERAGE: // calculate the average of the readings
      {
        int sum = 0;
        for (int i = 0; i < NUM_READINGS; i++) {
          sum += readings[i]; // sum all readings
        }
        averagedValue = sum / NUM_READINGS; // calculate the average

        // Only update the screen if the averaged value has changed
        if (averagedValue != previousAveragedValue) {
          // Shift graph data left and add new value
          for (int i = 0; i < NUM_READINGS - 1; i++) {
            graphData[i] = graphData[i + 1];
          }
          graphData[NUM_READINGS - 1] = averagedValue;

          displayStatus(averagedValue);          // update the display
          previousAveragedValue = averagedValue; // store the new averaged value
        }

        // Reset for the next set of readings
        readIndex = 0;
        currentState = State::SENSOR_READ; // transition back to the SENSOR_READ state
      }
      break;
  }
}
