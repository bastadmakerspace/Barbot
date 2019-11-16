

// VALID MESAGE FORMATS
// {"TYPE":"MIX","P1":1,"P2":2,"P3":3,"P4":4,"P5":5,"P6":6,"P7":7,"P8":8,"P9":30}
// {"TYPE":"STOP"}

//========================== INCLUDES ===============================================//
#include <ArduinoJson.h>

//========================== DECLARATIONS ===========================================//
int pumpPin[9] = {3, 4, 5, 6, 7, 8, 9, 10, 11};
int pumpTime[9];
boolean pumpActive[] = {false, false, false, false, false, false, false, false, false};
long int timer;
boolean isTapping;
boolean newMessageArrived;
float totalTime;
float readyLevel = 0;

//========================== SETUP ==================================================//
void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  for (int i = 0 ; i < 9 ; i++) {
    pinMode(pumpPin[i], OUTPUT);
  }
  turnOffAll();
  isTapping = false;
  newMessageArrived = false;
}

//========================== LOOP ===================================================//
void loop() {
  // Wait for command from PI
  while (Serial.available()) {
    String msg = Serial.readString(); // read the incoming data as string
    Serial.println("{\"msg\" : \"command recieved\"}");
    parseMessage(msg);
  }
  // Initiate pumps if recepie arrived
  if (newMessageArrived) {
    initRecepie();
    newMessageArrived = false;
  }
  if (isTapping) {
    // Check if any pump should be turned off
    readyLevel = round(min(((float)millis() - (float)timer) / totalTime, 1000) / 10);
    // Notify about current status
    sendSerialMessage();
    // Check if drink is ready
    if (!atLeastOnePumpOn()) {
      isTapping = false;
      Serial.println("{\"msg\" : \"drink ready\"}");
    }
    // Check if any pump should stop
    checkPumps();
  }
}

//========================== METHODS ==============================================//

void sendSerialMessage() {
  Serial.print("{\"progress\" : ");
  Serial.print((String)readyLevel);
  Serial.println("}");
  Serial.print("{\"pumps\" : [");
  for (int i = 0 ; i < 9 ; i++) {
    Serial.print(pumpActive[i]);
    if (i < 8) {
      Serial.print(", ");
    }
  }
  Serial.println("]}");
}


void initRecepie() {
  timer = millis();
  for (int i = 0 ; i < 9 ; i++) {
    if (pumpTime[i] > 0) {
      digitalWrite(pumpPin[i], HIGH);
      pumpActive[i] = true;
    } else {
      digitalWrite(pumpPin[i], LOW);
      pumpActive[i] = false;
    }
  }
}


float getTotalTime() {
  float maxTime = 0;
  for (int i = 0 ; i < 9 ; i++) {
    if (pumpTime[i] > maxTime) {
      maxTime = (float)pumpTime[i];
    }
  }
  return maxTime;
}


void parseMessage(String msg) {
  DynamicJsonDocument doc(1024);
  JsonObject root = doc.as<JsonObject>();
  DeserializationError error = deserializeJson(doc, msg);
  // Parsing.
  if (error) {
    turnOffAll();
    Serial.print("{\"msg\" : \"invalid format, turning off\"} ");
    return;
  }
  // Fetch values.
  const char* msgType = doc["TYPE"];
  if (strcmp(msgType,"MIX") == 0) {
    pumpTime[0] = doc["P1"];
    pumpTime[1] = doc["P2"];
    pumpTime[2] = doc["P3"];
    pumpTime[3] = doc["P4"];
    pumpTime[4] = doc["P5"];
    pumpTime[5] = doc["P6"];
    pumpTime[6] = doc["P7"];
    pumpTime[7] = doc["P8"];
    pumpTime[8] = doc["P9"];
    totalTime = getTotalTime();
    isTapping = true;
    Serial.println("{\"msg\" : \"recepie ok, starting\"}");
  } else if (strcmp(msgType,"STOP") == 0) {
    turnOffAll();
    Serial.println("{\"msg\" : \"aborting recepie\"}");
  } else {
    turnOffAll();
    Serial.println("{\"msg\" : \"unknown command\"}");
  }
  // Set status
  newMessageArrived = true;
}


void checkPumps() {
  long int elapsedTime = (millis() - timer);
  for (int i = 0 ; i < 9 ; i++) {
    if ((pumpTime[i] * 1000) < elapsedTime) {
      digitalWrite(pumpPin[i], LOW);
      pumpActive[i] = false;
    }
  }
}


boolean atLeastOnePumpOn() {
  boolean anyOn = false;
  for (int i = 0 ; i < 9 ; i++) {
    if (pumpActive[i]) {
      anyOn = true;
      break;
    }
  }
  return anyOn;
}


void turnOffAll() {
  for (int i = 0 ; i < 9 ; i++) {
    digitalWrite(pumpPin[i], LOW);
    pumpActive[i] = false;
    pumpTime[i] = 0;
    isTapping = false;
    totalTime = 0;
  }
}
