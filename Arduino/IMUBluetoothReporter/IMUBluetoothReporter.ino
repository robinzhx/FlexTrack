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
#include <MadgwickAHRS.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"

//#ifndef _BV
//#define _BV(bit) (1 << (bit)) 
//#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

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

// BLEFloatCharacteristic imuRollChar("2107", BLERead | BLENotify);
// BLEFloatCharacteristic imuPitchChar("2108", BLERead | BLENotify);
// BLEFloatCharacteristic imuHeadChar("2109", BLERead | BLENotify);

BLEFloatCharacteristic imuQ0Char("2107", BLERead | BLENotify);
BLEFloatCharacteristic imuQ1Char("2108", BLERead | BLENotify);
BLEFloatCharacteristic imuQ2Char("2109", BLERead | BLENotify);
BLEFloatCharacteristic imuQ3Char("2110", BLERead | BLENotify);

BLEUnsignedIntCharacteristic touchChar("2111", BLERead | BLENotify);

BLEDevice central;

const char localName[] = "FlexTrackIoT";

Madgwick filter;

// Temporary val
float x, y, z;
// acceleration data
float ax, ay, az;
// gyroscope data
float gx, gy, gz;
//float roll, pitch, heading;
float q0, q1, q2, q3;
//timer
unsigned long microsPerReading, microsPrevious;

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

    if (!IMU.begin())
    {
        Serial.println("Failed to initialize IMU!");
        delay(500);
        while (1);
    }

    int rate = IMU.accelerationSampleRate();

    Serial.print("Accelerometer sample rate = ");
    Serial.print(rate);
    Serial.print(" Hz ");
    Serial.println("(in G's)");
    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.print(" Hz ");
    Serial.println("(in degrees/second)");

    filter.begin(rate);

    // initialize variables to pace updates to correct rate
    microsPerReading = 1000000 / rate;
    microsPrevious = micros();

    // Default address is 0x5A, if tied to 3.3V its 0x5B
    // If tied to SDA its 0x5C and if SCL then 0x5D
    if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
    }
    Serial.println("MPR121 found!");

    if (!BLE.begin())
    {
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

    // imuService.addCharacteristic(imuRollChar);
    // imuService.addCharacteristic(imuPitchChar);
    // imuService.addCharacteristic(imuHeadChar);

    imuService.addCharacteristic(imuQ0Char);
    imuService.addCharacteristic(imuQ1Char);
    imuService.addCharacteristic(imuQ2Char);
    imuService.addCharacteristic(imuQ3Char);

    imuService.addCharacteristic(touchChar);

    BLE.addService(imuService);

    imuAXChar.writeValue(ax);
    imuAYChar.writeValue(ay);
    imuAZChar.writeValue(az);
    imuGXChar.writeValue(gx);
    imuGYChar.writeValue(gy);
    imuGZChar.writeValue(gz);

    // imuRollChar.writeValue(roll);
    // imuPitchChar.writeValue(pitch);
    // imuHeadChar.writeValue(heading);

    imuQ0Char.writeValue(q0);
    imuQ1Char.writeValue(q1);
    imuQ2Char.writeValue(q2);
    imuQ3Char.writeValue(q3);

    touchChar.writeValue(currtouched);

    BLE.advertise();

    Serial.println("Bluetooth device is now active, waiting for connections...");
}

void loop()
{
    unsigned long microsNow;

    // check if it's time to read data and update the filter 
    microsNow = micros();
    if (microsNow - microsPrevious >= microsPerReading) 
    {
        IMU.readAcceleration(ax, ay, az);
        IMU.readGyroscope(gx, gy, gz);

        // Serial.print(ax);
        // Serial.print('\t');
        // Serial.print(ay);
        // Serial.print('\t');
        // Serial.print(az);
        // Serial.print('\t');
        // Serial.print('\t');
        // Serial.print(gx);
        // Serial.print('\t');
        // Serial.print(gy);
        // Serial.print('\t');
        // Serial.println(gz);

        // update the filter, which computes orientation
        filter.updateIMU(gx, gy, gz, ax, ay, az);

        // print the heading, pitch and roll
        // roll = filter.getRoll();
        // pitch = filter.getPitch();
        // heading = filter.getYaw();

        //float * q = filter.getQuaternion();

        q0 = filter.getQ0();
        q1 = filter.getQ1();
        q2 = filter.getQ2();
        q3 = filter.getQ3();

        float roll, pitch, heading;

        roll = filter.getRoll();
        pitch = filter.getPitch();
        heading = filter.getYaw();
        Serial.print("Orientation: ");
        Serial.print(heading);
        Serial.print(" ");
        Serial.print(pitch);
        Serial.print(" ");
        Serial.println(roll);

        // Serial.print("Orientation: ");
        // Serial.print(q0);
        // Serial.print(" ");
        // Serial.print(q1);
        // Serial.print(" ");
        // Serial.println(q2);
        // Serial.print(" ");
        // Serial.println(q3);

        // Get the currently touched pads
        currtouched = cap.touched();

        touchChar.writeValue(currtouched);
                
        // for (uint8_t i=0; i<12; i++) {
        //     // it if *is* touched and *wasnt* touched before, alert!
        //     if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
        //     Serial.print(i); Serial.println(" touched");
        //     }
        //     // if it *was* touched and now *isnt*, alert!
        //     if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        //     Serial.print(i); Serial.println(" released");
        //     }
        // }

        // reset our state
        lasttouched = currtouched;

        // increment previous time, so we keep proper pace
        microsPrevious = microsPrevious + microsPerReading;
    }
    
    if (central)
    {
        if (central.connected()) {
            writeBLEdata();
        }
        else 
        {
            digitalWrite(LED_BUILTIN, LOW);
            Serial.print("Disconnected from central: ");
            Serial.println(central.address());
            central = BLE.central();
        }

    }
    else 
    {
        central = BLE.central();
        if (central && central.connected()) 
        {
            Serial.print("Connected to central: ");
            Serial.println(central.address());
            digitalWrite(LED_BUILTIN, HIGH);
            writeBLEdata();
        }
    }
        
}

void writeBLEdata() 
{
    imuAXChar.writeValue(ax);
    imuAYChar.writeValue(ay);
    imuAZChar.writeValue(az);
    imuGXChar.writeValue(gx);
    imuGYChar.writeValue(gy);
    imuGZChar.writeValue(gz);

    // imuRollChar.writeValue(roll);
    // imuPitchChar.writeValue(pitch);
    // imuHeadChar.writeValue(heading);

    imuQ0Char.writeValue(q0);
    imuQ1Char.writeValue(q1);
    imuQ2Char.writeValue(q2);
    imuQ3Char.writeValue(q3);
}

void read_Accel()
{

    if (IMU.accelerationAvailable())
    {
        IMU.readAcceleration(x, y, z);
        //    ax = (1+x)*100;
        //    ay = (1+y)*100;
        //    az = (1+z)*100;
        ax = x;
        ay = y;
        az = z;
    }
}

void read_Gyro()
{

    if (IMU.gyroscopeAvailable())
    {
        IMU.readGyroscope(x, y, z);
        //    ax = (1+x)*100;
        //    ay = (1+y)*100;
        //    az = (1+z)*100;
        gx = x;
        gy = y;
        gz = z;
    }
}
