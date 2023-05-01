#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
#include "dht.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#define dht_apin A5 // Analog Pin sensor is connected to

 RTC_DS3231 rtc;
 //for lcd display with I2C module:
 LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address, if it's not working try 0x27.set the LCD address to 0x3F for a 20 chars and 4 line display
 dht DHT;
 
 //EC Input and Calculation Variables: 
 int R1= 1000;
 int Ra=25; //Resistance of powering Pins
 int ECPin= A0;
 int ECGround=A1; 
 int ECPower =A4;
 float PPMconversion=0.64;
 float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
 float K=2.79;;
 
//Temp Probe Related pins:
#define ONE_WIRE_BUS 10          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive =8;  //Temp Probe power connected to pin 9
const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
//EC and temperature output variables: 
float Temperature=10;
float EC;
float EC25; // Final EC value
float ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;

//DHT11 Output variables:
float airhum;//Variable to store the air humidity.
float airtemp;//Variable to store the air temperature.

//Levels of pH ,EC ,temperature range to compare:
float HighpHrange = 6.59;
float LowpHrange = 5.5;
float LowECrange = 0.6;
float HighECrange = 1.0;
float temp_maintained = 31;



//Time measuring variables:
int hr;//Store the hour variable
int mint;//Store the minute variable 
int sec;//Store the second variable



//pH Variables:
float calibration = 0.9372804; //change this value to calibrate
const int pHpin = A6; //ph sensor pin.
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10];
int temp;//A temporary variable to store average of pH.

//Float sensor pin
int fltsensorpin = 32;

//Actuators variables:
int pHdown = 22;               //relay pin for pHdown pump, relay1
int pHup = 23;                  //relay pin for pHup pump, relay 2
int wtpump = 24; //relay 4'
int nutriApump = 25; // relay 3
int nutriBpump = 26; //relay 4
int growLights = 27 ; // relay 1'
int solenoid = 28; // relay 1" to be shifted
int acpump = 29; // relay 2" to be shifted
int normAirpump = 30;// relay 2' " to be shifted"
int dualAirpump = 31;// relay 3' " to be shifted"



//Timer and logical variables:
int priority = 1;

unsigned long mintflag; // Ensures to pick the time of that instant when the water level is low only for one time.
int flag = 1;//Give access to turn on the water pump due to low water level.
int minlevel;//stores the value of float sensor.

unsigned long lasttime;

int key1 = 0;//Switch the execution of checking value of low pH and the execution of pump running , waiting.
int flag1 = 1;// Ensures to pick the time of that instant when the pH is low only for one time.
unsigned long timepick1;// Stores the time count of the arduino timer when the pH in low.
unsigned long timepick1a;//Stores the time count of the arduino timer the pump running execution is finished.

int key2 = 0;//Switch the execution of checking value of high pH and the execution of pump running , waiting.
int flag2 = 1;// Ensures to pick the time of that instant when the pH is High only for one time.
unsigned long timepick2;// Stores the time count of the arduino timer when the pH in High.
unsigned long timepick2a;//Stores the time count of the arduino timer the pump running execution is finished.

int key3 = 0;//Switch the execution of checking value of High EC and the execution of pump running , waiting.
int flag3 = 1;// Ensures to pick the time of that instant when the EC is High only for one time.
unsigned long timepick3;// Stores the time count of the arduino timer when the EC in High.
unsigned long timepick3a;//Stores the time count of the arduino timer the pump running execution is finished.

int key4 = 0;//Switch the execution of checking value of Low EC and the execution of pump running , waiting.
int flag4 = 1;// Ensures to pick the time of that instant when the EC is LOW only for one time.
unsigned long timepick4;// Stores the time count of the arduino timer when the EC in LOW.
unsigned long timepick4a;//Stores the time count of the arduino timer the pump running execution is finished.

//Time(ms) to run the pumps:
unsigned long runpHupmotor = 2000;//time(ms) to run the pH up pump when pH level is low.
unsigned long runpHdownmotor = 2000;//time(ms) to run the pH down pump when pH level is high.
unsigned long runECupmotor = 2000;//time(ms) to run the EC down pump when EC level is high.
unsigned long runECdownmotor = 2000;//time(ms) to run the EC up pump when EC level is low.
unsigned long runwtpump = 2000;//time(ms) to run the water tank pump when the float sensor senses low water level.

unsigned long watingtime = 600000;//All the pumps waits for 10 minutes/600000ms after checking and runnning the pumps.




void setup()
 {
   Serial.begin(9600);
   Serial1.begin(9600);

   Wire.begin();
   
   lcd.begin(20,4);   // iInit the LCD for 20 chars 4 lines
   lcd.backlight();   // Turn on the backligt (try lcd.noBaklight() to turn it off)
   lcd.setCursor(0,0); //First line
   lcd.print("HYDROPONICS");
   delay(3000);
   lcd.clear();

   //Actators State:
   pinMode(pHdown , OUTPUT);
   pinMode(pHup , OUTPUT);
   pinMode(wtpump , OUTPUT);
   pinMode(nutriApump , OUTPUT);
   pinMode(nutriBpump , OUTPUT);
   pinMode(growLights , OUTPUT);
   pinMode(solenoid , OUTPUT);
   pinMode(acpump , OUTPUT);
   pinMode(normAirpump , OUTPUT);
   pinMode(dualAirpump , OUTPUT);

   digitalWrite(pHdown,HIGH);
   digitalWrite(pHup,HIGH);
   digitalWrite(wtpump,HIGH);
   digitalWrite(nutriApump,HIGH);
   digitalWrite(nutriBpump,HIGH);
   digitalWrite(growLights,HIGH);
   digitalWrite(solenoid,HIGH);
   digitalWrite(acpump,HIGH);
   digitalWrite(normAirpump,HIGH);
   digitalWrite(dualAirpump,HIGH);

   

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
  

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // If you need to set the time of the uncomment line 34 or 37
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //rtc.adjust(DateTime(2020, 7, 13, 14, 44, 0));//unplug the battery and uncomment this line to change the time
 }

void loop() 
{   
   //Time Measurement:
    DateTime now = rtc.now();
    Serial.print(now.hour(), DEC);
    hr = now.hour();//Storing hour to variable.
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    mint = now.minute();//Storing minute to variable.
    Serial.print(':');
    Serial.print(now.second(), DEC);
    sec = now.second();//Storing second to variable.
    Serial.println(); 



  
  //Air temp and Air Humidity Measurement:
  DHT.read11(dht_apin);
    
  Serial.print("Air humidity = ");
  Serial.print(DHT.humidity);
  airhum = DHT.humidity;//Storing air humidity to variable.
  Serial.print("%  ");
  Serial.print("Air Temperature = ");
  Serial.print(DHT.temperature); 
  airtemp = DHT.temperature;//Storing air temperature to variable. 
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
  float phi = (3.2161 * pHVol)+ calibration;


 
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
   Serial.print(" TDS :  ");
   Serial.print(ppm);
   Serial.print(" ppm ");
   Serial.print(" Water temperature: ");
   Serial.print(Temperature);
   Serial.println(" *C ");


      //LCD Print:

       lcd.setCursor(0 , 0);
       lcd.print("pH:");
       lcd.setCursor(10, 0);
       lcd.print("EC:");
       lcd.setCursor(0 , 1);
       lcd.print("AT:");
       lcd.setCursor(10 , 1);
       lcd.print("H:");
       lcd.setCursor(0 , 2);
       lcd.print("WaterTemp:");
       lcd.setCursor(0 , 3);
       lcd.print(hr);
       lcd.setCursor(2,3);
       lcd.print(":");
       lcd.setCursor(3,3);
       lcd.print(mint);
       lcd.setCursor(5,3);
       lcd.print(":");
       lcd.setCursor(6,3);
       lcd.print(sec);

       lcd.setCursor(4, 0);
       lcd.print(phi);
       lcd.setCursor(15 , 0);
       lcd.print(EC25);
       lcd.setCursor(4 , 1);
       lcd.print(airtemp);
       lcd.setCursor(11 , 2);
       lcd.print(Temperature);
       lcd.setCursor(13, 1);
       lcd.print(airhum);

      //Clears the Screen in every 20 Seconds:
       if (millis() - lasttime > 20000UL )
      {
        lcd.clear();
        lasttime = millis();
      }
       
       
       

 

      //DATA FETCHING To the NodeMCU:
      float sen1= airhum;
      float sen2= airtemp;
      float sen3= phi;
      float sen4=  EC25;
      float sen5= Temperature;
      float sen6 = ppm;
      
      
      //Sending data to the NodeMCU:
      float data1=sen1;
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

      float data2=sen2;
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

      float data3=sen3;
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

      float data4=sen4;
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

      float data5=sen5;
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

      float data6=sen6;
      if(Serial1.available()>0)
      {
       char q =Serial1.read();
       if(q =='z')
        {
         Serial1.print(data6);        
         Serial1.print('\n');
         Serial1.flush();
        } 
      }
  
  //Logics for pH Up Tank:
   if( key1 == 0 )
   {
    if( phi <= LowpHrange )//Lower value of PH to be maintained 
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
         if( millis() < timepick1 + runpHupmotor )//Time the pHup pump will run,  
           {
            digitalWrite(pHup , LOW);
            timepick1a = millis();
           }
         else
           {
            digitalWrite(pHup , HIGH);
            if( millis() > timepick1a + watingtime )//Time for waiting before checking again and taking the action, to ensure the solution is mixed properly throughout
             { 
               key1 = 0;
             }
            flag1 = 1;
           }   
      }

   //Logics for pH Down Tank:
   if( key2 == 0 )
   {
    if( phi >= HighpHrange )//Higher value of PH to be maintained 
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
         if( millis() < timepick2 + runpHdownmotor ) ////Time the pHdown pump will run, 
           {
            Serial.println("pH down pump on");
            digitalWrite(pHdown , LOW);
            timepick2a = millis();
           }
         else
           {
            
            digitalWrite(pHdown , HIGH);
            Serial.println("Waiting for ph down break 10 min");
            if( millis() > timepick2a + watingtime ) ////Time for waiting before checking again and taking the action, to ensure the solution is mixed properly throughout
             { 
               Serial.println("Waiting Done");
               key2 = 0;
             }
            flag2 = 1;
           }   
      }
 
   //Logics to deal with EC higher than desired:
   if( key3 == 0 )
   {
    if( EC25 >= HighECrange )//Upper limit of EC to be maintained
      { 
       if(flag3 == 1)
        {
         Serial.println("EC is higher than desired value");
         timepick3 = millis();
         flag3 = 0;
        }
        key3 = 1;
      }
   } 

    if( key3 == 1 )
      {
         if( millis() < timepick3 + runECdownmotor ) ////Time the water pump will run, 
           {
            digitalWrite(wtpump , LOW);
            timepick3a = millis();
           }
         else
           {
            priority = 0 ;
            if( millis() > timepick3a + watingtime ) //Time for waiting before checking again and taking the action, to ensure the solution is mixed properly throughout
             { 
               digitalWrite(wtpump , HIGH);
               priority = 1 ;
               key3 = 0;
             }
            flag3 = 1;
           }   
      } 
 

  //Logics for EC Down Tank:
   if( key4 == 0 )
   {
    if( EC25 <= LowECrange )//Change the lower value of EC
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
         if( millis() < timepick4 + runECupmotor ) //Time needed to provide 10ml
           {
            digitalWrite(nutriApump , LOW);
            digitalWrite(nutriBpump , LOW);
            timepick4a = millis();
           }
         else
           {
             digitalWrite(nutriApump , HIGH);
             digitalWrite(nutriBpump , HIGH);
            if( millis() > timepick4a + watingtime ) //Time for waiting 10 minutes
             { 
               key4 = 0;
             }
            flag4 = 1;
           }   
      } 

    //Logics for reducing Air Temperature:
    
    if ( airtemp >= temp_maintained )//Temperature to be maintained
      {
       if((mint >= 0 && mint < 1) || (mint >= 7 && mint < 8) || (mint >= 14  && mint < 15) || (mint >= 22 && mint <23)|| (mint >= 30 && mint < 31) || (mint >= 38  && mint < 39) || (mint >= 46 && mint <47)|| (mint >= 54 && mint <55))
        {
         digitalWrite(solenoid ,  LOW);
        }
       if ((mint >= 1 && mint < 7) || ( mint >= 8 && mint < 14) || ( mint >=15 && mint < 22 ) || ( mint >= 23 && mint < 30)|| ( mint >= 31 && mint < 38) || ( mint >=39 && mint < 46 ) || ( mint >= 47 && mint < 54) || ( mint >= 55) )
        {
         digitalWrite(solenoid , HIGH); 
        }
      }
    else
      {
       digitalWrite(solenoid,HIGH);  
      }

   

    //Logics for Grow lights:
    if(( hr >= 7 && hr <= 11 ) || ( hr >= 13 && hr <= 23 )  ) // Lights switching on and off time
      {
        digitalWrite(growLights , LOW);    
      }
    else
      {
        digitalWrite(growLights , HIGH);  
      }

      
     //Logics for AC pump:
     if((mint >= 0 && mint < 10) || (mint >= 15 && mint < 25) || (mint >= 30  && mint < 40) || (mint >= 45 && mint <55))
       {
        digitalWrite(acpump ,  LOW);
       }
     if ((mint >= 10 && mint < 15) || ( mint >= 25 && mint < 30) || ( mint >=40 && mint < 45 ) || ( mint >= 55 && mint <= 60))
       {
        digitalWrite(acpump , HIGH); 
       }

      //Logics for Normal AC Air pump:
     if((mint >= 0 && mint < 10) || (mint >= 15 && mint < 25) || (mint >= 30  && mint < 40) || (mint >= 45 && mint <55))
       {
        digitalWrite(normAirpump ,  LOW);
       }
     if ((mint >= 10 && mint < 15) || ( mint >= 25 && mint < 30) || ( mint >=40 && mint < 45 ) || ( mint >= 55 && mint <= 60))
       {
        digitalWrite(normAirpump , HIGH); 
       }

            //Logics for Dual AC Air pump:
     if((mint >= 0 && mint < 10) || (mint >= 15 && mint < 25) || (mint >= 30  && mint < 40) || (mint >= 45 && mint <55))
       {
        digitalWrite(dualAirpump ,  LOW);
       }
     if ((mint >= 10 && mint < 15) || ( mint >= 25 && mint < 30) || ( mint >=40 && mint < 45 ) || ( mint >= 55 && mint <= 60))
       {
        digitalWrite(dualAirpump , HIGH); 
       }

     //Logics for Water LEVEL:
     minlevel = digitalRead(fltsensorpin);
     Serial.print("Float Sensor Signal: ");
     Serial.println(minlevel);


     if( minlevel == 0 )
       { 
         Serial.println("Water in less than 45 mm");
         if(flag == 1)
          { 
            Serial.println("Time picked for the water tank level");
            mintflag = millis();
            flag = 0;
          }
       }
      if ( priority == 1)
      {
      if(flag == 0)
      {
      if ( millis() < mintflag + runwtpump )//the motor will be turned on for 1 sec.
         {
          Serial.println("Motor is running");
          digitalWrite(wtpump , LOW);
         } 
      else
         {
          Serial.println("Water tank level is fine");
          flag = 1; 
          digitalWrite(wtpump, HIGH);
         }
      }
      }
         

}
