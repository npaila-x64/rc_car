#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPLF4KOcG-R"
#define BLYNK_DEVICE_NAME "test"
#define BLYNK_AUTH_TOKEN "qlnsbfZj9nhSvLBtvo5-7AcSev-B0WHR"
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
SimpleTimer timer;

//********************* COMPLETE WITH YOUR DETAILS *************
// Get Auth Token in the Blynk App.
char auth[] = "qlnsbfZj9nhSvLBtvo5-7AcSev-B0WHR";  
char cloudserver[16] = "blynk-cloud.com";
char localserver[16] = "*******";  // IP details for the local server.
char ssid[] = "*******";           // Your WiFi credentials.
char pass[] = "*******";           // Password.
//****************

void reconnectBlynk() {                         // reconnect to server if disconnected
  if (!Blynk.connected()) {
    if(Blynk.connect()) {
      BLYNK_LOG("Reconnected");
    } else {
      BLYNK_LOG("Not reconnected");
    }
  }
}

int motor ; // motor connected at D5 and D6 WeMos Mini D1
int X = 128;
int motor_speed = 0;
int mode = 0;
bool isFirstConnect = true;

Servo servo;

unsigned long previousMillis = 0;

BLYNK_CONNECTED() {
if (isFirstConnect) {
Blynk.syncAll();
isFirstConnect = false;
}
}

void setup()
{
  Serial.begin(9600);
  Serial.println("\n Starting");

  Blynk.begin(auth, ssid, pass);

  long unsigned int mytimeout = millis() / 1000;
  while (Blynk.connect(1000) == false) {        // wait here until connected to the server
    if((millis() / 1000) > mytimeout + 24){      // try to connect to the server for less than 15 seconds
      break; 
    }
  }

  timer.setInterval(15000, reconnectBlynk); // check every 15 seconds if we are connected to the server

  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);

  servo.attach(D4, 800, 2520);
}

BLYNK_WRITE(V1)
{
 mode = param.asInt();
}

BLYNK_WRITE(V5)
{
 motor_speed = param.asInt();
}

BLYNK_WRITE(V6)
{
 X = param[0].asInt();
}

void loop() {

  if (mode == 1) {
    motor = motor_speed;
    analogWrite(D5, motor);
    analogWrite(D6, 0);
  } else {
    motor = motor_speed;
    analogWrite(D5, 0);
    analogWrite(D6, motor);
  }

  int m = map(X,0,255,158,22);

  servo.write(m);

  if (Blynk.connected()) {   // to ensure that Blynk.run() function is only called if we are still connected to the server
      Blynk.run();
  }
  
  timer.run();
}