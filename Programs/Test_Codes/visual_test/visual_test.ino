#include "dht.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

#define dht_apin A5 // Analog Pin sensor is connected to
 
int R1= 1000;
int Ra=25; //Resistance of powering Pins
int ECPin= A0;
int ECGround=A1;
int ECPower =A4;

LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
 
float PPMconversion=0.64;

//*************Compensating for temperature ************************************//
//The value below will change depending on what chemical solution we are measuring
//0.019 is generaly considered the standard for plant nutrients [google "Temperature compensation EC" for more info
float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
 
//********************** Cell Constant For Ec Measurements *********************//
//Mine was around 2.9 with plugs being a standard size they should all be around the same
//But If you get bad readings you can use the calibration script and fluid to get a better estimate for K
float K=1.4015;
 
//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 10          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive =8;  //Temp Probe power connected to pin 9
const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
 
float Temperature=10;
float EC=0;
float EC25 =0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;
 
dht DHT;

float airhum;
float airtemp;

float calibration =19.97; //change this value to calibrate
const int analogInPin = A6; 
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10];
int temp;
float dataq[5];


void setup()
 {
  Serial.begin(9600);
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
 buf[i]=analogRead(analogInPin);
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
   
 float pHVol=(float)avgValue*5.0/1024/6;//need to calibrated
 float phi = (-4.57 * pHVol)+ calibration;
 
 Serial.print("pH value = ");
 Serial.print(phi);
 Serial.print("  Signal Voltage = ");
 Serial.print(pHVol);
 Serial.println(" Volt");


 //EC and Water Temperature Measurement:
 sensors.requestTemperatures();// Send the command to get temperatures
 Temperature=sensors.getTempCByIndex(0); //Stores Value in Variable
 
 //************Estimates Resistance of Liquid ****************//
 digitalWrite(ECPower,HIGH);
 raw= analogRead(ECPin);
 raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
 digitalWrite(ECPower,LOW);
 
 //***************** Converts to EC **************************//
 Vdrop= (Vin*raw)/1024.0;
 Rc=(Vdrop*R1)/(Vin-Vdrop);
 Rc=Rc-Ra; //acounting for Digital Pin Resitance
 EC = 1000/(Rc*K);

 //*************Compensating For Temperature********************//
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
 
}
