#include <ArduinoJson.h>

String msg;
int P1, P2, P3, P4, P5, P6, P7, P8, P9;
boolean P1on, P2on, P3on, P4on, P5on, P6on, P7on, P8on, P9on;
int pumpPin1 = 3;
int pumpPin2 = 4;
int pumpPin3 = 5;
int pumpPin4 = 6;
int pumpPin5 = 7;
int pumpPin6 = 8;
int pumpPin7 = 9;
int pumpPin8 = 10;
int pumpPin9 = 11;
long int timer;
boolean isTapping;
boolean newMessageArrived;


void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  pinMode(pumpPin1, OUTPUT);
  pinMode(pumpPin2, OUTPUT);
  pinMode(pumpPin3, OUTPUT);
  pinMode(pumpPin4, OUTPUT);
  pinMode(pumpPin5, OUTPUT);
  pinMode(pumpPin6, OUTPUT);
  pinMode(pumpPin7, OUTPUT);
  pinMode(pumpPin8, OUTPUT);
  pinMode(pumpPin9, OUTPUT);
  turnOffAll();
  isTapping = false;
  newMessageArrived = false;
}


void loop() {
  // Wait for command from PI
  while (Serial.available()) {
    msg = Serial.readString(); // read the incoming data as string
    Serial.print("Recieved message : ");
    Serial.println(msg);
    parseMessage(msg);
  }

  if (newMessageArrived) {
    // Initiate new recepie from json
    // turnOffAll();
    initRecepie();
    timer = millis();
    newMessageArrived = false;
  }
  
  if (isTapping) {
    // Check if any pump should be turned off
    if (!atLeastOnePumpOn()) {
      isTapping = false;  
      Serial.println("DRINK READY!!!");
    }
    // Check if any pump should stop
    checkPumps();
  }
}

void checkPumps() {
  long int elapsedTime = (millis()-timer);
  Serial.print("IS TAPPING...");
  Serial.println(isTapping);


  if((P1*1000) < elapsedTime){
    digitalWrite(pumpPin1, LOW);
    P1on = false;
    Serial.println("PUMP 1 OFF!");
  }
    if((P2*1000) < elapsedTime){
    digitalWrite(pumpPin2, LOW);
    P2on = false;
    Serial.println("PUMP 2 OFF!");
  }
    if((P3*1000) < elapsedTime){
    digitalWrite(pumpPin3, LOW);
    P3on = false;
    Serial.println("PUMP 3 OFF!");
  }
    if((P4*1000) < elapsedTime){
    digitalWrite(pumpPin4, LOW);
    P4on = false;
    Serial.println("PUMP 4 OFF!");
  }
    if((P5*1000) < elapsedTime){
    digitalWrite(pumpPin5, LOW);
    P5on = false;
    Serial.println("PUMP 5 OFF!");
  }
    if((P6*1000) < elapsedTime){
    digitalWrite(pumpPin6, LOW);
    P6on = false;
    Serial.println("PUMP 6 OFF!");
  }
    if((P7*1000) < elapsedTime){
    digitalWrite(pumpPin7, LOW);
    P7on = false;
    Serial.println("PUMP 7 OFF!");
  }
    if((P8*1000) < elapsedTime){
    digitalWrite(pumpPin8, LOW);
    P8on = false;
    Serial.println("PUMP 8 OFF!");
  }
    if((P9*1000) < elapsedTime){
    digitalWrite(pumpPin9, LOW);
    P9on = false;
    Serial.println("PUMP 9 OFF!");
  }
  
}

boolean atLeastOnePumpOn(){
  return (P1on || P2on || P3on || P4on || P5on || P6on || P7on || P8on || P9on);
}

void turnOffAll() {
  digitalWrite(pumpPin1, LOW);
  P1on = false;
  Serial.println("PUMP 1 OFF!");
  digitalWrite(pumpPin2, LOW);
  P2on = false;
  Serial.println("PUMP 2 OFF!");
  digitalWrite(pumpPin3, LOW);
  P3on = false;
  Serial.println("PUMP 3 OFF!");
  digitalWrite(pumpPin4, LOW);
  P4on = false;
  Serial.println("PUMP 4 OFF!");
  digitalWrite(pumpPin5, LOW);
  P5on = false;
  Serial.println("PUMP 5 OFF!");
  digitalWrite(pumpPin6, LOW);
  P6on = false;
  Serial.println("PUMP 6 OFF!");
  digitalWrite(pumpPin7, LOW);
  P7on = false;
  Serial.println("PUMP 7 OFF!");
  digitalWrite(pumpPin8, LOW);
  P8on = false;
  Serial.println("PUMP 8 OFF!");
  digitalWrite(pumpPin9, LOW);
  P9on = false;
  Serial.println("PUMP 9 OFF!");
}

void initRecepie() {
  if (P1 > 0) {
    digitalWrite(pumpPin1, HIGH);
    P1on = true;
    Serial.println("PUMP 1 ON!");
  }
  if (P2 > 0) {
    digitalWrite(pumpPin2, HIGH);
    P2on = true;
    Serial.println("PUMP 2 ON!");
  }
  if (P3 > 0) {
    digitalWrite(pumpPin3, HIGH);
    P3on = true;
    Serial.println("PUMP 3 ON!");
  }
  if (P4 > 0) {
    digitalWrite(pumpPin4, HIGH);
    P4on = true;
    Serial.println("PUMP 4 ON!");
  }
  if (P5 > 0) {
    digitalWrite(pumpPin5, HIGH);
    P5on = true;
    Serial.println("PUMP 5 ON!");
  }
  if (P6 > 0) {
    digitalWrite(pumpPin6, HIGH);
    P6on = true;
    Serial.println("PUMP 6 ON!");
  }
  if (P7 > 0) {
    digitalWrite(pumpPin7, HIGH);
    P7on = true;
    Serial.println("PUMP 7 ON!");
  }
  if (P8 > 0) {
    digitalWrite(pumpPin8, HIGH);
    P8on = true;
    Serial.println("PUMP 8 ON!");
  }
  if (P9 > 0) {
    digitalWrite(pumpPin9, HIGH);
    P9on = true;
    Serial.println("PUMP 9 ON!");
  }
}


void parseMessage(String msg) {
  DynamicJsonDocument doc(1024);
  JsonObject root = doc.as<JsonObject>();
  DeserializationError error = deserializeJson(doc, msg);

  // Parsing.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Fetch values.
  const char* msgType = doc["TYPE"];
  P1 = doc["P1"];
  P2 = doc["P2"];
  P3 = doc["P3"];
  P4 = doc["P4"];
  P5 = doc["P5"];
  P6 = doc["P6"];
  P7 = doc["P7"];
  P8 = doc["P8"];
  P9 = doc["P9"];

 
  isTapping = true;
  newMessageArrived = true;
  Serial.print("IS TAPPING...");
  Serial.println(isTapping);

}
