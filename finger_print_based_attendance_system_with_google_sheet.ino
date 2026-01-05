#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
SoftwareSerial mySerial(D4, D5);  // RX & TX pins

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);
//// MOSI D7 MISO D6 SCK D5 
bool flag1=false;
bool flag2=false;

int led1=D7;
int led2=D4;

// Enter network credentials:
const char* ssid     = "theinfoflux";
const char* password = "12345678";

// Enter Google Script Deployment ID:
const char *GScriptId = "";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;

String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Declare variables that will be published to Google Sheets
String cardholder = "";
String id = "";
String Status ="";



void showScanMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place finger");
  lcd.setCursor(0, 1);
  lcd.print("to scan...");
}


void setup() {
  Serial.begin(115200);        
  Serial.println('\n');
  Serial.println("System initialized");
  pinMode(led1,OUTPUT);
  lcd.begin();
lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1500);

  finger.begin(57600);
  delay(100);

  if (!finger.verifyPassword()) {
    Serial.println("Sensor not found");
    while (1);
  }

  finger.getParameters();
  finger.getTemplateCount();

  delay(500);
  showScanMessage();
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object


  
}


void loop() {
uint8_t result = getFingerprintID();

  if (result > 0) {  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Matched!");
        if (finger.fingerID == 1) 
  {
   digitalWrite(led1,HIGH);
    if( flag1== false)
    {
       lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
   
      cardholder="Salman";
      id="EE123";
      Status="in";
      updatesheet(cardholder,id, Status);
      delay(1000);
     flag1=true;
      digitalWrite(led1,LOW);
    }

 
    else
    {   digitalWrite(led1,HIGH);
           lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
    
      Status="";
      cardholder="Salman";
      id="EE123";
      Status="out";
             updatesheet(cardholder,id, Status);
      delay(1000);
       flag1=false;
       digitalWrite(led1,LOW);
    }

  }
    showScanMessage();   
     
  }

}

void updatesheet(String cardholder, String id, String Status)

{


  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + cardholder + "," + id + "," + Status + "\"}";
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  delay(5000);
  }
  
uint8_t getFingerprintID() {

  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return 0;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return 0;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) {

    // ===== WRONG FINGER AREA =====


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(2000);

    showScanMessage();
    return 0;
  }

  // MATCH FOUND
  return finger.fingerID;
}
  
