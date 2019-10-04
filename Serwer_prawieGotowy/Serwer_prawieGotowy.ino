#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#define FS_NO_GLOBALS //dodane ponieważ był konflikt bibliotek File, teraz obiekty File z FS.h  np:--> fs::File plik;
#include <FS.h>

char ssid[] = "PaciorT";       //  your network SSID (name)
char pass[] = "Pacior11235";          //  your network password
IPAddress ip(192, 168, 8, 121);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(80);
WiFiClient client;
RTC_DS1307 rtc;
File fsd; //obiekt File karty SD do zapisu i odczytu
fs::File fhtml; // obiekt File do odczytu z FS

String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String request = "";


//------------funkcje---------------
String printTime();
void pobierzSPIFFS(String patch);
void zapiszSD(String patch);
void pobierzSD(String patch);
void pobierzSDPosition(String patch);
void pobierzSDPosition(String patch, int Fline);
String dodajZero(String x);


void setup()
{
  Serial.begin(250000);
  Serial.println();

  //pamięć procesora inicjalizacja
  SPIFFS.begin();

  Wire.begin(5, 4);  // initralization i2c wire
  //---------------------RTC TIME--------------------------------
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  //-----------------------SD Card--------------------------------
  Serial.println();
  Serial.print("Initializing SD card...");

  if (!SD.begin(SS)) {
    Serial.println("initialization failed!");
    Serial.println();
    return;
  }
  Serial.println("initialization done.");
  Serial.println();


  //---------------------WiFi Network--------------------------------------

  // Connect to a WiFi network
  WiFi.config(ip, gateway, subnet);
  Serial.print(F("Connecting to "));  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println(F("[CONNECTED]"));
  Serial.print("[IP ");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  // start a server
  server.begin();
  Serial.println("Server started");
  Serial.println();

} // void setup()



void loop()
{
  // Check if a client has connected
  client = server.available();
  if (!client)  {
    return;
  }

  // Read the first line of the request
  request = client.readStringUntil('\r');
  Serial.print("Request: "); 
  Serial.println(request);

// GET strony zaczynamy od 1
// GET czujników zaczynamy od 10
// POST czujników zaczynamy od 20


  int zmienna = 0;
  if (request == "GET / HTTP/1.1") {
    zmienna = 1;
  }
  else if (request == "GET /index.html HTTP/1.1") {
    zmienna = 1;
  }
  else if (request == "GET /wykresy.html HTTP/1.1") {
    zmienna = 2;
  }
  else if (request == "GET /download.html HTTP/1.1") {
    zmienna = 3;
  }
  else if (request == "GET /Itemp HTTP/1.1") {
    zmienna = 4;
  }
  else if (request == "GET /Iwil HTTP/1.1") {
    zmienna = 5;
  }
  else if (request == "GET /Icis HTTP/1.1") {
    zmienna = 6;
  }
  else if (request == "GET /WykresyTemp HTTP/1.1") {
    zmienna = 7;
  }
  else if (request == "GET /WykresyWil HTTP/1.1") {
    zmienna = 8;
  }
  else if (request == "GET /WykresyCis HTTP/1.1") {
    zmienna = 9;
  }
  else if (request == "GET /TEMP HTTP/1.1") {
    zmienna = 10;
  }
  else if (request == "GET /PRESSURE HTTP/1.1") {
    zmienna = 11;
  }
  else if (request == "GET /HUMANITY HTTP/1.1") {
    zmienna = 12;
  }
  else if (request == "GET /LIGHT HTTP/1.1") {
    zmienna = 13;
  }
  else if (request == "GET /SMOKE HTTP/1.1") {
    zmienna = 14;
  }
  else if (request == "POST /TEMP HTTP/1.1") {
    zmienna = 20;
  }
  else if (request == "POST /HUMANITY HTTP/1.1") {
    zmienna = 21;
  }
  else if (request == "POST /PRESSURE HTTP/1.1") {
    zmienna = 22;
  }
  else {
    zmienna = 0;
  };

 // Serial.print( "Zmienna: " );
 // Serial.println( zmienna );



  String patch = "";
  int Fline = 0;
  switch (zmienna) {

    case 1:
      patch = "/index.html";
      Serial.println("1 - Index");
      pobierzSPIFFS(patch);
      break;

    case 2:
      patch = "/wykresy.html";
      Serial.println("2 - Wykresy");
      pobierzSPIFFS(patch);
      break;

    case 3:
      patch = "/download.html";
      Serial.println("3 - Download");
      pobierzSPIFFS(patch);
      break;

    case 4:
      patch = "TEMP.txt";
      Serial.println("4 - TempPosition");
      pobierzSDPosition(patch, Fline);
      break;

    case 5:
      patch = "HUMIDITY.txt";
      Serial.println("5 - WilPosition");
      pobierzSDPosition(patch, Fline);
      break;

    case 6:
      patch = "PRESSURE.txt";
      Serial.println("6 - CisPosition");
      pobierzSDPosition(patch, Fline);
      break;

    case 7:
      patch = "TEMP.txt";
      Fline = 1;
      Serial.println("7 - WykresyTemp");
      pobierzSDPosition(patch, Fline);
      break;

    case 8:
      patch = "HUMIDITY.txt";
      Fline = 1;
      Serial.println("8 - WykresyWil");
      pobierzSDPosition(patch, Fline);
      break;

    case 9:
      patch = "PRESSURE.txt";
      Fline = 1;
      Serial.println("9 - WykresyCis");
      pobierzSDPosition(patch, Fline);
      break;


    //   ------------------------------------------- G E T  całe pliki ------------------------------------  //

    case 10:
      patch = "TEMP.txt";
      Serial.println("10 - TEMP");
      pobierzSD(patch);
      break;

    case 11:
      patch = "PRESSURE.txt";
      Serial.println("12 - PRESSURE");
      pobierzSD(patch);
      break;

    case 12:
      patch = "HUMIDITY.txt";
      Serial.println("13 - HUMIDITY");
      pobierzSD(patch);
      break;

    case 13:
      patch = "LIGHT.txt";
      Serial.println("14 - LIGHT");
      pobierzSD(patch);
      break;

    case 14:
      patch = "SMOKE.txt";
      Serial.println("15 - SMOKE");
      pobierzSD(patch);
      break;

    //    ------------------------------------------- P O S T  Czujniki ------------------------------------  //
    case 20:
      patch = "TEMP.txt";
      Serial.println("20 - TEMP - ZAPIS");
      zapiszSD(patch);
      break;

    case 21:
      patch = "HUMIDITY.txt";
      Serial.println("21 - HUMIDITY - ZAPIS");
      zapiszSD(patch);
      break;

    case 22:
      patch = "PRESSURE.txt";
      Serial.println("22 - PRESSURE - ZAPIS");
      zapiszSD(patch);
      break;

    default:
      break;
  }

  Serial.println("Client disonnected");
  Serial.println();
  client.flush();
} // void loop()






//------------------------------------------- F U N K C J E --------------------------------------------------


String printTime() {
  DateTime noww = rtc.now();

  String rok = String(noww.year());
  String mies = String(noww.month());
  String dzien = String(noww.day());
  String godz = String(noww.hour());
  String minuta = String(noww.minute());
  String sec = String(noww.second());

  mies = dodajZero(mies);
  dzien = dodajZero(dzien);
  godz = dodajZero(godz);
  minuta = dodajZero(minuta);
  sec = dodajZero(sec);

  String datatime = rok + '-' + mies + '-' + dzien + ' ' + godz + ':' + minuta ;
  
  Serial.print(">Date and time: ");
  Serial.println(datatime + ':' + sec);
  return datatime;
}

String dodajZero(String x) {
  if (x.equals("0") || x.equals("1") || x.equals("2") || x.equals("3") || x.equals("4") ||
      x.equals("5") || x.equals("6") || x.equals("7") || x.equals("8") || x.equals("9")) {
    x = "0" + x;
  }
  return x;
}


//Pobiera dane z Plików SPIFFS
void pobierzSPIFFS(String patch) {
  String danePlik = "";

  //"/wykresy.html"
  if (SPIFFS.exists(patch)) {
    fhtml = SPIFFS.open(patch, "r");
    if (!fhtml) {
      Serial.println("file open failed");
    }
    if (fhtml && fhtml.size()) {
      client.flush();
      client.print( header );
      int i = 0;
      while (fhtml.available()) {
        danePlik += fhtml.readStringUntil('\r');
        if (i > 50)
        {
          client.print(danePlik);
          danePlik = "";
          i = 0;
        }
        i++;
      }
    }
    client.print(danePlik);
    fhtml.close();
  }
}



//--------------------------------------------SD CARD-------------------------------------------//

//download all file with data
void pobierzSD(String patch) {
  String danePlik = "";

  // re-open the file for reading:
  fsd = SD.open(patch);

  if (!fsd) {
    Serial.print("file open failed: ");
    Serial.println(patch);
  }
  if (fsd && fsd.size()) {
    Serial.println(patch);
    client.flush();
    client.print( header );
    int i = 0;
    while (fsd.available() && client.available()) {
      danePlik += fsd.readStringUntil('\r');
      if (i == 50 || i == 100 || i == 150 || i == 200 || i == 250 || i == 300 || i == 350 || i == 400) {
        client.print(danePlik);
        Serial.println(danePlik);
        danePlik = "";
      }
      i++;
    }
  }
  client.print(danePlik);
  Serial.println(danePlik);
  fsd.close();
}


//save measurment data from sensors
void zapiszSD(String patch) {
  String wartosc = " empty ";

  //wiadomość pozycjonujaca - --------------------
  while (request.indexOf("--------------------") == -1) {
    request = client.readStringUntil('\r');
    Serial.println(request);
  }
  Serial.println(client.readStringUntil('\r'));// nazwa czujnika
  wartosc = client.readStringUntil('\r'); // wartość czujnik
  Serial.println(wartosc);

  fsd = SD.open(patch, FILE_WRITE);

  if (fsd) {
    Serial.println("Writing to " + patch + "...");
    fsd.print(wartosc + " ;");
    fsd.println(printTime()); // dodanie czasu pobrania danych do pliku
    fsd.close();
    Serial.println("File saved!");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening " + patch);
  }
}



//only one line for plot on WWW website
void pobierzSDPosition(String patch, int Fline) {
  String danePlik = "";
  unsigned long rozmiar = 0;

  // re-open the file for reading:
  fsd = SD.open(patch);
  rozmiar = fsd.size();

  if (!fsd) {
    Serial.print("file open failed: ");
    Serial.println(patch);
  }
  if (fsd && rozmiar) {
    if (patch == "TEMP.txt") fsd.seek(rozmiar - 25);
    else if (patch == "HUMIDITY.txt") fsd.seek(rozmiar - 25);
    else if (patch == "PRESSURE.txt") fsd.seek(rozmiar - 26); //przesuwamy wskaźnik na początek wiersza w pliku aby pobrać ostatnio zapisana wartosc
    //Serial.println(patch);
    client.flush();
    client.print( header );
    client.println("Massage for synchronization!");
    if (Fline == true)danePlik = fsd.readStringUntil('/r');
    else danePlik = fsd.readStringUntil(';');
    client.print(danePlik);
    Serial.println(danePlik);
    fsd.seek(rozmiar - 1);
    fsd.close();
  }
  else {
    Serial.println("Pusty plik");
  }
}

