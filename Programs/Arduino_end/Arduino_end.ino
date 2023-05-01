#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
#include "dht.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#define dht_apin A5 // Analog Pin sensor is connected to

 RTC_DS1307 rtc;
 //for lcd display with I2C module:
 LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address, if it's not working try 0x27.set the LCD address to 0x3F for a 20 chars and 4 line display
 dht DHT;
 
 //EC Input and Calculation Variables: 
 int R1= 1000;
 int Ra=25; //Resistance of powering Pins
 int ECPin= A0;
 int ECGround=A1; 
 int ECPower =A4;
 float PPMconversion=0.64;
 float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
 float K=1.4015;
 
//Temp Probe Related pins:
#define ONE_WIRE_BUS 10          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive =8;  //Temp Probe power connected to pin 9
const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
//EC and temperature output variables: 
float Temperature=10;
float EC=0;
float EC25 =0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;

//DHT11 Output variables:
float airhum;
float airtemp;

//Time measuring variables:
int hr;
int mint;

//pH Variables:
float calibration =19.97; //change this value to calibrate
const int pHpin = A6; //ph sensor pin.
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10];
int temp;

//Float sensor pin
int fltsensorpin = 30;

//Actuators variables:
int pHdown = 22;
int pHup = 23;
int wtpump = 24;
int nutriApump = 25;
int nutriBpump = 26;
int rgb = 27 ;
int solenoid = 28;
int acpump = 29;


//Timer and logical variables:
int mintflag;
int flag;
int minlevel;

unsigned long lasttime;

int key1 = 0;
int flag1 = 1;
unsigned long timepick1;
unsigned long timepick1a;

int key2 = 0;
int flag2 = 1;
unsigned long timepick2;
unsigned long timepick2a;

int key3 = 0;
int flag3 = 1;
unsigned long timepick3;
unsigned long timepick3a;

int key4 = 0;
int flag4 = 1;
unsigned long timepick4;
unsigned long timepick4a;




void setup()
 {
   Serial.begin(9600);
   Serial1.begin(9600);

   Wire.begin();
   
   lcd.begin(20,4);   // iInit the LCD for 16 chars 2 lines
   lcd.backlight();   // Turn on the backligt (try lcd.noBaklight() to turn it off)
   lcd.setCursor(0,0); //First line
   lcd.print("HYDROPONICS");
   delay(1000);
   lcd.clear();

   //Actators State:
   pinMode(pHdown , OUTPUT);
   pinMode(pHup , OUTPUT);
   pinMode(wtpump , OUTPUT);
   pinMode(nutriApump , OUTPUT);
   pinMode(nutriBpump , OUTPUT);
   pinMode(rgb , OUTPUT);
   pinMode(solenoid , OUTPUT);
   pinMode(acpump , OUTPUT);

   //Digital Flaot sensor:
   pinMode(fltsensorpin, INPUT_PULLUP);


   //For EC and Tempearture Sensor:
   pinMode(TempProbeNegative , OUTPUT ); //seting ground pin as output for tmp probe
   digitalWrite(TempProbeNegative , LOW );//Seting it to ground so it can sink current
   pinMode(TempProbePossitive , OUTPUT );//ditto but for positive
   digitalWrite(TempProbePossitive , HIGH );
   pinMode(ECPin,INPUT);
   pinMode(ECPower,OUTPUT);//Setting pin for sourcing current
   pinMode(ECGround,OUTPUT);//setting pin for sinking current
   digitalWrite(ECGround,LOW);//We can leave the ground connected permanantly
 
   delay(100);// gives sensor time to settle
   sensors.begin();
   delay(100);
   
   //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
   // Consule Read-Me for Why, or just accept it as true
   R1=(R1+Ra);// Taking into account Powering Pin Resitance

   if (! rtc.begin()) 
   {
    Serial.println("Couldn't find RTC");
    while (1);
   }
  

  if (rtc.isrunning()) 
    {
    Serial.println("RTC lost power, lets set the time!");
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
  
  // If you need to set the time of the uncomment line 34 or 37
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
 }

void loop() 
{
  //Air temp and Air Humidity Measurement:
  DHT.read11(dht_apin);
    
  Serial.print("Air humidity = ");
  Serial.print(DHT.humidity);
  airhum = DHT.humidity;
  Serial.print("%  ");
  Serial.print("Air Temperature = ");
  Serial.print(DHT.temperature); 
  airtemp = DHT.temperature; 
  Serial.println(" C  ");

  //pH Measurement:
  for(int i=0;i<10;i++) 
  { 
  buf[i]=analogRead(pHpin);
  delay(30);
  }
  for(int i=0;i<9;i++)
  {
   for(int j=i+1;j<10;j++)
   {
    if(buf[i]>buf[j])
    {
     temp=buf[i];
     buf[i]=buf[j];
     buf[j]=temp;
    }
   }
  }
  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];

  //calibration needed hear:
  float pHVol=(float)avgValue*5.0/1024/6;//need to calibrated
  float phi = (-4.57 * pHVol)+ calibration;
 
  Serial.print("pH value = ");
  Serial.print(phi);
  Serial.print("  Signal Voltage = ");
  Serial.print(pHVol);
  Serial.println(" Volt");


  //Water Temperature Measurement:
  sensors.requestTemperatures();// Send the command to get temperatures
  Temperature=sensors.getTempCByIndex(0); //Stores Value in Variable


  //Water EC Measurement:
  //Estimates Resistance of Liquid
  digitalWrite(ECPower,HIGH);
  raw= analogRead(ECPin);
  raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(ECPower,LOW);
 
  //Converts to EC
  Vdrop= (Vin*raw)/1024.0;
  Rc=(Vdrop*R1)/(Vin-Vdrop);
  Rc=Rc-Ra; //acounting for Digital Pin Resitance
  EC = 1000/(Rc*K);

 //Compensating For Temperature
  EC25  =  EC/ (1+ TemperatureCoef*(Temperature-25.0));
  ppm=(EC25)*(PPMconversion*1000);

   Serial.print("Rc: ");
   Serial.print(Rc);
   Serial.print(" EC: ");
   Serial.print(EC25);
   Serial.print(" Simens  ");
   Serial.print(" Strength :  ");
   Serial.print(ppm);
   Serial.print(" ppm ");
   Serial.print(" Water temperature: ");
   Serial.print(Temperature);
   Serial.println(" *C ");


      //LCD Print:

       lcd.setCursor(0 , 0);
       lcd.print("pH:");
       lcd.setCursor(8 , 0);
       lcd.print("EC:");
       lcd.setCursor(0 , 1);
       lcd.print("AirTemp:");
       lcd.setCursor(0 , 2);
       lcd.print("WaterTemp:");
       lcd.setCursor(0 , 3);
       lcd.print("AirHum:");

       lcd.setCursor(4, 0);
       lcd.print(phi);
       lcd.setCursor(11 , 0);
       lcd.print(EC25);
       lcd.setCursor(10 , 1);
       lcd.print(airtemp);
       lcd.setCursor(11 , 2);
       lcd.print(Temperature);
       lcd.setCursor(8 , 3);
       lcd.print(airhum);
               

      //DATA FETCHING To the NodeMCU:
      int sen1= airhum;
      int sen2= airtemp;
      int sen3= phi;
      int sen4=  EC25;
      int sen5= Temperature;
      
      
      //Sending data to the NodeMCU:
      int data1=sen1;
      if(Serial1.available()>0)
      {
       char c=Serial1.read();
       if(c=='s')
        {
         Serial1.print(data1);        
         Serial1.print('\n');
         Serial1.flush();
        } 
      }

      int data2=sen2;
      if(Serial1.available()>0)
      {
       char p=Serial1.read();
       if(p=='k')
        {
         Serial1.print(data2);        
         Serial1.print('\n');
         Serial1.flush();
        } 
      }

      int data3=sen3;
      if(Serial1.available()>0)
      {
       char m=Serial1.read();
       if(m=='l')
        {
         Serial1.print(data3);        
         Serial1.print('\n');
         Serial1.flush();
        } 
      }

      int data4=sen4;
      if(Serial1.available()>0)
      {
       char n=Serial1.read();
       if(n=='j')
        {
         Serial1.print(data4);        
         Serial1.print('\n');
         Serial1.flush();
        } 
      }

      int data5=sen5;
      if(Serial1.available()>0)
      {
       char o=Serial1.read();
       if(o=='y')
        {
         Serial1.print(data5);        
         Serial1.print('\n');
         Serial1.flush();
        } 
      }

  //Logics for pH Up Tank:
   if( key1 == 0 )
   {
    if( phi <= 5.5 )
      { 
       if(flag1 == 1)
        {
         Serial.println("Time picked for pH Up Tank");
         timepick1 = millis();
         flag1 = 0;
        }
        key1 = 1;
      }
   } 

    if( key1 == 1 )
      {
         if( millis() < timepick1 + 10000UL )//Time needed to provide 10ml 
           {
            digitalWrite(pHup , HIGH);
            timepick1a = millis();
           }
         else
           {
            digitalWrite(pHup , LOW);
            if( millis() > timepick1a + 600000UL )//Time for waiting 10 minutes
             { 
               key1 = 0;
             }
            flag1 = 1;
           }   
      }

   //Logics for pH Down Tank:
   if( key2 == 0 )
   {
    if( phi >= 6.5 )
      { 
       if(flag2 == 1)
        {
         Serial.println("Time picked for pH Down Tank");
         timepick2 = millis();
         flag2 = 0;
        }
        key2 = 1;
      }
   } 

    if( key2 == 1 )
      {
         if( millis() < timepick2 + 10000UL ) //Time needed to provide 10ml
           {
            digitalWrite(pHdown , HIGH);
            timepick2a = millis();
           }
         else
           {
            digitalWrite(pHdown , LOW);
            if( millis() > timepick2a + 600000UL ) //Time for waiting 10 minutes
             { 
               key2 = 0;
             }
            flag2 = 1;
           }   
      } 
 
   //Logics for EC up Tank:
   if( key3 == 0 )
   {
    if( EC25 >= 2.4 )
      { 
       if(flag3 == 1)
        {
         Serial.println("Time picked for EC up Tank");
         timepick3 = millis();
         flag3 = 0;
        }
        key3 = 1;
      }
   } 

    if( key3 == 1 )
      {
         if( millis() < timepick3 + 10000UL ) //Time needed to provide 10ml
           {
            digitalWrite(wtpump , HIGH);
            timepick3a = millis();
           }
         else
           {
            digitalWrite(wtpump , LOW);
            if( millis() > timepick3a + 600000UL ) //Time for waiting 10 minutes
             { 
               key3 = 0;
             }
            flag3 = 1;
           }   
      } 
 

  //Logics for EC Down Tank:
   if( key4 == 0 )
   {
    if( EC25 <= 1.5 )
      { 
       if(flag4 == 1)
        {
         timepick4 = millis();
         flag4 = 0;
        }
        key4 = 1;
      }
   } 

    if( key4 == 1 )
      {
         if( millis() < timepick4 + 10000UL )//Time needed to provide 10ml
           {
            digitalWrite(nutriApump , HIGH);
            digitalWrite(nutriBpump , HIGH);
            timepick4a = millis();
           }
         else
           {
             digitalWrite(nutriApump , LOW);
             digitalWrite(nutriBpump , LOW);
            if( millis() > timepick4a + 600000UL ) //Time for waiting 10 minutes
             { 
               key4 = 0;
             }
            flag4 = 1;
           }   
      } 

    //Logics for reducing Air Temperature:
    if ( airtemp >= 30 )
      {
      digitalWrite(solenoid,HIGH);  
      }
    else
      {
       digitalWrite(solenoid,LOW);  
      }

    //Time Measurement:
    DateTime now = rtc.now();
    Serial.print(now.hour(), DEC);
    hr = now.hour();
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    mint = now.minute();
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    //Logics for RGB lights:
    if( hr >= 6 && hr <= 9  )
      {
        digitalWrite(rgb , HIGH);    
      }
    else
      {
        digitalWrite(rgb , LOW);  
      }


     //Logics for AC pump:
     if((mint >= 0 && mint < 10) && (mint >= 15 && mint < 25) && (mint >= 30  && mint < 40) && (mint >= 45 && mint <55))
       {
        digitalWrite(acpump ,  HIGH);
       }
     if ((mint >= 10 && mint < 15) && ( mint >= 25 && mint < 30) && ( mint >=40 && mint < 45 ) && ( mint >= 55 && mint <= 60))
       {
        digitalWrite(acpump , LOW); 
       }

     //Logics for Water LEVEL:
     minlevel = digitalRead(fltsensorpin);
     Serial.println(minlevel);
     
     if( fltsensorpin == HIGH )
       { 
         Serial.println("Water in less than 45 mm");
         if(flag == 1)
          {
            mintflag = mint + 20;
            flag = 0;
          }
       }
      
      if ( mint < mintflag )
         {
          digitalWrite(wtpump , HIGH);
         } 
      else
         {
          flag = 1; 
         }

}
