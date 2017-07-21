/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
See more at http://blog.squix.ch
*/

#include <ESP8266WiFi.h>
#include "DHT.h"

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

/***************************
 * Begin Settings
 **************************/

const char* host = "api.thingspeak.com";

const char* THINGSPEAK_API_KEY = "XXXXXXXXXXXXXXXX";

// DHT Settings
#define DHTPIN D6     // what digital pin we're connected to. If you are not using NodeMCU change D6 to real pin


// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTTYPE DHT21   // DHT 21 (AM2301)

const boolean IS_METRIC = true;

// Update every 30 seconds = 0,5 minutes. Min with Thingspeak is ~20 seconds
const int UPDATE_INTERVAL_SECONDS = 30;

/***************************
 * End Settings
 **************************/
 
// Initialize the temperature/ humidity sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFiManager wifiManager;
  wifiManager.autoConnect("CzujnikDHT21_lok1");
  Serial.println("");
  Serial.println("WiFi połączone");  
  Serial.println("IP adres: ");
  Serial.println(WiFi.localIP());
}

void loop() {      
    Serial.print("Łączymy się z ");
    Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("połączenie niedostępne");
      return;
    }

    // read values from the sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(!IS_METRIC);

     if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Nie mogę odczytać danych z czujnika DHT!");
      return;
     }
    
    // We now create a URI for the request
    String url = "/update?api_key=";
    url += THINGSPEAK_API_KEY;
    url += "&field1=";
    url += String(temperature);
    url += "&field2=";
    url += String(humidity);
    
    Serial.print("Szukam URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(10);
    while(!client.available()){
      delay(100);
      Serial.print(".");
    }
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
     Serial.print("Temperatura: ");
     Serial.print(temperature);
     Serial.print(" stopni Celcius Wilgotność: "); 
     Serial.print(humidity);
     Serial.println("% wysłane do Thingspeak"); 

  // Go back to sleep. If your sensor is battery powered you might
  // want to use deep sleep here
  delay(1000 * UPDATE_INTERVAL_SECONDS);
}
