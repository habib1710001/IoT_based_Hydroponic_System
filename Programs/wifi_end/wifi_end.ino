#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

SoftwareSerial wifi_end(D6,D5);//rx tx


const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";

// Service API Key
String apiKey = "Y868PQXYM286TPL4";//change that with your api key name:

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;

// Timer set to 1 seconds (1000)
unsigned long timerDelay = 1000;


String sen1;
String sen2;
String sen3;
String sen4;
String sen5;

void setup() 
{
  Serial.begin(115200);
  wifi_end.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
}

void loop() 
{   
     //Air Humidity => field1 in the thinkspeak
     wifi_end.write("s");
    if (wifi_end.available()>0)
    {
      sen1=wifi_end.readStringUntil('\n');
      Serial.println(sen1);
    }

     //Air temperature => field2 in the thinkspeak
    wifi_end.write("k");
    if (wifi_end.available()>0)
    {
      sen2=wifi_end.readStringUntil('\n');
      Serial.println(sen2);
    }

    //pH => field3 in the thinkspeak
    wifi_end.write("l");
    if (wifi_end.available()>0)
    {
      sen3=wifi_end.readStringUntil('\n');
      Serial.println(sen3);
    }

    //EC => field4 in the thinkspeak
     wifi_end.write("j");
    if (wifi_end.available()>0)
    {
      sen4=wifi_end.readStringUntil('\n');
      Serial.println(sen4);
    }
    
    //Temperature => field5 in the thinkspeak
     wifi_end.write("y");
    if (wifi_end.available()>0)
    {
      sen5=wifi_end.readStringUntil('\n');
      Serial.println(sen5);
    }

  //Send an HTTP POST request every 10 seconds
  if ((millis() - lastTime) > timerDelay) 
  {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      // Data to send with HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(sen1)+ "&field2=" + String(sen2)+ "&field3=" + String(sen3)+ "&field4=" + String(sen4)+ "&field5=" + String(sen5);           
     
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
