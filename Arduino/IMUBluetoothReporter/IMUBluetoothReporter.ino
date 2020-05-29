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

BLEFloatCharacteristic imuRollChar("2107", BLERead | BLENotify);
BLEFloatCharacteristic imuPitchChar("2108", BLERead | BLENotify);
BLEFloatCharacteristic imuHeadChar("2109", BLERead | BLENotify);

const char localName[] = "FlexTrackIoT";

Madgwick filter;

// Temporary val
float x, y, z;
// acceleration data
float aX, aY, aZ;
// gyroscope data
float gX, gY, gZ;
float roll, pitch, heading;
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

    imuService.addCharacteristic(imuRollChar);
    imuService.addCharacteristic(imuPitchChar);
    imuService.addCharacteristic(imuHeadChar);

    BLE.addService(imuService);

    imuAXChar.writeValue(aX);
    imuAYChar.writeValue(aY);
    imuAZChar.writeValue(aZ);
    imuGXChar.writeValue(gX);
    imuGYChar.writeValue(gY);
    imuGZChar.writeValue(gZ);

    imuRollChar.writeValue(roll);
    imuPitchChar.writeValue(pitch);
    imuHeadChar.writeValue(heading);

    BLE.advertise();

    Serial.println("Bluetooth device is now active, waiting for connections...");
}

void loop()
{
    BLEDevice central = BLE.central();
    unsigned long microsNow;

    // check if it's time to read data and update the filter
    microsNow = micros();
    if (central)
    {
        Serial.print("Connected to central: ");
        Serial.println(central.address());
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("end if");
        while (central.connected())
        {
            // check if it's time to read data and update the filter
            microsNow = micros();
            if (microsNow - microsPrevious >= microsPerReading) {
                //delay(200);

                read_Accel();
                read_Gyro();

                imuAXChar.writeValue(aX);
                imuAYChar.writeValue(aY);
                imuAZChar.writeValue(aZ);
                imuGXChar.writeValue(gX);
                imuGYChar.writeValue(gY);
                imuGZChar.writeValue(gZ);

                // Serial.print(aX);
                // Serial.print('\t');
                // Serial.print(aY);
                // Serial.print('\t');
                // Serial.print(aZ);
                // Serial.print('\t');
                // Serial.print('\t');
                // Serial.print(gX);
                // Serial.print('\t');
                // Serial.print(gY);
                // Serial.print('\t');
                // Serial.println(gZ);

                // update the filter, which computes orientation
                filter.updateIMU(gX, gY, gZ, aX, aY, aZ);

                // print the heading, pitch and roll
                roll = filter.getRoll();
                pitch = filter.getPitch();
                heading = filter.getYaw();
                Serial.print("Orientation: ");
                Serial.print(roll);
                Serial.print(" ");
                Serial.print(pitch);
                Serial.print(" ");
                Serial.println(heading);

                imuRollChar.writeValue(roll);
                imuPitchChar.writeValue(pitch);
                imuHeadChar.writeValue(heading);

                // increment previous time, so we keep proper pace
                microsPrevious = microsPrevious + microsPerReading;
            }
        }
        digitalWrite(LED_BUILTIN, LOW);
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
    delay(1000);
}

void read_Accel()
{

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

void read_Gyro()
{

    if (IMU.gyroscopeAvailable())
    {
        IMU.readGyroscope(x, y, z);
        //    aX = (1+x)*100;
        //    aY = (1+y)*100;
        //    aZ = (1+z)*100;
        gX = x;
        gY = y;
        gZ = z;
    }
}
