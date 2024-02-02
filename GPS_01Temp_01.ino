#include "Adafruit_MCP9601.h"
#include <Adafruit_NeoPixel.h>

#define I2C_ADDRESS1 (0x60)
#define I2C_ADDRESS2 (0x65)
#define I2C_ADDRESS3 (0x66)
#define I2C_ADDRESS4 (0x67)

Adafruit_MCP9601 mcp1;
Adafruit_MCP9601 mcp2;
Adafruit_MCP9601 mcp3;
Adafruit_MCP9601 mcp4;

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#define SD_CS 5

#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_PIN 45  // Change to your desired pin
#define NEOPIXEL_COUNT 1
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

SoftwareSerial mySerial(12, 13);
File logfile;

#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define PMTK_Q_RELEASE "$PMTK605*31"

unsigned long lastDataWriteTime = 0;

void setup() {
  mySerial.begin(9600);
  delay(2000);

  Serial.begin(115200);
    delay(10);

    /* Initialise the driver with I2C_ADDRESS and the default I2C bus. */
    if (! mcp4.begin(I2C_ADDRESS4)) {
        Serial.println("Sensor 4 not found. Check wiring!");
        while (1);
    }  


  Serial.println("Found MCP9601!");

  mcp4.setADCresolution(MCP9600_ADCRESOLUTION_14);
  Serial.print("ADC resolution set to ");
  switch (mcp4.getADCresolution()) {
    case MCP9600_ADCRESOLUTION_18:   Serial.print("18"); break;
    case MCP9600_ADCRESOLUTION_16:   Serial.print("16"); break;
    case MCP9600_ADCRESOLUTION_14:   Serial.print("14"); break;
    case MCP9600_ADCRESOLUTION_12:   Serial.print("12"); break;
  }
  Serial.println(" bits");

  mcp4.setThermocoupleType(MCP9600_TYPE_N);
  Serial.print("Thermocouple type set to ");
  switch (mcp4.getThermocoupleType()) {
    case MCP9600_TYPE_K:  Serial.print("K"); break;
    case MCP9600_TYPE_J:  Serial.print("J"); break;
    case MCP9600_TYPE_T:  Serial.print("T"); break;
    case MCP9600_TYPE_N:  Serial.print("N"); break;
    case MCP9600_TYPE_S:  Serial.print("S"); break;
    case MCP9600_TYPE_E:  Serial.print("E"); break;
    case MCP9600_TYPE_B:  Serial.print("B"); break;
    case MCP9600_TYPE_R:  Serial.print("R"); break;
  }
  Serial.println(" type");

  mcp4.setFilterCoefficient(0);
  Serial.print("Filter coefficient value set to: ");
  Serial.println(mcp4.getFilterCoefficient());

  mcp4.setAlertTemperature(1, 25);
  Serial.print("Alert #1 temperature set to ");
  Serial.println(mcp4.getAlertTemperature(1));
  mcp4.configureAlert(1, false, true);  // alert 1 enabled, rising temp

  mcp4.enable(true);

  Serial.println(F("------------------------------"));
  
     
  Serial.println("Software Serial GPS Test Echo Test");
  mySerial.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  mySerial.println(PMTK_SET_NMEA_UPDATE_10HZ);

  Serial.println("Get version!");
  mySerial.println(PMTK_Q_RELEASE);  

  // Initialize the SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    while (1);  // Halt the program if SD card initialization fails
  }
  
  // Create a unique filename for the log
  char filename[15];
  strcpy(filename, "/LOG00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = '0' + i / 10;
    filename[5] = '0' + i % 10;
    if (!SD.exists(filename)) {
      break;
    }
  }  

  // Open the log file for writing
  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.print("Could not create log file: ");
    Serial.println(filename);
  }

  // Print header for the log
  logfile.println("time UTC,latitude N ,longitude E,temperature °C");

  Serial.println("Ready!");

}

void parseGPRMC(const char *sentence) {
  char *token = strtok((char *)sentence, ",");
  int count = 0;

  while (token != NULL) {
    if (count == 1) {
      float gpstime = atof(token);
      Serial.print("Time: ");
      Serial.print(gpstime, 1);
      Serial.print(", ");
      logfile.print(gpstime, 1);
      logfile.print(", "); 
    } else if (count == 3) {
      float latitude = atof(token) / 100.0; // Move decimal point 2 positions to the left
      Serial.print("Latitude: ");
      Serial.print(latitude, 6);
      Serial.print(", ");
      logfile.print(latitude, 6);
      logfile.print(", ");      
    } else if (count == 5) {
      float longitude = atof(token) / 100.0; // Move decimal point 2 positions to the left
      Serial.print("Longitude: ");
      Serial.print(longitude, 6);
      Serial.print(", ");
      logfile.print(longitude, 6);
      logfile.print(", ");
      Serial.println(mcp4.readThermocouple(), 0);   
      logfile.println(mcp4.readThermocouple(), 0);
      // Flush the logfile to ensure data is written
      logfile.flush();
    }

    token = strtok(NULL, ",");
    count++;
  }
}


void updateNeopixel() {
  unsigned long currentTime = millis();
  if (currentTime - lastDataWriteTime <= 500) {  // Check if data was written in the last second
    // Green color if data was written
    neopixel.setPixelColor(0, neopixel.Color(0, 255, 0));
  } else if (currentTime - lastDataWriteTime <= 1000){
    // yellow color if no data was written in half a second
    neopixel.setPixelColor(0, neopixel.Color(255, 255, 0));
  } else {
    // Red color if no data was written in a second
    neopixel.setPixelColor(0, neopixel.Color(255, 0, 0));
  }
  neopixel.show();
}

void loop() {
  updateNeopixel();  // Update Neopixel based on data write status
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
  }

  if (mySerial.available()) {
    char c = mySerial.read();

    if (c == '\n') {
      char buffer[100];
      mySerial.readBytesUntil('\n', buffer, sizeof(buffer));
      if (strstr(buffer, "$GNRMC") != NULL) {
        parseGPRMC(buffer);
        lastDataWriteTime = millis();  // Update the time of the last data write

      }
    }
  }
}
