/**

Debugging I did and self Reflection:
1. The BMP280 chip did not exist so had to use external projects to copy the specifications, However I did a typo in bmp.chip.json and wrote bm.chip.json, had to rectify the error...
2. the SD card was unable to open because of excessive RAM usage by print statements, so I used Serial.print(F()) to store print in Flash memory...

*/


#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

//Now we define the objects of the specific libraries...
Adafruit_BMP280 bmp; // Pressure & Temperature
Adafruit_MPU6050 mpu; // Acceleration and Gyro sensor
File logFile; //object to enable File handling
//The text log file name is Changed to FLIGHT.txt
//This was done because Arduino can't support the 10 Character long FLIGHT_LOG.TXT



const byte BMP_ADDRESS =0x76;
const byte MPU_ADDRESS =0x68;
const byte LCD_ADDRESS =0x27;
const int CS_PIN=10;// Chip Select pin of SD Card.
int RED_LED=7; //we will run red LED as warning from digital pin 7..
int GREEN_LED=6; //run green LED as system running light
int LCD_columns=16;
int LCD_rows=2;

//Initialise the lcd
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_columns,LCD_rows); 



int samplingTime=1000;//Set the sampling interval as 1 second
int delayTime=1000;

//Function for startup I2C scan
void scanI2C() {
  byte error, address;
  int nDevices = 0;

  Serial.println(F("Scanning I2C bus..."));

  for(address = 0; address < 128; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();//If the device does not return the transmission, then an error

    if (error == 0) //there exists an I2C device at this address.
    {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(F("  !")); // F tells compiler to keep this in Flash memory and save RAM.
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println(F("No I2C devices found\n"));
  else Serial.println(F("I2C Scan done.\n"));
}

// the I2C startup scan finds Address as follows
// MPU6050: 0x68
// LCD: 0x27
// BMP280: 0x76

// Function for Handling faults and turning on warning LED
void triggerFault() {
  digitalWrite(RED_LED, HIGH); // Turn on RED LED 
  digitalWrite(GREEN_LED, LOW); //Turn off GREEN LED
  while (1); // Stops Exectution
}





void setup() {

  Serial.begin(9600); // Start serial communication for output
  while(!Serial);//waiting for serial window to open..

  //Set Pin mode for the LED.
  pinMode(RED_LED,OUTPUT);
  pinMode(CS_PIN,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);


  //I2C startup scan.
  Wire.begin(); // Join the I2C bus
  scanI2C();

  //BMP check
  if(!bmp.begin(BMP_ADDRESS))
  {
  Serial.println(F(" X BMP Connection failed "));
  triggerFault(); //stop here.. no sensor found
  }
  Serial.println(F("BMP OK "));

  //MPU check
  if(!mpu.begin(MPU_ADDRESS))
  {
  Serial.println(F(" X MPU Connection failed "));
  triggerFault();//stop here.. no sensor found
  }
  Serial.println(F("MPU OK "));

  // Setting MPU ranges...
  //Setting accelerometer range: options- 2 4 8 16 G. 
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  //Setting Gyroscope Range as 500 deg/s 
  //Chose this for our rocket as precision needed.
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); 

  //Setting Bandwidth to filter noise.
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);


  //Setting up SD card
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(CS_PIN)) {
    Serial.println(F("SD Card Init Failed!"));
    triggerFault();
  }
  Serial.println(F("SD Card Initialized."));

  // Opening Log Data File
  logFile = SD.open("FLIGHT.TXT", FILE_WRITE); 
  
  if (logFile) {
    // Write Header
    Serial.println(F("Writing Header to File..."));
    logFile.println("TIME_MS,TEMP_C,PRESS_PA,ACC_X,ACC_Y,ACC_Z,GYRO_X,GYRO_Y,GYRO_Z");
    logFile.close(); // Close to save header
    Serial.println(F("Log File Created."));
  }
  
  else
  {
    Serial.println(F("Error While opening Flight.txt"));
    triggerFault();
  }

  //Initialising the LCD Display

  lcd.init();
  lcd.backlight(); // Turn on the light
  
  // Show a startup message
  lcd.setCursor(0, 0); // Column 0, Row 0
  lcd.print("Rocket Start");
  delay(delayTime); 
  lcd.clear();

  //Print everything is ok on LCD
  lcd.print("BMP OK");
  delay(delayTime);
  lcd.clear();

  lcd.print("MPU OK");
  delay(delayTime);
  lcd.clear();

  lcd.print("SD CARD OK");
  delay(delayTime);
  lcd.clear();


  digitalWrite(GREEN_LED,HIGH); //Turn on GREEN LED


}

//Helper Function to print to terminal in telemetry fromat
void print(float bmpTemp, float bmpPres, sensors_event_t a, sensors_event_t g)
{
  // --- Print Telemetry Data (CSV Format) ---
  //Prints Time
  Serial.print(millis()); 
  Serial.print(", ");
  
  // BMP280 Data
  Serial.print(bmpTemp);
  Serial.print(F(", "));
  Serial.print(bmpPres);
  Serial.print(F(", "));

  // MPU6050 Acceleration (m/s^2)
  Serial.print(a.acceleration.x);
  Serial.print(F(", "));
  Serial.print(a.acceleration.y);
  Serial.print(F(", "));
  Serial.print(a.acceleration.z);
  Serial.print(F(", "));

  // MPU6050 Gyro (rad/s)
  Serial.print(g.gyro.x);
  Serial.print(F(", "));
  Serial.print(g.gyro.y);
  Serial.print(F(", "));
  Serial.println(g.gyro.z);
}

void logData(float bmpTemp, float bmpPres, sensors_event_t a, sensors_event_t g) {
  // Open file for appending
  logFile = SD.open("FLIGHT.TXT", FILE_WRITE);

  // If file opens
  if (logFile) {
    // Writing the data in the same format
    logFile.print(millis());
    logFile.print(", ");
    logFile.print(bmpTemp);
    logFile.print(", ");
    logFile.print(bmpPres);
    logFile.print(", ");
    logFile.print(a.acceleration.x);
    logFile.print(", ");
    logFile.print(a.acceleration.y);
    logFile.print(", ");
    logFile.print(a.acceleration.z);
    logFile.print(", ");
    logFile.print(g.gyro.x);
    logFile.print(", ");
    logFile.print(g.gyro.y);
    logFile.print(", ");
    logFile.println(g.gyro.z); // End with a new line

    logFile.close(); // Save the data
     
  } 
  else {
    // If file didn't open, indicate error
    Serial.println(F(" Error writing to SD!"));
    digitalWrite(RED_LED,HIGH);
    //triggerFault(); We didn't use triggerFault() to avoid complete stoppage of rocket
  }
}

//Helper Function to display System parameters
void Display(float t, float p, float az, float gres)
{
  // --- Row 0: Temperature & Pressure ---
  lcd.setCursor(0, 0); 
  lcd.print("T:"); 
  lcd.print((int)t); // Cast to int to save space
  lcd.print(" P:"); 
  lcd.print((int)p); // Cast to int
  
  //Row 1: Z-Acceleration and Resultant Gyro
  lcd.setCursor(0, 1);
  lcd.print("AccZ:");
  lcd.print(az);
  lcd.print("Gyro:");
  lcd.print((int)gres);

}
void loop() {
 
 /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Read from BMP280
  float bmpTemp = bmp.readTemperature();
  float bmpPres = bmp.readPressure() / 100.0F; // Convert Pa to hPa

  //Print the output to serial in Telemetry format../
  print(bmpTemp,bmpPres,a,g);
  logData(bmpTemp,bmpPres,a,g);

  float gx=g.gyro.x,gy=g.gyro.y,gz=g.gyro.z;
  float gres=sqrt(gx*gx+gy*gy+gz*gz); //calculating resultant Gyroscopic value
  
  //Display to LCD: delay time in printing is equal to the samplingTime
  Display(bmpTemp,bmpPres,a.acceleration.z,gres);


  

  delay(samplingTime); //Wait until Sampling Time for next scan...

  lcd.clear();//Clears LCD for next set of Data 
}