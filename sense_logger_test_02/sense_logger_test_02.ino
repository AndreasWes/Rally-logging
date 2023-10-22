#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS33.h>

// Set the pins used
#define cardSelect 10

// Define sensor objects
Adafruit_BMP280 bmp280;     // Barometric pressure sensor
Adafruit_LIS3MDL lis3mdl;   // Magnetometer
Adafruit_LSM6DS33 lsm6ds33; // Accelerometer and Gyroscope

// Sensor data variables
float pressure;
float magnetic_x, magnetic_y, magnetic_z;
float accel_x, accel_y, accel_z;
float gyro_x, gyro_y, gyro_z;

File logfile;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize the SD card
  if (!SD.begin(cardSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);  // Halt the program if SD card initialization fails
  }

  // Create a unique filename for the log
  char filename[15];
  strcpy(filename, "/SENSOR00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i / 10;
    filename[8] = '0' + i % 10;
    if (!SD.exists(filename)) {
      break;
    }
  }

  // Open the log file for writing
  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.print("Could not create log file: ");
    Serial.println(filename);
    while (1);  // Halt the program if log file creation fails
  }

  // Initialize the sensors
  bmp280.begin();
  lis3mdl.begin_I2C();
  lsm6ds33.begin_I2C();

  // Print header for the log
  logfile.println("Time (ms), Pressure (Pa), Magnetic X (uTesla), Magnetic Y (uTesla), Magnetic Z (uTesla), Acceleration X (m/s^2), Acceleration Y (m/s^2), Acceleration Z (m/s^2), Gyro X (dps), Gyro Y (dps), Gyro Z (dps)");

  Serial.println("Ready!");
}

void loop() {
  // Read sensor data
  pressure = bmp280.readPressure();
  lis3mdl.read();
  magnetic_x = lis3mdl.x;
  magnetic_y = lis3mdl.y;
  magnetic_z = lis3mdl.z;
  
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds33.getEvent(&accel, &gyro, &temp);
  accel_x = accel.acceleration.x;
  accel_y = accel.acceleration.y;
  accel_z = accel.acceleration.z;
  gyro_x = gyro.gyro.x;
  gyro_y = gyro.gyro.y;
  gyro_z = gyro.gyro.z;
  
  // Get the current microcontroller time (in milliseconds)
  unsigned long currentTime = millis();

  // Log sensor data to the SD card with microcontroller time
  logfile.print(currentTime);
  logfile.print(", ");
  logfile.print(pressure);
  logfile.print(", ");
  logfile.print(magnetic_x);
  logfile.print(", ");
  logfile.print(magnetic_y);
  logfile.print(", ");
  logfile.print(magnetic_z);
  logfile.print(", ");
  logfile.print(accel_x);
  logfile.print(", ");
  logfile.print(accel_y);
  logfile.print(", ");
  logfile.print(accel_z);
  logfile.print(", ");
  logfile.print(gyro_x);
  logfile.print(", ");
  logfile.print(gyro_y);
  logfile.print(", ");
  logfile.println(gyro_z);

  // Flush the logfile to ensure data is written
  logfile.flush();
}
