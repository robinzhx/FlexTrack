/*
  This script can connect to BLE and reads the acceleration 
  values from the LSM6DS3 sensor to continuously prints them 
  to the Serial Monitor or Serial Plotter, also BLE client

  The circuit:
  - Arduino Nano 33 IoT

  created 26 May 2020
  by Zhuoqun Robin Xu
*/

#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

/** 
 *  Inertial Measurement Unit(IMU) Service - somewhat surprisely I could not find a standard IMU
 *  service on the Bluetooth SIG website(https://www.bluetooth.com/develop-with-bluetooth). So I 
 *  used an online uuid generator(http://www.itu.int/en/ITU-T/asn1/Pages/UUID/uuids.aspx) to 
 *  create this uuid. You are welcome to use it or make your own. Keep in mind that custom service 
 *  uuid must always be specified in 128 bit format as seen below.
*/ 
//BLEService imuService("0396df23-7b62-468a-bd60-db06b6dc36ce"); // Custom UUID
BLEService imuService("1101");
BLEFloatCharacteristic imuAXChar("2101", BLERead | BLENotify);
BLEFloatCharacteristic imuAYChar("2102", BLERead | BLENotify);
BLEFloatCharacteristic imuAZChar("2103", BLERead | BLENotify);
BLEFloatCharacteristic imuGXChar("2104", BLERead | BLENotify);
BLEFloatCharacteristic imuGYChar("2105", BLERead | BLENotify);
BLEFloatCharacteristic imuGZChar("2106", BLERead | BLENotify);

const char localName[] = "FlexTrackIoT";

// Temporary val
float x, y, z;
// acceleration data 
float  aX=1;
float  aY=1;
float  aZ=0;
// gyroscope data 
float gX=0;
float gY=0;
float gZ=0;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    delay(500);
    while (1);
  }
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.print(" Hz ");
  Serial.println("(in G's)");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.print(" Hz ");
  Serial.println("(in degrees/second)");

  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    delay(500);
    while (1);
  }

  BLE.setLocalName(localName);
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(imuAXChar);
  imuService.addCharacteristic(imuAYChar);
  imuService.addCharacteristic(imuAZChar);
  imuService.addCharacteristic(imuGXChar);
  imuService.addCharacteristic(imuGYChar);
  imuService.addCharacteristic(imuGZChar);
    
  BLE.addService(imuService);
  
  imuAXChar.writeValue(aX);
  imuAYChar.writeValue(aY);
  imuAZChar.writeValue(aZ);
  imuGXChar.writeValue(gX);
  imuGYChar.writeValue(gY);
  imuGZChar.writeValue(gZ);
  
  BLE.advertise();

  Serial.println("Bluetooth device is now active, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();
  
  if(central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("end if");
    while (central.connected()) {
      //delay(200);
      Serial.println("while");
      
      read_Accel();
      read_Gyro();
      
      imuAXChar.writeValue(aX);
      imuAYChar.writeValue(aY);
      imuAZChar.writeValue(aZ);
      imuGXChar.writeValue(gX);
      imuGYChar.writeValue(gY);
      imuGZChar.writeValue(gZ);
      
      Serial.print(aX);
      Serial.print('\t');
      Serial.print(aY);
      Serial.print('\t');
      Serial.print(aZ);
      Serial.print('\t');
      Serial.print('\t');
      Serial.print(gX);
      Serial.print('\t');
      Serial.print(gY);
      Serial.print('\t');
      Serial.println(gZ);
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
  delay(1000);
}

void read_Accel() {

  if (IMU.accelerationAvailable()) 
  {
    IMU.readAcceleration(x, y, z);
//    aX = (1+x)*100;
//    aY = (1+y)*100;
//    aZ = (1+z)*100;
    aX = x;
    aY = y;
    aZ = z;
  }
}

void read_Gyro() {

  if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(x, y, z);
//    aX = (1+x)*100;
//    aY = (1+y)*100;
//    aZ = (1+z)*100;
    gX = x;
    gY = y;
    gZ = z;
  }
}
