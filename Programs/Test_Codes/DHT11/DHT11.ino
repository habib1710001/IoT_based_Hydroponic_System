//We'll start by adding our libraries

#include <LiquidCrystal.h>

#include <SimpleDHT.h>

//Declaring digital pin no 6 as the dht11 data pin

int pinDHT11 = 6;
SimpleDHT11 dht11;

//Declaring the lcd pins

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
// Don't forget to choose 9600 at the port screen
  
  Serial.begin(9600);
 
//Telling our lcd to start up
  
  lcd.begin(16, 2);
   
   
}

void loop() {

  //These serial codes are for getting readings on the port screen aswell as the LCD display, since they'll offer us a more detailed interface
  
  
  Serial.println("=================================");
  Serial.println("DHT11 readings...");
  
 
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;

  //This bit will tell our Arduino what to do if there is some sort of an error at getting readings from our sensor
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("No reading , err="); Serial.println(err);delay(1000);
    return;
  }
  
  Serial.print("Readings: ");
  Serial.print((int)temperature); Serial.print(" Celcius, ");
  Serial.print((int)humidity); Serial.println(" %");
 
  //Telling our lcd to refresh itself every 0.75 seconds
  lcd.clear();
 
  //Choosing the first line and row
  lcd.setCursor(0,0);
  //Typing Temp: to the first line starting from the first row
  lcd.print("Temp: ");
  //Typing the temperature readings after "Temp: " 
  lcd.print((int)temperature);
  //Choosing the second line and first row
  lcd.setCursor(0,1);
  //Typing Humidity(%): to the second line starting from the first row
  lcd.print("Humidity(%): ");
  //Typing the humidity readings after "Humidity(%): "
  lcd.print((int)humidity);
 
  
  
  
  delay(750);
}
