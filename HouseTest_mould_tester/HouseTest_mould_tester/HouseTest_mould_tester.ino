/*
 Name:		HouseTest_mould_tester.ino
 Created:	6/12/2017 11:13:06 AM
 Author:	Martin Pihooja 
 Company:   HouseTest ApS
*/

#include <Servo.h>
#include <SdFat.h>
#include "SdFat.h"
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include<avr/interrupt.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity
RTC_DS1307 RTC;
SdFat SD;
File myFile;
Servo servo1;

#define DS1307_I2C_ADDRESS 0x68 // the I2C address of Tiny RTC
#define SD_CS_PIN SS

const int volt_read_Pin = A0; //analog volt read pin
const int buttonPin = 5; //main button input pin
const int air_pump = 6; //air pump output pin
const int blue_LED = 2; //main button LED pin
const int power_LED = 9; //Power LED pin
const int servoPin = 3; //Servomotor pin // juhe otse pinist servosse !!!

int resistor1 = 10000;//Variables for voltage divider R1
int resistor2 = 2200; // R2
float denominator;// voltage devider denominator
int blue_LED_state = HIGH;

int buttonState = 0; //main button state 
int lastButtonState = 0;
int eepromReadValue = 0;

unsigned long previous_button_millis = 0;
const long interval = 1000;


boolean Inside_Test_1_address_eeprom;
boolean Inside_Test_2_address_eeprom;
boolean Outside_Test_1_address_eeprom;
boolean Outside_Test_2_address_eeprom;

boolean Inside_Test_1_status;
boolean Inside_Test_2_status;
boolean Outside_Test_1_status;
boolean Outside_Test_2_status;

void setup()
{
	Wire.begin();

	pinMode(air_pump, OUTPUT);
	pinMode(buttonPin, INPUT);
	pinMode(blue_LED, OUTPUT);
	digitalWrite(blue_LED, HIGH);
	pinMode(volt_read_Pin, INPUT);
	pinMode(power_LED, OUTPUT);
	digitalWrite(power_LED, HIGH);

	//Convert resistor values to division value
	//  Equation is previously mentions voltage divider equation
	//  R2 / (R1 + R2)
	denominator = (float)resistor2 / (resistor1 + resistor2);
	RTC.begin();
	if (!RTC.isrunning()) {
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		RTC.adjust(DateTime(__DATE__, __TIME__));
	}
	DateTime now = RTC.now();
	pinMode(10, OUTPUT);

	myFile = SD.open("HouseTest.txt", FILE_WRITE); //Open or create HouseTest.txt file
	if (myFile) {
		myFile.println("Mould Test information");
		myFile.println("Device started : ");
		myFile.print(now.hour());
		myFile.print(":");
		myFile.print(now.minute());
		myFile.print(":");
		myFile.print(now.second());
		myFile.print("   ");
		myFile.print(now.year());
		myFile.print("-");
		myFile.print(now.month());
		myFile.print("-");
		myFile.println(now.day());
		myFile.close();
	}
	servo1.detach();
	Hello_Text();
}

void loop()
{
	Inside_Test_1_status = EEPROM.get(100, Inside_Test_1_address_eeprom);
	Inside_Test_2_status = EEPROM.get(101, Inside_Test_2_address_eeprom);
	Outside_Test_1_status = EEPROM.get(102, Outside_Test_1_address_eeprom);
	Outside_Test_2_status = EEPROM.get(103, Outside_Test_2_address_eeprom);

	if (Inside_Test_1_status == false)
	{
		Inside_Test_1();
	}
	if (Inside_Test_2_status == false)
	{
		Inside_Test_2();
	}
	if (Outside_Test_1_status == false)
	{
		Outside_Test_1();
	}
	if (Outside_Test_2_status == false)
	{
		Outside_Test_2();
	}
	if (Inside_Test_1_status == true && Inside_Test_2_status == true && Outside_Test_1_status == true && Outside_Test_2_status == true)
	{
		Mould_Test_Completed();
	}
}

void Hello_Text()
{
	lcd.begin(16, 2);
	lcd.setCursor(4, 0);
	lcd.print("HOUSETEST");
	lcd.setCursor(2, 1);
	lcd.print("MOULD TESTER");
	lcd.setCursor(15, 1);
	delay(4000);
}

void Inside_Test_1()
{
	servo1.attach(3);
	servo1.write(68);
	delay(1000);
	servo1.detach();
	lcd.clear();
	lcd.setCursor(1, 0);
	lcd.print("INSIDE TEST 1");
	lcd.setCursor(3, 1);
	lcd.print("PRESS START");
	batterylevel(15, 0);
	boolean buttonPressStatus = false;
	DateTime now = RTC.now();

	while (buttonPressStatus == false)
	{
		buttonState = digitalRead(buttonPin);
		unsigned long currentMillis = millis();

		if (currentMillis - previous_button_millis >= interval) {
			// save the last time you blinked the LED
			previous_button_millis = currentMillis;

			// if the LED is off turn it on and vice-versa:
			if (blue_LED_state == HIGH) {
				blue_LED_state = LOW;
			}
			else {
				blue_LED_state = HIGH;
			}

			// set the LED with the ledState of the variable:
			digitalWrite(blue_LED, blue_LED_state);
		}
		if (buttonState != lastButtonState)
		{
			if (buttonState == HIGH)
			{
				lcd.clear();
				lcd.setCursor(1, 0);
				lcd.print("INSIDE TEST 1");
				batterylevel(15, 0);
				myFile = SD.open("HouseTest.txt", FILE_WRITE); //Open or create HouseTest.txt file
				if (myFile) {
					myFile.println("Inside Test 1 Started : ");
					myFile.print(now.hour());
					myFile.print(":");
					myFile.print(now.minute());
					myFile.print(":");
					myFile.print(now.second());
					myFile.print("   ");
					myFile.print(now.year());
					myFile.print("-");
					myFile.print(now.month());
					myFile.print("-");
					myFile.println(now.day());
					myFile.close();
				}
				else {
					// if the file didn't open, error:
				}
				Turn_pump_on_and_countdown();
				digitalWrite(air_pump, LOW);
				buttonPressStatus = true;
			}
			else
			{
				digitalWrite(air_pump, LOW);
			}
		}
		delay(50);
		lastButtonState = buttonState;

	}

	lcd.clear();
	batterylevel(15, 0);
	lcd.setCursor(1, 0);
	lcd.print("INSIDE TEST 1");
	lcd.setCursor(4, 1);
	lcd.print("FINISHED");
	Inside_Test_1_status = true;
	EEPROM.write(100, true);
	delay(4000);
}

void Inside_Test_2()
{
	servo1.attach(3);
	servo1.write(98);
	delay(1000);
	servo1.detach();
	lcd.clear();
	lcd.setCursor(1, 0);
	lcd.print("INSIDE TEST 2");
	lcd.setCursor(3, 1);
	lcd.print("PRESS START");
	batterylevel(15, 0);
	digitalWrite(power_LED, HIGH);
	DateTime now = RTC.now();
	boolean buttonPressStatus = false;

	while (buttonPressStatus == false)
	{
		buttonState = digitalRead(buttonPin);
		unsigned long currentMillis = millis();

		if (currentMillis - previous_button_millis >= interval) {
			// save the last time you blinked the LED
			previous_button_millis = currentMillis;

			// if the LED is off turn it on and vice-versa:
			if (blue_LED_state == HIGH) {
				blue_LED_state = LOW;
			}
			else {
				blue_LED_state = HIGH;
			}

			// set the LED with the ledState of the variable:
			digitalWrite(blue_LED, blue_LED_state);
		}
		if (buttonState != lastButtonState)
		{
			if (buttonState == HIGH)
			{
				lcd.clear();
				lcd.setCursor(1, 0);
				lcd.print("INSIDE TEST 2");
				batterylevel(15, 0);
				myFile = SD.open("HouseTest.txt", FILE_WRITE); //Open or create HouseTest.txt file
				if (myFile) {
					myFile.println("Inside Test 2 Started : ");
					myFile.print(now.hour());
					myFile.print(":");
					myFile.print(now.minute());
					myFile.print(":");
					myFile.print(now.second());
					myFile.print("   ");
					myFile.print(now.year());
					myFile.print("-");
					myFile.print(now.month());
					myFile.print("-");
					myFile.println(now.day());
					myFile.close();
				}
				else {
					// if the file didn't open, error:
				}
				Turn_pump_on_and_countdown();
				digitalWrite(air_pump, LOW);
				buttonPressStatus = true;
			}
			else
			{
				digitalWrite(air_pump, LOW);
			}
		}
		delay(50);
		lastButtonState = buttonState;

	}

	lcd.clear();
	batterylevel(15, 0);
	lcd.setCursor(1, 0);
	lcd.print("INSIDE TEST 2");
	lcd.setCursor(4, 1);
	lcd.print("FINISHED");
	EEPROM.write(101, true);
	Inside_Test_2_status = true;
	delay(4000);
}

void Outside_Test_1()
{
	servo1.attach(3);
	servo1.write(118);
	delay(1000);
	servo1.detach();
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("OUT TEST 1");
	lcd.setCursor(3, 1);
	lcd.print("PRESS START");
	batterylevel(15, 0);
	digitalWrite(power_LED, HIGH);

	DateTime now = RTC.now();
	boolean buttonPressStatus = false;

	while (buttonPressStatus == false)
	{
		buttonState = digitalRead(buttonPin);
		unsigned long currentMillis = millis();

		if (currentMillis - previous_button_millis >= interval) {
			// save the last time you blinked the LED
			previous_button_millis = currentMillis;

			// if the LED is off turn it on and vice-versa:
			if (blue_LED_state == HIGH) {
				blue_LED_state = LOW;
			}
			else {
				blue_LED_state = HIGH;
			}

			// set the LED with the ledState of the variable:
			digitalWrite(blue_LED, blue_LED_state);
		}
		if (buttonState != lastButtonState)
		{
			if (buttonState == HIGH)
			{
				lcd.clear();
				lcd.setCursor(3, 0);
				lcd.print("OUT TEST 1");
				batterylevel(15, 0);
				myFile = SD.open("HouseTest.txt", FILE_WRITE); //Open or create HouseTest.txt file
				if (myFile) {
					myFile.println("Outside Test 1 Started : ");
					myFile.print(now.hour());
					myFile.print(":");
					myFile.print(now.minute());
					myFile.print(":");
					myFile.print(now.second());
					myFile.print("   ");
					myFile.print(now.year());
					myFile.print("-");
					myFile.print(now.month());
					myFile.print("-");
					myFile.println(now.day());
					myFile.close();
				}
				else {
					// if the file didn't open, error:
				}
				Turn_pump_on_and_countdown();
				digitalWrite(air_pump, LOW);
				buttonPressStatus = true;
			}
			else
			{
				digitalWrite(air_pump, LOW);
			}
		}
		delay(50);
		lastButtonState = buttonState;

	}

	lcd.clear();
	batterylevel(15, 0);
	lcd.setCursor(3, 0);
	lcd.print("OUT TEST 1");
	lcd.setCursor(4, 1);
	lcd.print("FINISHED");
	EEPROM.write(102, true);
	Outside_Test_1_status = true;
	delay(5000);
}

void Outside_Test_2()
{
	servo1.attach(3);
	servo1.write(140);
	delay(1000);
	servo1.detach();
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("OUT TEST 2");
	lcd.setCursor(3, 1);
	lcd.print("PRESS START");
	batterylevel(15, 0);
	digitalWrite(power_LED, HIGH);

	DateTime now = RTC.now();
	boolean buttonPressStatus = false;

	while (buttonPressStatus == false)
	{
		buttonState = digitalRead(buttonPin);
		unsigned long currentMillis = millis();

		if (currentMillis - previous_button_millis >= interval) {
			// save the last time you blinked the LED
			previous_button_millis = currentMillis;

			// if the LED is off turn it on and vice-versa:
			if (blue_LED_state == HIGH) {
				blue_LED_state = LOW;
			}
			else {
				blue_LED_state = HIGH;
			}

			// set the LED with the ledState of the variable:
			digitalWrite(blue_LED, blue_LED_state);
		}
		if (buttonState != lastButtonState)
		{
			if (buttonState == HIGH)
			{
				lcd.clear();
				lcd.setCursor(3, 0);
				lcd.print("OUT TEST 2");
				batterylevel(15, 0);
				myFile = SD.open("HouseTest.txt", FILE_WRITE); //Open or create HouseTest.txt file
				if (myFile) {
					myFile.println("Outside test 2 Started : ");
					myFile.print(now.hour());
					myFile.print(":");
					myFile.print(now.minute());
					myFile.print(":");
					myFile.print(now.second());
					myFile.print("   ");
					myFile.print(now.year());
					myFile.print("-");
					myFile.print(now.month());
					myFile.print("-");
					myFile.println(now.day());
					myFile.close();
				}
				else {
					// if the file didn't open, error:
				}
				Turn_pump_on_and_countdown();
				digitalWrite(air_pump, LOW);
				buttonPressStatus = true;
			}
			else
			{
				digitalWrite(air_pump, LOW);
			}
		}
		delay(50);
		lastButtonState = buttonState;

	}

	lcd.clear();
	batterylevel(15, 0);
	lcd.setCursor(3, 0);
	lcd.print("OUT TEST 2");
	lcd.setCursor(4, 1);
	lcd.print("FINISHED");
	EEPROM.write(103, true);
	Outside_Test_2_status = true;
	delay(4000);

}

void Mould_Test_Completed()
{
	EEPROM.write(100, false);
	EEPROM.write(101, false);
	EEPROM.write(102, false);
	EEPROM.write(103, false);

	DateTime now = RTC.now();
	servo1.attach(3);
	servo1.write(68);
	delay(1500);

	digitalWrite(power_LED, HIGH);
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("GOOD JOB");
	lcd.setCursor(1, 1);
	lcd.print("TEST FINISHED");
	batterylevel(15, 0);
	servo1.detach();
	myFile = SD.open("HouseTest.txt", FILE_WRITE); //Open or create HouseTest.txt file

	if (myFile) {
		myFile.println("Test Finished : ");
		myFile.print(now.hour());
		myFile.print(":");
		myFile.print(now.minute());
		myFile.print(":");
		myFile.print(now.second());
		myFile.print("   ");
		myFile.print(now.year());
		myFile.print("-");
		myFile.print(now.month());
		myFile.print("-");
		myFile.println(now.day());
		myFile.close();
	}
	delay(5000);
	lcd.clear();
	//lcd.noBacklight();
	for (;;)
	{
		batterylevel(15, 0);
		DateTime now = RTC.now();
		lcd.setCursor(2, 0);
		lcd.print("CURRENT TIME");
		lcd.setCursor(4, 1);

		if (now.hour()<10)
		{
			lcd.print('0');
		}
		lcd.print(now.hour());
		lcd.print(':');

		if (now.minute()<10)
		{
			lcd.print('0');
		}
		lcd.print(now.minute());
		lcd.print(':');

		if (now.second()<10)
		{
			lcd.print('0');
		}
		lcd.print(now.second());

	}
}
void batterylevel(int xpos, int ypos)
{

	// display näitab 0.4 madalamat väärtust kui tegelikult 
	double voltage;
	//Obtain RAW voltage data
	voltage = analogRead(volt_read_Pin);
	//Convert to actual voltage (0 - 5 Vdc)
	voltage = (voltage / 1024) * 5.0;
	// Convert to voltage before divider
	//  Divide by divider = multiply
	//  Divide by 1/5 = multiply by 5
	voltage = voltage / denominator;

	if (voltage > 14)
	{
		byte batlevel[10] = {
			B01110,
			B11111,
			B10101,
			B10001,
			B11011,
			B11011,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage <= 14 && voltage > 11.3)
	{
		byte batlevel[] = {
			B01110,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage <= 11.3 && voltage > 11)
	{
		byte batlevel[8] = {
			B01110,
			B10001,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage <= 11 && voltage > 10.5)
	{
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B11111,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage <= 10.5 && voltage > 10)
	{
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B11111,
			B11111,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage <= 10 && voltage > 9.5)
	{
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B10001,
			B11111,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage <= 9.5 && voltage > 9)
	{
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B10001,
			B10001,
			B11111,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
	if (voltage < 9)
	{
		byte batlevel[8] = {
			B01110,
			B10001,
			B10001,
			B10001,
			B10001,
			B10001,
			B10001,
			B11111,
		};
		lcd.createChar(0, batlevel);
		lcd.setCursor(xpos, ypos);
		lcd.write(byte(0));
	}
}
void Turn_pump_on_and_countdown()
{
	int hoursleft = 0;
	int minleft = 0;
	int secleft = 0;
	digitalWrite(blue_LED_state, HIGH);

	for (int counter = 10; counter > 0; counter--)
	{

		batterylevel(15, 0);
		hoursleft = counter / 60 / 60;
		minleft = counter / 60 - hoursleft * 60;
		secleft = counter - minleft * 60 - hoursleft * 60 * 60;
		lcd.setCursor(4, 1);
		if (hoursleft<10)
		{
			lcd.print('0');
		}
		lcd.print(hoursleft);
		lcd.print(":");
		if (minleft<10)
		{
			lcd.print('0');
		}
		lcd.print(minleft);
		lcd.print(":");
		if (secleft<10)
		{
			lcd.print('0');
		}
		lcd.print(secleft);
		digitalWrite(air_pump, HIGH);
		delay(1000);
	}
}

