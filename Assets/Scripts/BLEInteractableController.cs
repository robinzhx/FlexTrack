using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using ArduinoBluetoothAPI;
using System;
using UnityEngine.UI;


/*********
Author: Robin Xu
Email: robinxu@ucsd.com
Feature: Test interaction module of the Arduino Bluetooth module
*********/

namespace Bluetooth
{
    public class BLEInteractableController : MonoBehaviour
    {
        BluetoothHelper _m_helper;

        public InputField m_deviceName_ipt; // Enter device name
        //public Button m_connect_btn; // Connect Bluetooth button
        //public Button m_disconnect_btn; // Disconnect Bluetooth button
        //public Button m_clearLog_btn; // Clear the date panel information button
        //public Button m_hideLog_btn; // hide the current panel button, generally close these panels after successful connection, enter the control
        public Text m_log_txt; // input log information

        float gX, gY, gZ;

        public GameObject visObject;

        string received_message;

        //string[] subscribeAddresses = { "2104", "2105", "2106" };
        string[] subscribeAddresses = { "2107", "2108", "2109" };

        // Message processing callback
        Action<string> _m_msgHandler;
        public void AddMsgHandler(Action<string> handler)
        {
            _m_msgHandler = handler;
        }

        void Awake()
        {

            try
            {
                //use Bluetooth Low Energy Technology
                BluetoothHelper.BLE = true;  
                // Get BluetoothHelper instance
                _m_helper = BluetoothHelper.GetInstance();
                // Turn on Bluetooth
                _m_helper.EnableBluetooth(true);

                // Set the length of the send and receive characters, here is the key point, 
                // if you dont set it, you can receive it, you cant send a message, and you 
                // can only send it to a certain amount of data cache

                //_m_helper.setFixedLengthBasedStream(1);
                //_m_helper.setTerminatorBasedStream("\n");

                _m_helper.OnConnected += OnConnected;
                _m_helper.OnConnectionFailed += OnConnectionFailed;
                //_m_helper.OnDataReceived += OnMessageReceived; //read the data
                _m_helper.OnScanEnded += OnScanEnded;
                _m_helper.OnCharacteristicChanged += OnCharacteristicChanged;

                BluetoothHelperService service = new BluetoothHelperService("1101");
                service.addCharacteristic(new BluetoothHelperCharacteristic(subscribeAddresses[0]));
                service.addCharacteristic(new BluetoothHelperCharacteristic(subscribeAddresses[1]));
                service.addCharacteristic(new BluetoothHelperCharacteristic(subscribeAddresses[2]));
                _m_helper.Subscribe(service);


                // No callback function found for the device
                //_m_helper.OnServiceNotFound += serviceName =>
                //{
                //    Log("No device found:" + serviceName);
                //    // Disconnect
                //    Disconnect();
                //};

                Scan();

            }
            catch (Exception e)
            {
                Log("Error in connection:" + e.Message);
                Disconnect();
            }
        }

        void OnCharacteristicChanged(byte[] val, BluetoothHelperCharacteristic id)
        {
            if (id.getName().Equals("0000" + subscribeAddresses[0] + "-0000-1000-8000-00805f9b34fb")) // BASE_UUID conversion
            {
                gX = System.BitConverter.ToSingle(val, 0);
            }
            else if (id.getName().Equals("0000" + subscribeAddresses[1] + "-0000-1000-8000-00805f9b34fb")) // BASE_UUID conversion
            {
                gY = System.BitConverter.ToSingle(val, 0);
            }
            else if (id.getName().Equals("0000" + subscribeAddresses[2] + "-0000-1000-8000-00805f9b34fb")) // BASE_UUID conversion
            {
                gZ = System.BitConverter.ToSingle(val, 0);
            }
            else
            {
                Log(id.getName()  + " not equal to" + subscribeAddresses[0]);
            }

            //Log(gX + " " + gY + " " + gZ);

            //float c1 = Mathf.Cos(Mathf.Deg2Rad * gX);
            //float s1 = Mathf.Sin(Mathf.Deg2Rad * gX);
            //float c2 = Mathf.Cos(Mathf.Deg2Rad * gY);
            //float s2 = Mathf.Sin(Mathf.Deg2Rad * gY);
            //float c3 = Mathf.Cos(Mathf.Deg2Rad * gZ);
            //float s3 = Mathf.Sin(Mathf.Deg2Rad * gZ);

            //Matrix4x4 convertion = new Matrix4x4(
            //    new Vector4 (c2 * c3, s1 * s3 + c1 * c3 * s2, c3 * s1 * s2 - c1 * s3, 0),
            //    new Vector4(-s2, c1 * c2, c2 * s1, 0),
            //    new Vector4(c2 * s3, c1 * s2 * s3 - c3 * s1, c1 * c3 + s1 * s2 * s3, 0),
            //    new Vector4(0, 0, 0, 1)
            //).transpose;



            if (visObject)
            {
                //visObject.transform.rotation *= Quaternion.LookRotation(convertion.GetColumn(2), convertion.GetColumn(1));
                visObject.transform.eulerAngles = new Vector3(gX, gY, gZ);
            }

        }

        // callback function for successful connection
        void OnConnected()
        {
            Log("Connect successfully");
            // Successful connection, start listening for messages
            _m_helper.StartListening();
        }

        // callback function for connection failure
        void OnConnectionFailed()
        {
            Log("Connection failed");
            Disconnect();
        }

        // callback function for received message
        void OnMessageReceived()
        {
            Log("A new message was received");
            // process the received callback
            _m_msgHandler(_m_helper.Read());
        }

        void OnScanEnded(LinkedList<BluetoothDevice> nearbyDevices)
        {
            Log("1 Scan ended");

            if (nearbyDevices.Count == 0)
            {
                _m_helper.ScanNearbyDevices();
                return;
            }


            foreach (BluetoothDevice device in nearbyDevices)
            {
                Log(device.DeviceName);
                if (device.DeviceName == "FlexTrackIoT")
                    Log("FOUND!!");
            }

            _m_helper.setDeviceName("FlexTrackIoT");
            _m_helper.Connect();
            //if (_m_helper.isDevicePaired())
            //    Log("Connected: " + _m_helper.getDeviceName());

        }

        /// <summary>
        /// output log
        /// </ summary>
        /// <param name = "log"> Log content </ param>
        public void Log(string log)
        {
            m_log_txt.text = log + "\n" + m_log_txt.text;
        }


        /// <summary>
        /// Send a message
        /// </ summary>
        /// <param name = "msg"> Content of the message </ param>
        public void Send(string msg)
        {
            _m_helper.SendData(msg);
        }


        void OnDestroy()
        {
            Disconnect();
        }


        /// <summary>
        /// Disconnect
        /// </summary>
        public void Disconnect()
        {
            Log("Disconnect");
            if (_m_helper != null)
                _m_helper.Disconnect();
        }

        // Add event to connect button
        public void connect()
        {
            if (!string.IsNullOrEmpty(m_deviceName_ipt.text))
            {
                Log("Start connecting Bluetooth:" + m_deviceName_ipt.text);
                // Set the connected device name
                _m_helper.setDeviceName(m_deviceName_ipt.text);
                // Start connection
                _m_helper.Connect();
            }
        }

        // Add event to connect button
        public void Scan()
        {
            _m_helper.ScanNearbyDevices();
            Log("start scan");
        }

        // hide panel operation event monitoring
        public void hideUnhideLog()
        {
            gameObject.SetActive(false);
        }

        // Add event of log clear button
        public void clearLog()
        {
            m_log_txt.text = "BLE Log";
        }
    }
}
