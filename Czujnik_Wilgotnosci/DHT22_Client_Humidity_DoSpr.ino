#include "DHT.h"
#include <ESP8266WiFi.h>

#define DHTPIN 2   
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);


const char* ssid     = "PaciorT";
const char* password = "Pacior11235";

const char* host = "192.168.8.121";
const char* streamId   = "....................";
const char* privateKey = "....................";

void setup() {
  Serial.begin(250000);
  delay(10);
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  //Serial.println("DHT22 Tylko wilgotnosc, mozna dodaj jeszcze temp odczuwalna i temp powietrza !");
  dht.begin();

}

void loop() {
    delay(60000);

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) /*|| isnan(f)*/) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
 // float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.println("Humidity: ");
  Serial.println(h);
  //Serial.print(" %\t");
 // Serial.print("Temperature: ");
 // Serial.print(t);
 // Serial.print(" *C ");
//  Serial.print(f);
//  Serial.print(" *F\t");
 // Serial.print("Heat index: ");
 // Serial.print(hic);
 // Serial.println(" *C ");
//  Serial.print(hif);
//  Serial.println(" *F");


//**********************************************************************************************//

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
   Serial.print("Requesting URL: ");
  //Serial.println(url);
  Serial.println("/HUMIDITY");
  
  // This will send the request to the server
  client.print(String("POST ") + "/HUMIDITY" + " HTTP/1.1\r\n" +
               "Host: " + host + "\r" + 
               "--------------------" + "\r" +
               "DHT22 = " + "\r" + h + "\r\n"       );
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);

  }
  
  Serial.println();
  client.stop();
  Serial.println("closing connection");
}
