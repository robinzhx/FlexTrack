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
                _m_helper.setFixedLengthBasedStream(1);

                // callback function for successful connection
                _m_helper.OnConnected += () =>
                {
                    Log("Connect successfully");
                    // Successful connection, start listening for messages
                    _m_helper.StartListening();
                };

                // callback function for connection failure
                _m_helper.OnConnectionFailed += () =>
                {
                    Log("Connection failed");
                    Disconnect();
                };

                // No callback function found for the device
                _m_helper.OnServiceNotFound += serviceName =>
                {
                    Log("No device found:" + serviceName);
                    // Disconnect
                    Disconnect();
                };

                // callback function for received message
                _m_helper.OnDataReceived += () =>
                {
                    Log("A new message was received");
                    // process the received callback
                    _m_msgHandler(_m_helper.Read());
                };

            }
            catch (Exception e)
            {
                Log("Error in connection:" + e.Message);
                Disconnect();
            }
        }

        /// <summary>
        /// output log
        /// </ summary>
        /// <param name = "log"> Log content </ param>
        public void Log(string log)
        {
            m_log_txt.text += "\n" + log;
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
