#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#define DEBUG true  // set to true for debug output, false for no debug output
#define DEBUG_SERIAL if(DEBUG) Serial

// Written by Nicolás Paila, 2023
// To upload to the ESP866, type 'pio run -t upload -e rc_car_receiver' into the terminal

// TODO
// - Add a validation function for received packets
// - Define a struct for recieved data

// Car actuator objects and properties
int wheelDirection = 128; // the wheel direction value goes from 0 to 255, 128 is middle position
int motorSpeed = 255; // the motor speed value goes from 0 to 255, 255 is full throttle
Servo steeringServo; // servo object assigned to the servo steering system 
const int BUTTON_PRESSED = 1;

// All incoming packets should be arrays of numerical characters, 
// Each array is composed of five elements or 'bits'
// The first three bits represent the absolute axis position of the steering system, from '000' to '255'
// The fourth bit represents the backward state of the engine system, can either be '0' or '1'
// The fifth bit represents the forward state of the engine system, can either be '0' or '1'
// For example, the incoming packet array '25501' represents forward engine throttle at maximum right angle

// Here each bit of the incoming packets are assigned
const int AXIS_POSITION_BITS[] = {0, 1, 2};
const int BACKWARD_BUTTON_BIT = 3;
const int FORWARD_BUTTON_BIT = 4;
const int AXIS_POSITION_BITCOUNT = sizeof(AXIS_POSITION_BITS);

// WiFi credentials
const char ssid[] = "*****";
const char password[] = "*****";
char incomingPacket[255];

// ESP8266 IP address and port
IPAddress localIP(192, 168, 18, 78);
const int localPort = 22495;

WiFiUDP Udp;

void printWifiConnection() {
  // Print the SSID of the connected network
  DEBUG_SERIAL.print("SSID: ");
  DEBUG_SERIAL.println(WiFi.SSID());

  // Print the ESP8266 IP address:
  IPAddress ip = WiFi.localIP();
  DEBUG_SERIAL.print("IP Address: ");
  DEBUG_SERIAL.println(ip);
}

void connectToWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUG_SERIAL.println("Connecting to WiFi...");
  }
  DEBUG_SERIAL.println("Connected to WiFi");
  printWifiConnection();
}

void turnOnBuiltinLed() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void setupMechanicalObjects() {
  // Initialize pin D5 and D6 as the motor's terminals
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);

  // Attach the servo with an acceptable pulse width range per ESP8266 hardware specification,
  // by contrast, the Arduino defaults are 544 and 2400 respectively
  steeringServo.attach(D4, 800, 2520);
}

void readPacket() {
  int len = Udp.read(incomingPacket, 255);
  if (len > 0) {
    incomingPacket[len] = 0;
  }
}

int parseAxisPosition() {
  char buffer[AXIS_POSITION_BITCOUNT + 1];

  for (unsigned int i = 0; i < AXIS_POSITION_BITCOUNT; i++) {
    buffer[i] = incomingPacket[i];
  }
  buffer[AXIS_POSITION_BITCOUNT] = '\0'; // this end character is necessary to split the buffer

  int axisPosition = atoi(buffer);
  return axisPosition;
}

bool isButtonPressed(int button) {
  return button == BUTTON_PRESSED;
}

void sendSteeringSignal() {
  int axisPosition = parseAxisPosition();
  // The values 158 and 22 represent the maximum physical 
  // values that my steering system is designed for
  int mappedValue = map(axisPosition,0,255,158,22);
  steeringServo.write(mappedValue);
}

int parseButton(int bit) {
  return incomingPacket[bit]-'0'; // converts char to int
}

void sendMotorSignal() {
  int exButton = parseButton(FORWARD_BUTTON_BIT);
  int squareButton = parseButton(BACKWARD_BUTTON_BIT);

  if (isButtonPressed(exButton)) {
      // Forward state of the engine system
      analogWrite(D5, motorSpeed);
      analogWrite(D6, 0);
  } else if (isButtonPressed(squareButton)) {
      // Backward state of the engine system
      analogWrite(D5, 0);
      analogWrite(D6, motorSpeed);
  } else {
      // Motor is stopped
      analogWrite(D5, 0);
      analogWrite(D6, 0);
  }
}

bool hasPacketBeenReceived() {
  // Check for incoming packets
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    return true;
  } else {
    return false;
  }
}

void setup() {
  DEBUG_SERIAL.begin(9600);

  connectToWifi();
  turnOnBuiltinLed();
  setupMechanicalObjects();

  // Begin listening for incoming UDP packets
  Udp.begin(localPort);
}

void loop() {
  if (hasPacketBeenReceived()) {
    readPacket();
    sendMotorSignal();
    sendSteeringSignal();
  }
}

