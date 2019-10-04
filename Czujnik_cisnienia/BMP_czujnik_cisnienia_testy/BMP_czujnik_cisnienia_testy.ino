#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>

Adafruit_BMP085 bmp;

const char* ssid     = "PaciorT";
const char* password = "Pacior11235";

const char* host = "192.168.8.121";
const int httpPort = 80;

void setup() {
  Serial.begin(250000);
  Serial.println("Setup! ");
  delay(500);
  Wire.begin(0, 2);
  if (!bmp.begin()) {
     Serial.println("No BMP180 / BMP085");// we dont wait for this
     while (1){};
  }
  
  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Koniec Setup!!!! ");
}

void loop() {
  delay(60000);
  String t = "T=" + String(bmp.readTemperature()) + " *C";
  String p = String(bmp.readPressure());
  String a = "A=" + String(bmp.readAltitude(103000)) + " m";// insert pressure at sea level
  Serial.println("Temperatura: ");
  Serial.println(t);
  Serial.println("Cisnienie: ");
  Serial.println(p);
  Serial.println("Wysokosc: ");
  Serial.println(a);



  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  //Serial.println(url);
  Serial.println("/PRESSURE");
  
  // This will send the request to the server
  client.print(String("POST ") + "/PRESSURE" + " HTTP/1.1\r\n" +
               "Host: " + host + "\r" + 
               "--------------------" + "\r" +
               "BMP180 = " + "\r" + p + "\r\n"       );

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 3000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  client.stop();
  Serial.println(">>> Client Timeout !");
  
}
