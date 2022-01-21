////////
// Arduino Alarm
// Components: Arduino Leonardo, Real Time Clock RS3231, piezo buzzer, OLED I2C screen 128x64
// Features:
//  - Shows the time on the Screen
//  - Piezo sounds when the alam is triggered
//  - Temperature provided by DS3231 shown on the screen

//Real time clock 
#include <Wire.h>
#include "ds3231.h"

// Oled screen
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO

const int piezoPin = 8;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Alarm time
uint8_t wake_HOUR = 6;
uint8_t wake_MINUTE = 46;
uint8_t wake_SECOND = 00;


// how often to refresh the time ms
unsigned long prev = 1000;
unsigned long prev_sound = 0;
unsigned long interval = 1000;
bool play_alarm = false;

bool display_inverted = false;

void set_alarm(void)
{

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm - see datasheet
    // A1M1 (seconds) (0 to enable, 1 to disable)
    // A1M2 (minutes) (0 to enable, 1 to disable)
    // A1M3 (hour)    (0 to enable, 1 to disable) 
    // A1M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[5] = { 0, 0, 0, 1, 1 };

    // set Alarm1
    DS3231_set_a1(wake_SECOND, wake_MINUTE, wake_HOUR, 0, flags);

    // activate Alarm1
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A1IE);
}

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    DS3231_init(DS3231_CONTROL_INTCN);
    DS3231_clear_a1f();
    set_alarm();

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);


    display.begin(i2c_Address, true); // Address 0x3C default
    //display.setContrast (0); // dim display
    // Clear the buffer.
    display.clearDisplay();
    // text display tests
//    display.setTextSize(1);
//    display.setTextColor(SH110X_WHITE);
//    display.setCursor(0, 0);
//    display.println("Failure is always an option");
//    display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
//    display.println(3.141592);
//    display.setTextSize(2);
//    display.setTextColor(SH110X_WHITE);
//    display.println("");
//    display.print("BUON NATALE"); 
//    //display.println(0xDEADBEEF, HEX);
//    display.display();
//    delay(2000);
//    display.clearDisplay();
    display.setContrast(0); // dim display
    //display.setContrast(255); // dim display
}

void loop()
{
    // Real time clock
    //char buff[BUFF_MAX];
    const unsigned long now = millis();
    struct ts date;

    // once a while show what is going on
    if ((now - prev > interval) && (Serial.available() <= 0)) {
        prev = now;
        DS3231_get(&date);
        float temperature = DS3231_get_treg();

        // display current time
        //snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year,
        //     t.mon, t.mday, t.hour, t.min, t.sec);
        //Serial.println(buff);

        String day_of_the_week = ""; 
        switch (date.wday) {
          case 1:
              day_of_the_week = "Sunday";
              break;
          case 2:
              day_of_the_week = "Monday";
              break;
          case 3:
              day_of_the_week = "Tuesday";
              break;
          case 4:
              day_of_the_week = "Wednesday";
              break;
          case 5:
              day_of_the_week = "Thurday";
              break;
          case 6:
              day_of_the_week = "Friday";
              break;
          case 7:
              day_of_the_week = "Saturday";
              break;
              
          default:
              day_of_the_week = "Unknown";
              break;
        }

        /////////////////////
        // Serial interface
        Serial.print("Day of the week: ");
        Serial.println(day_of_the_week);


        Serial.print("Date : ");
        Serial.print(date.mday);
        Serial.print("/");
        Serial.print(date.mon);
        Serial.print("/");
        Serial.print(date.year);
        Serial.print("\t Hour : ");
        Serial.print(date.hour);
        Serial.print(":");
        Serial.print(date.min);
        Serial.print(".");
        Serial.print(date.sec);
        Serial.print(" temp: ");
        Serial.print(temperature);
        Serial.println(" C");

        /////////////////
        //Display
     
        display.clearDisplay();
        display.setCursor(0, 0);

        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);

        // Print current date
        if (date.mday < 10) {
           display.print("0");
        }
        display.print(date.mday);
        display.print("/");
        if (date.mon < 10) {
           display.print("0");
        }
        display.print(date.mon);
        display.print("/");
        display.print(date.year);
        //display.print("\n");

        // Print day of the week
        display.print("  ");
        display.print(day_of_the_week);


        //Print time
        display.setTextSize(2);
        display.setTextColor(SH110X_WHITE);

        display.print("  ");
        if (date.hour < 10) {
           display.print("0");
        }
        display.print(date.hour);
        display.print(":");
        if (date.min < 10) {
           display.print("0");
        }
        display.print(date.min);
        display.print(":");
        if (date.sec < 10) {
           display.print("0");
        }
        display.print(date.sec);

        display.print("\n");
        display.setTextSize(1);
        display.print("\n");
        
        // Alarm time
        display.print("Alarm: ");
        if (wake_HOUR < 10) {
           display.print("0");
        }
        display.print(wake_HOUR);
        display.print(":");
        if (wake_MINUTE < 10) {
           display.print("0");
        }
        display.print(wake_MINUTE);
        display.print(":");
        if (wake_SECOND < 10) {
           display.print("0");
        }
        display.print(wake_SECOND);      
        
        display.print("\n");
        
        // Temperature display
        display.print("Temperature: ");
        display.print(temperature);
        display.print(" C");
        
        display.display();

        if (DS3231_triggered_a1()) {
            // INT has been pulled low
            Serial.println(" -> alarm1 has been triggered");
            play_alarm = true;
            // clear a1 alarm flag and let INT go into hi-z
            DS3231_clear_a1f();
        }
    }
    
    if (play_alarm) {
      if ((now - prev_sound > 1000)) {
        prev_sound = now;
        tone(piezoPin, 500, 500); // async
        display_inverted = not display_inverted;
        display.invertDisplay(display_inverted);
      }
    }
}
