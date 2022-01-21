//https://docs.arduino.cc/library-examples/wifi-library/WiFiRTC
#include <WiFi101.h>
#include <RTCZero.h>

RTCZero rtc;
#include "arduino_secrets.h"

const char ssid[] = SECRET_SSID;        // your network SSID (name)
const char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// Check the connection every 10 seconds
const long interval = 10000;
unsigned long previous_millis = 0;
bool wifi_first_connection = true;

// Check the NTP server every 10 minutes
const int GMT = +1; //change this to adapt it to your time zone

const long ntp_interval = 60000 * 5;
unsigned long ntp_previous_millis = 0;
bool is_ntp_time_sync = false;

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // No Wifi Connection

  rtc.begin(true); //resetTime = true
}

void loop() {
  const unsigned long current_millis = millis();

  // Checking the network status every 10 seconds
  if (current_millis - previous_millis >= interval or wifi_first_connection) {
    previous_millis = current_millis;
    status = WiFi.status();
    if (isWifiConnected()) {
      digitalWrite(LED_BUILTIN, LOW); // Wifi Ok
      
      //printWiFiStatus();
      //if (is_ntp_time_sync) {
      //  Serial.print("Internal RTC time: ");
      //  printRTCInternalDate();
      //}

      // I'm sure that the WIFI is active I sync the time with NTP Protocol
      if (current_millis - ntp_previous_millis >= ntp_interval or not is_ntp_time_sync) {
        uint32_t epoch = 0;
        uint8_t attempts = 0;
                
        while (epoch == 0) {
          epoch = WiFi.getTime();
          // Limit
          if (++attempts > 4) {
            break;
          }
        }
        if (epoch == 0) {
          Serial.println("NTP unreachable!");
        } else {
          if (is_ntp_time_sync) {
            // Checking if the internal RTC is still sync
            uint32_t rtc_epoch = rtc.getEpoch();
            if (rtc_epoch != epoch) {
              Serial.print("Internal epoch NOT sync anymore: rtc_epoch != epoch ");
              Serial.print(rtc_epoch);
              Serial.print(" != ");
              Serial.println(epoch);
              is_ntp_time_sync = false;
            } else {
              Serial.print("Internal epoch still sync at ");
              printRTCInternalDate();
            }
          }
          // First epoch set or out of sync..
          if (not is_ntp_time_sync) {
            rtc.setEpoch(epoch);
            is_ntp_time_sync = true; //TODO check if the time is still on sync!
            Serial.print("Epoch set: ");
            Serial.println(epoch);
            Serial.print("Internal RTC time: ");
            printRTCInternalDate();
          }
          ntp_previous_millis = current_millis; // Force NTP check every n minutes
        }
      }
    } else {
      digitalWrite(LED_BUILTIN, HIGH); // No Wifi Connection
      Serial.print("Wifi status: ");
      Serial.println(status);
      if (wifi_first_connection) {
        Serial.print("Attempting to connect to network: ");
      } else {
        Serial.print("Attempting to re-connect to network: ");
      }
      Serial.println(ssid);

      // Connect to WPA/WPA2 network returns the status
      status = WiFi.begin(ssid, pass);
      if (isWifiConnected()) {
        printWiFiStatus();
        wifi_first_connection = false;
      }
    }
  }
}

void printRTCInternalDate() {
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());
  Serial.print(" ");
  Serial.print(rtc.getHours() + GMT);
  Serial.print(":");
  Serial.print(rtc.getMinutes());
  Serial.print(":");
  Serial.print(rtc.getSeconds());
  Serial.println(" ");
}

bool isWifiConnected() {
  return status == WL_CONNECTED;
}

void printWiFiStatus() {
  Serial.println("\n----------------------------------------");
  
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  IPAddress subney_mask = WiFi.subnetMask();
  IPAddress gateway_ip = WiFi.gatewayIP();
  Serial.print("Board IP Address: ");
  Serial.print(ip);
  Serial.print(" Subnet mask: ");
  Serial.print(subney_mask);
  Serial.print(" gateway: ");
  Serial.println(gateway_ip);
  Serial.print("Network ");
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(" signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}
