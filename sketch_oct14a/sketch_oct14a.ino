#include <ThingSpeak.h>
#include<stdlib.h>
#include <SoftwareSerial.h>       //Software Serial library
#include<LiquidCrystal.h>
#include <EEPROM.h>
#include "GravityTDS.h"
#include "DHTStable.h"
DHTStable DHT;

#define CarbonMonoxide A1          //another analog pin
#define dht_apin A0                 //analog pin
#define moist_pin A5                //analog pin for moisture sensor
#define TdsSensorPin A3


int soilMoistureValue = 0;


GravityTDS gravityTds;
int buzzer = 12;
int sensorthres = 100;

SoftwareSerial espSerial(9, 10);   //Pin 9 and 10 act as RX and TX. Connect them to TX and RX of ESP8266   
LiquidCrystal lcd(2,3,4,5,6,7);           //lcd pins: 2-rs, 3-en, 4-7: data: D4-D7

char * messagePadded = "IoT Based Environment monitor System";

#define DEBUG true
String mySSID = "JioFiber 4G";       // WiFi SSID
String myPWD = "Leomessi23";       // WiFi Password
String myAPI = "26MSFW9UQP2GCMLI";   // API Key
unsigned long Channel_ID = 1874530;
String myHOST = "api.thingspeak.com";
String myPORT = "80";
String myFIELD_1 = "field1"; 
String myFIELD_2 = "field2"; 
String myFIELD_3 = "field3"; 
String myFIELD_4 = "field4"; 
String myFIELD_5 = "field5"; 
String myFIELD_6 = "field6";

int sendVal;
int sensorValue = 0;
int carbonMonoxideValue = 0;
int temperature;
int humidity =0;
int moisture;
float tdsValue = 0;

void setup()
{
  Serial.begin(115200);
  espSerial.begin(9600);

  pinMode(buzzer, OUTPUT); // Set buzzer - pin 12 as an output

  lcd.begin(16,2);                       //matrix 
  //StartUpMessage();                      //function call

    gravityTds.setPin(TdsSensorPin);
    gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds.begin();  //initialization

  espData("AT+RST", 1000, DEBUG);                      //Reset the ESP8266 module
  espData("AT+CWMODE=3", 1000, DEBUG);                 //Set the ESP mode as station mode
  espData("AT+CWJAP=\""+ mySSID +"\",\""+ myPWD +"\"", 1000, DEBUG);   //Connect to WiFi network
 /* while(!esp.find("OK")) 
  {          
      Wait for connection
  }*/
  delay(1000);
  //SetParameters();                       //function call
}
  String espData(String command, const int timeout, boolean debug)
{
  Serial.print("AT Command ==> ");
  Serial.print(command);
  Serial.println("     ");
  
  String response = "";
  espSerial.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (espSerial.available())
    {
      char c = espSerial.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);

  }
  return response;
}

  void loop()
  {
    /* Here, I'm using the function random(range) to send a random value to the 
     ThingSpeak API. You can change this value to any sensor data
     so that the API will show the sensor data  
    */
    for (int letter = 0; letter <= strlen(messagePadded) - 16; letter++) //From 0 to upto n-16 characters supply to below function
    {
      showLetters(0, letter);
    }

    DHT.read11(dht_apin);  //reading temperature and humidity from A0 pin: DHT11
    temperature = DHT.getTemperature();
    humidity = DHT.getHumidity();

    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value

    sensorValue = analogRead(CarbonMonoxide);   //reading CO from A1 pin: MQ7
    carbonMonoxideValue = map(sensorValue,0,1023,0,255);   //mapping the value: 10 bit to 8 bit

    if (carbonMonoxideValue > sensorthres)
    {
      tone(buzzer, 10, 100000);
    }
    else
    {
      noTone(buzzer); 
    }

   soilMoistureValue = analogRead(moist_pin);  //put Sensor insert into soil
   //float moisture;
   int op;
   op = analogRead(moist_pin);
   Serial.println(op);
   moisture = ( 100 - ( (op/1023.00) * 100) );

    sendVal = random(1000); // Send a random number between 1 and 1000
    String sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD_1 +"="+String(sendVal);
    sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD_2 +"="+String(temperature);
    sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD_3 +"="+String(humidity);
    sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD_4 +"="+String(carbonMonoxideValue);
    sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD_5 +"="+String(moisture);
    sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD_6 +"="+String(tdsValue);

     


    espData("AT+CIPMUX=1", 1000, DEBUG);       //Allow multiple connections
    espData("AT+CIPSTART=0,\"TCP\",\""+ myHOST +"\","+ myPORT, 1000, DEBUG);
    espData("AT+CIPSEND=0," +String(sendData.length()+20),1000,DEBUG);  
    espSerial.find(">"); 
    espSerial.println(sendData);
    Serial.print("Value to be sent: ");
    Serial.println(sendVal);
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("CO: ");
    Serial.println(carbonMonoxideValue);
    Serial.println(soilMoistureValue);
    Serial.print("Moisture Percentage = ");
    Serial.println(moisture);
    Serial.print("TDS value = ");
    Serial.print(tdsValue,0);
    Serial.println("ppm");
    
     
    espData("AT+CIPCLOSE=0",1000,DEBUG);
    //DisplayParameters();
    SetParameters();                       //function call
    DisplayParameters();
    delay(10000);
  }


/*void StartUpMessage()
{
  lcd.clear();
  lcd.setCursor(0,0);   //init to row=0, col=0
  lcd.print("IOT WEATHER & PO");
  lcd.println("LLUTION MTR S/M");
}*/

void SetParameters()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.setCursor(10,0);
  lcd.print("H:");
  lcd.setCursor(0,1);
  lcd.print("CO:");
  lcd.setCursor(6,1);
  lcd.print("M:");
  lcd.setCursor(10, 1);
  lcd.print("t:");
  delay(100);
}
void DisplayParameters()
{
  lcd.setCursor(2,0);
  lcd.print(DHT.getTemperature());
  lcd.setCursor(12,0);
  lcd.print(DHT.getHumidity());
  lcd.setCursor(3,1);
  lcd.print(carbonMonoxideValue);
  lcd.setCursor(8,1);
  lcd.print(moisture);
  lcd.setCursor(12, 1);
  lcd.print(tdsValue,0);
}
void showLetters(int printStart, int startLetter)
{
  lcd.setCursor(0, 0);
  for (int letter = startLetter; letter <= startLetter + 15; letter++) // Print only 16 chars in Line #1 starting 'startLetter'
  {
    lcd.print(messagePadded[letter]);
  }
  delay(350);
  lcd.clear();
}
/*
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.setCursor(10,0);
  lcd.print("H:");
  lcd.setCursor(0,1);
  lcd.print("CO:");
  lcd.setCursor(7,1);
  lcd.print("Moist:");
}*/

  