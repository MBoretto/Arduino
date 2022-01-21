//https://docs.arduino.cc/tutorials/mkr-1000-wifi/mkr-1000-connect-to-wifi
#include <SPI.h>
#include <WiFi101.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <TimeLib.h> // unixx time manipualtions

WiFiUDP ntpUDP;

#include "arduino_secrets.h"
const char ssid[] = SECRET_SSID;        // your network SSID (name)
const char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//Real time clock 
#include <Wire.h>
#include "ds3231.h"



// By default 'pool.ntp.org' is used with 60 seconds update interval and
// Setting time zone +1
NTPClient timeClient(ntpUDP, +3600);



// You can specify the time server pool and the offset, (in seconds)
// additionally you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);


void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

  // attempt to connect to Wifi network:
  WiFi.hostname("MKR1000");
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");

  timeClient.begin();
}

void printData() {
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);
  DS3231_clear_a1f();


  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

}

void loop() {
  struct ts date;
  char buff[256];

  // First network time
  if (not timeClient.update()) {
    // Cannot get the time skipping
    return;
  } 

  DS3231_get(&date);
  // display current time
  snprintf(buff, 256, "%d.%02d.%02d %02d:%02d:%02d", date.year,
       date.mon, date.mday, date.hour, date.min, date.sec);
  
  Serial.println(buff); 
  Serial.println(timeClient.getFormattedTime());
  Serial.println("Epoch time:");
  long unix_time_clock = (long) get_unixtime(date);
  long unix_time_network = timeClient.getEpochTime();
  Serial.println(unix_time_clock);
  Serial.println(unix_time_network);
  long delta = 0;
  if (unix_time_network >= unix_time_clock) {
    delta = unix_time_network - unix_time_clock;
  } else {
    delta = unix_time_clock - unix_time_network;
  }

  Serial.println("difference: "); 
  Serial.print(delta);
  Serial.println(" s"); 
  if (delta > 0) {
    Serial.println("Setting Network time.."); 
    setTime(unix_time_network);
    Serial.println("Network time set!!!!!"); 
  }
  
  Serial.println("----");
  // check the network time connection once every 10 seconds:
  delay(10000);
}


void setTime(long unix_timestamp) {  
  ts new_time;  
  new_time.hour = hour(unix_timestamp); 
  new_time.min = minute(unix_timestamp);
  new_time.sec = second(unix_timestamp);
  new_time.wday = weekday(unix_timestamp);         // day of the week (1-7), Sunday is day 1
  new_time.mday = day(unix_timestamp);
  new_time.mon = month(unix_timestamp);
  new_time.year = year(unix_timestamp);


//int16_t year, uint8_t mon, uint8_t mday, uint8_t hour, uint8_t min, uint8_t sec
//  
//  ts new_time;  
//  new_time.hour = hour; 
//  new_time.min = min;
//  new_time.sec = sec;
//  new_time.mday = mday;
//  new_time.mon = mon;
//  new_time.year = year;
 
  DS3231_set(new_time);  
}
