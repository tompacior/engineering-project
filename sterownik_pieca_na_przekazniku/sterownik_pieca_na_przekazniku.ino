#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

unsigned long czasProgramu;
unsigned long ostatniCzas = 0;

unsigned long tPobraniaDanych = 1;

bool outDisabled = true;

OneWire  ds(5);  // on pin 2 (a 4.7K resistor is necessary)      

RTC_DS1307 rtc;
String rok, mies, dzien, godz, minuta, sec;
int tablica[10] = {2, 3, 9, 10, 16, 17, 23, 24, 30, 31}; //weekendy

LiquidCrystal_I2C lcd(0x3F, 16, 2);
ESP8266WiFiMulti WiFiMulti;

#define PIN_INPUT 5 // termometr DS18b20 podłączony do RTC
#define PIN_OUTPUT 13 //D8 Przekaznik
#define BTN_UP  14 // przycisk up
#define BTN_DOWN  12 // przycisk down 
#define BTN_ENTER    15 //przycisk Enter

typedef struct { //struktura do LCD
  String label;
  int minVal;
  int maxVal;
  int currentVal;
  void (*handler)();
} STRUCT_MENUPOS;

typedef enum {
  DOWN, UP, ENTER, NONE
} ENUM_BUTTON;

STRUCT_MENUPOS menu[6];

//zmienne do LCD
int currentMenuPos = 0;
int menuSize;
bool isInLowerLevel = false;
int tempVal;


int tempInHome = 0; // temp pobrana z serwera
int setPoint = 0;
int setPointWeekend = 23;// 23
int setPointWeek = 22; //22
int setPointNoc = 20 ; //20
int histereza = 2;//2
int DS; // temperatura DS18b20


//funkcje
void setup_WiFi();                 //Ustawienia SETUP do TCP
int conection();                   //Funkcja odpowiedzialna za połaczenie TCP i pobranie temp z czujnika
String print_time();               //odczyt czasu z RTC
float connection_DS ();            //DS18B20 Odczytanie temperatury z RTC
void error_connection(int blad);   //błąd połączenia


//*******************************************************************************************************


void setup() {
  Serial.begin(250000);

  pinMode(PIN_OUTPUT, OUTPUT);
  pinMode(PIN_INPUT, INPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);

  menu[0] = {"T.WEEKEND", 10, 80, setPointWeekend, NULL};
  menu[1] = {"T.WEEK", 10, 80, setPointWeek, NULL};
  menu[2] = {"T.NIGHT", 10, 80, setPointNoc, NULL};
  menu[3] = {"HYSTERESIS", 1, 10, histereza, NULL};
  menu[4] = {"REFRESH", 1, 100, tPobraniaDanych, NULL};
  menu[5] = {"SHOW", 0, 0, 0, mGlowne};
  menuSize = sizeof(menu) / sizeof(STRUCT_MENUPOS);

  setup_WiFi();

  Wire.begin(2, 4); // INIT I2C ON PIN D5 - SDA, D6 - SCL //

  //**********************************INIT_Rtc******************************
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // zgranie czasu z komputerem

  // ********************************INIT_LCD*******************************
  lcd.init();
  lcd.setBacklight(255);

  float tempPt100 = connection_DS ();   //odczyt temperatury z DS18b20

  DS = int(tempPt100);
  if (DS == -1) {
    error_connection(3);
    Serial.print("DS18B20 = ");
    Serial.println(DS);
  } else if (DS >= 40 || DS <= 10) {
    error_connection(2);
    Serial.print("DS18B20 = ");
    Serial.println(DS);
  }
}
//****************************************************************************************************


void loop() {

  czasProgramu = millis();
  if (czasProgramu >= ostatniCzas + menu[4].currentVal * 60000) {
    ostatniCzas = czasProgramu ;

    float tempPt100 = connection_DS ();   //odczyt temperatury z DS18b20
    DS = int(tempPt100);

    tempInHome = connection();
    if (tempInHome == -1) {   // odczyt temperatury z serwera
      error_connection(5);
      tempInHome = DS;
      Serial.println("Błędy danych, tempInHome = DS  outDisabled false");
      outDisabled = false;

    } else {
      outDisabled = false;
      //Serial.println("outDisabled false");
    }
  }
  if (tempInHome < 10 || tempInHome > 40) {
    outDisabled = true;
    //Serial.println("Błędy danych ");
  }

  String czasOdczytany = print_time();
  int dzienInt = dzien.toInt();
  int godzInt = godz.toInt();
  // Serial.print("Dzien: ");
  // Serial.println(dzienInt);
  // Serial.print("godz: ");
  // Serial.println(godzInt);

  //sprawdzamy czy weekend
  for (int i = 0; i <= 10; i++) {
    if (dzienInt == tablica[i]) {
      setPoint = menu[0].currentVal;
      //Serial.print("Weekend:");
      //Serial.println(setPoint);
      i = 10;
    } else {
      setPoint = menu[1].currentVal;
      //  Serial.print("Week:");
      //  Serial.println(setPoint);
    }
  }

  //sprawdzamy czy noc
  if (godzInt <= 7 || godzInt >= 21) {
    setPoint = setPoint = menu[2].currentVal;
    Serial.print("Noc:");
    Serial.println(setPoint);
  }


  Serial.print("OutDisabled: ");
  Serial.println(outDisabled);
  Serial.print("tempInHome: ");
  Serial.println(tempInHome);

  /*
    Serial.print("setPointWeekend: ");
    Serial.println(menu[0].currentVal);
    Serial.print("setPointWeek: ");
    Serial.println(menu[1].currentVal);
    Serial.print("setPointNoc: ");
    Serial.println(menu[2].currentVal);
    Serial.print("histereza: ");
    Serial.println(menu[3].currentVal);

    Serial.print("Refresh: ");
    Serial.println(menu[4].currentVal);
  */

    
   
  //Serial.println(" ");
  Serial.print("DS: ");
  Serial.println(DS);

  //Serial.println(" ");
  Serial.print("setPoint: ");
  Serial.println(setPoint);

  //Serial.println(" ");

  if (!outDisabled) {

    if (tempInHome >= setPoint + menu[3].currentVal) {
      digitalWrite(PIN_OUTPUT, LOW);
      //Serial.println("Output LOW");
      //Serial.println(" ");
    } else if (tempInHome <= setPoint - menu[3].currentVal) {
      digitalWrite(PIN_OUTPUT, HIGH);
      //Serial.println("Output HIGH");
      //Serial.println(" ");
    } else {
      Serial.println("W histerezie!");
      Serial.println(" ");
    }
  }
  drawMenu();
  Serial.println();
  //Serial.println("...");
  //delay(200);
}




//*******************************************************************************************************

void setup_WiFi() {                           //Ustawienia SETUP do TCP

  WiFiMulti.addAP("PaciorT", "Pacior11235");

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP());
  delay(500);
}

//******************************************************************************************************


int connection() {           //Funkcja odpowiedzialna za połaczenie TCP i pobranie temp z czujnika
  const uint16_t port = 80;
  const char * host = "192.168.8.121"; // ip or dns
  int tempOnHome;

  Serial.print("Connecting to: ");
  Serial.println( host);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 0,5 sec...");
    delay(10);
    return -1;
  }
  // This will send the request to the server
  client.println("GET /Itemp HTTP/1.1");
  String line = "999 ";

  while (client.available()) {
    line = client.readStringUntil('\r');
    while (line.indexOf("Massage for synchronization!") != -1) {
      line = client.readStringUntil('\r');
      //Serial.print("T. Home: ");
      //Serial.println(line);
      tempOnHome = line.toInt();
      Serial.print(">Temperatura : ");
      Serial.println(tempOnHome);
    }
  }
  Serial.println("Closing connection...");
  Serial.println();
  client.stop();

  return tempOnHome;
}


//******************************************************************************************************

String print_time() {                        // odczyt czasu z RTC
  DateTime noww = rtc.now();

  rok = String(noww.year());
  mies = String(noww.month());
  dzien = String(noww.day());
  godz = String(noww.hour());
  minuta = String(noww.minute());
  sec = String(noww.second());

  String data_time = rok + '/' + mies + '/' + dzien + ' ' + godz + ':' + minuta ;
  //Serial.println("Data_czas");
  //Serial.println(data_time + ':' + sec);

  return data_time;
}


//******************************************************************************************************


float connection_DS () {                //DS18B20 Odczytanie temperatury z RTC

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;

  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(50);
    return -1;
  }
  //delay(2000);


  Serial.print("ROM =");
  for ( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1;
  }
  Serial.println();

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return -1;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(800);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    // Serial.print(data[i], HEX);
    // Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  //Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;

  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius, ");


  ds.reset_search();

  return celsius;
}

//******************************************************************************************************

void error_connection(int blad) { //błąd połączenia

  Serial.println("error connection 2-za niska/wyskoka temp z DS ");
  Serial.println("error connection 3-bąd połączenia DS ");
  Serial.println("error connection 5-błąd połączenia serwer");
  Serial.println(blad);
  Serial.println(" ");

  for (int i = 0; blad <= i; i++) {

    delay(500);
  }
}

//******************************************************************************************************
//USTAWIENIA MENU NA LCD

ENUM_BUTTON getButton() {
  if (!digitalRead(BTN_DOWN)) return DOWN;
  if (!digitalRead(BTN_UP)) return UP;
  if (digitalRead(BTN_ENTER)) return ENTER;

  return NONE;
}


void drawMenu() {

  static unsigned long lastRead = 0;
  static ENUM_BUTTON lastPressedButton = ENTER;
  static unsigned int isPressedSince = 0;
  int autoSwitchTime = 500;

  ENUM_BUTTON pressedButton = getButton();

  if (pressedButton == NONE && lastRead != 0) {
    isPressedSince = 0;
    return;
  }
  if (pressedButton != lastPressedButton) {
    isPressedSince = 0;
  }

  if (isPressedSince > 3) autoSwitchTime = 70;
  if (lastRead != 0 && millis() - lastRead < autoSwitchTime && pressedButton == lastPressedButton) return;

  isPressedSince++;
  lastRead = millis();
  lastPressedButton = pressedButton;

  switch (pressedButton) {
    case DOWN: handleDown(); break;
    case UP: handleUp(); break;
    case ENTER: handleEnter(); break;
  }

  lcd.home();
  lcd.clear();
  if (isInLowerLevel) {
    lcd.print(menu[currentMenuPos].label);
    lcd.setCursor(0, 1);
    lcd.print(F("> "));

    if (menu[currentMenuPos].handler != NULL) {
      (*(menu[currentMenuPos].handler))();
    } else {
      lcd.print(tempVal);
    }
  } else {
    lcd.print(F("Menu glowne"));
    lcd.setCursor(0, 1);
    lcd.print(F("> "));

    lcd.print(menu[currentMenuPos].label);
  }
}


void handleUp() {
  if (isInLowerLevel) {
    tempVal++;
    if (tempVal > menu[currentMenuPos].maxVal) tempVal = menu[currentMenuPos].maxVal;
  } else {
    currentMenuPos = (currentMenuPos + 1) % menuSize;
  }
}

void handleDown() {
  if (isInLowerLevel) {
    tempVal--;
    if (tempVal < menu[currentMenuPos].minVal) tempVal = menu[currentMenuPos].minVal;
  } else {
    currentMenuPos--;
    if (currentMenuPos < 0) currentMenuPos = menuSize - 1;
  }
}


void handleEnter() {
  if (menu[currentMenuPos].handler != NULL && menu[currentMenuPos].maxVal <= menu[currentMenuPos].minVal) {
    (*(menu[currentMenuPos].handler))();
    //return;
  }
  if (isInLowerLevel) {
    menu[currentMenuPos].currentVal = tempVal;
    isInLowerLevel = false;
  } else {
    tempVal = menu[currentMenuPos].currentVal;
    isInLowerLevel = true;
  }
}

/* Funkcje-uchwyty użytkownika */

void mGlowne() {  // info o temp na serwerze i na PT100

  String przekaznik;
  lcd.setCursor(0, 0);
  lcd.print(F("Home:"));
  lcd.setCursor(5, 0);
  lcd.print(tempInHome);
  if (1 == digitalRead(PIN_OUTPUT)) {
    przekaznik = "Hi";
  } else {
    przekaznik = "Lo";
  }
  lcd.setCursor(11, 0);
  lcd.print(F("DS:"));
  lcd.setCursor(14, 0);
  lcd.print(DS);

  lcd.setCursor(0, 1);
  lcd.print(F("S.Point:"));
  lcd.setCursor(8, 1);
  lcd.print(setPoint);
  
  lcd.setCursor(12, 1);
  lcd.print(F("P:"));
  lcd.setCursor(14, 1);
  lcd.print(przekaznik);


}

