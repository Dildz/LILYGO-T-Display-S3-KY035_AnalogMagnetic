 LILYGO T-Display-S3 KY035 Analogue Hall Magnetic Sensor Project

 Description:
   This code reads the analog output of a KY035 Analogue Hall Magnetic Sensor Module connected to a
   LilyGO T-Display-S3 microcontroller board.
   
   It displays the sensor readings using a analog meter using the TFT_eSPI and TFT_eWidget libraries.
   
   The project demonstrates embedded programming concepts, including:

   - Analog Inputs: Reading sensor data from an analog pin.
   - Variables: Storing and manipulating data, such as the sensor reading and averaged values.
   - Functions: Placing code into reusable blocks for better organization and modularity.
   - TFT_eSPI and TFT_eWidget Libraries: Utilizing these libraries for direct display control and rendering an analog meter based off of the Meters library example.
   - Data Averaging: Taking multiple readings and averaging them to help smooth the reading.
   - Real-Time Meter Display: Visualizing sensor data as a needle on an analog meter.

 How It Works:

   1. Sensor Reading: The code reads the analog output of the KY035 sensor, which varies with the strength of the magnetic field.
   2. Averaging: Multiple readings are taken and averaged to provide a more stable value.
   3. Display Update: The averaged sensor reading is displayed as a needle on an analog meter, with the raw ADC value also shown on the screen.
   4. TFT_eWidget Meter: The project utilizes the TFT_eWidget library to render an analog meter with zones for visual feedback.

 Pin Connections:

   - Sensor (S)   -> A0 (Analog Input 0)
   - Ground (-)   -> GND
   - VCC (middle) -> 3.3V

 Notes:

   - The KY035 sensor outputs an analog signal proportional to the magnetic field strength.
   - The TFT_eSPI and TFT_eWidget libraries are configured to work with the LilyGO T-Display-S3, providing an easy way to display information on the built-in screen.
   - The analog meter dynamically updates based on the sensor readings, providing real-time feedback.
 
 KY035 Specifications:

   - Operating Voltage: 2.7V to 6V
   - Power Consumption: ~ 6mA
   - Sensitivity: 1.4 to 2.0mV/GS
   - Operating Temperature: -40 °C to 85 °C [-40 °F to 185 °F]