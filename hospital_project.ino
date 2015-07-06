/*************
Author: Benjamin Low
Date: Apr 2015
Description: A simple communication device for people with locked in syndrome like Stephen Hawking.
Green and red LEDs represent "yes" and "no". A button device is held by the user. A short press
means "yes" and a longer press means "no". The threshold time for determining this can be set by
turning a knob to a value of between 200 ms and 2 seconds. An LCD screen shows the threshold time
and the time held when the user is pressing the switch. A Teensy 3.1 is used as the microcontroller.
*************/

#include <SoftwareSerial.h>

SoftwareSerial LCD = SoftwareSerial(0, 8);

const int red_led = 2, green_led = 1, switch_pin = 0;
unsigned int threshold_time = 500, prev_threshold_time;
const int led_lit_time = 1000;
const int screen_timeout = 10000;
unsigned long pressed_time, released_time, red_led_on_time, green_led_on_time, held_duration, calibrated_time;
boolean has_been_pressed, has_been_released, is_red_led_lit, is_green_led_lit, is_red_registered, is_green_registered, is_held_duration_displayed;

void setup() {
  Serial.begin(9600);
  LCD.begin(9600);
  pinMode(switch_pin, INPUT_PULLUP); //active low
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  has_been_pressed = false;
  has_been_released = false;
  is_red_led_lit = false;
  is_green_led_lit = false;
  is_green_registered = false;
  is_red_registered = false;
  backlightOn();
  delay(100);
  clearLCD();
  delay(100);
}

void loop() {

  //int button_value = !digitalRead(switch_pin); //for the substitute switch
  int button_value = digitalRead(switch_pin); //for the proper switch

  int pot_value = analogRead(A0);

  threshold_time = map(pot_value, 0, 1023, 2000, 200);
  threshold_time /= 100;
  threshold_time *= 100; //round to nearest 100;
  
  if (threshold_time != prev_threshold_time) { //whenever knob is turned
           //clearLCD();
           calibrated_time = millis();     
         backlightOn();  
  } else if (millis() - calibrated_time > screen_timeout) {
          backlightOff();   
  }
  
  prev_threshold_time = threshold_time;

  selectLineOne();
  LCD.print("thres: ");
  LCD.print(threshold_time);
  LCD.print("  ");
  Serial.println(threshold_time);

  if (button_value == LOW) { //pressed

    if (has_been_pressed == false) { //done on first press only
      has_been_pressed = true;
      is_red_registered = false;
      has_been_released = false;
      pressed_time = millis();
      is_held_duration_displayed = true;
    }

    if (millis() - pressed_time > threshold_time && is_red_registered == false && is_red_led_lit == false) { //light the red led
      is_red_led_lit = true;
      red_led_on_time = millis();
      digitalWrite(red_led, HIGH);
      Serial.println("red activated");
    }
    
    held_duration = millis() - pressed_time;

  }

  else if (button_value == HIGH) { //released

    if (has_been_released == false) { //done on first release only
      has_been_released = true;
      has_been_pressed = false;
      released_time = millis();
      is_green_registered = false;
    }

    if (released_time - pressed_time < threshold_time && is_green_registered == false && is_green_led_lit == false && is_red_led_lit == false) { //light the green led
      is_green_led_lit = true;
      green_led_on_time = millis();
      digitalWrite(green_led, HIGH);
      Serial.println("green activated");
    }
  }

  if (millis() - red_led_on_time > led_lit_time && is_red_led_lit == true) {
    digitalWrite(red_led, LOW);
    is_red_led_lit = false;
    is_red_registered = true;
    is_held_duration_displayed = false;
    clearLCD();
  }
  
  if (millis() - green_led_on_time > led_lit_time && is_green_led_lit == true) {
    digitalWrite(green_led, LOW);
    is_green_led_lit = false;
    is_green_registered = true;
    is_held_duration_displayed = false;
    clearLCD();
  }
  
   if (is_held_duration_displayed == true) {
           display_held_duration();     
   }  
  
  delay(10); //to prevent blank screen
}

void display_held_duration() {
    selectLineTwo();
    LCD.print("held:  ");
    LCD.print(held_duration);
    LCD.print("  ");
}

void selectLineOne() { //puts the cursor at line 0 char 0.
  LCD.write(0xFE);   //command flag
  LCD.write(128);    //position
  delay(10);
}
void selectLineTwo() { //puts the cursor at line 0 char 0.
  LCD.write(0xFE);   //command flag
  LCD.write(192);    //position
  delay(10);
}
void goTo(int position) { //position = line 1: 0-15, line 2: 16-31, 31+ defaults back to 0
  if (position < 16) {
    LCD.write(0xFE);   //command flag
    LCD.write((position + 128));  //position
  } else if (position < 32) {
    LCD.write(0xFE);   //command flag
    LCD.write((position + 48 + 128)); //position
  } else {
    goTo(0);
  }
  delay(10);
}

void clearLCD() {
  LCD.write(0xFE);   //command flag
  LCD.write(0x01);   //clear command.
  delay(10);
}

void backlightOn() { //turns on the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(157);    //full light level
  //LCD.write(142);
  delay(10);
}

void backlightOff() { //turns off the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(128);     //light level for off.
  delay(10);
}

void backlightDim() {
   LCD.write(0x7C);
   LCD.write(138);
   delay(10);   
}

void serCommand() {  //a general function to call the command flag for issuing all other commands
  LCD.write(0xFE);
}




