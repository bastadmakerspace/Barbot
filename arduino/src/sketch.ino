/////////////////////////////////////////////////////////////////////////////////////
///// BARBOT, MAKERSPACE                                            2019-11-21 ////// 
/////////////////////////////////////////////////////////////////////////////////////
                         
// VALID INPUT COMMANDS
// -{"TYPE":"MIX","P1":1,"P2":2,"P3":3,"P4":4,"P5":5,"P6":6,"P7":0,"P8":8,"P9":6}
// -{"TYPE":"MIX","P1":0,"P2":0,"P3":0,"P4":0,"P5":0,"P6":0,"P7":0,"P8":0,"P9":30}
// -{"TYPE":"STOP"}
// -{"TYPE":"LIGHT", "STATUS" : 1/0}

// RESPONSES
// -OK response after sent command             {"msg" : "command recieved"}
// -Response after receiving recepie           {"msg" : "recepie ok, starting"}
// -Response after sending stop command        {"msg" : "aborting recepie"}
// -Alert when drink is ready                  {"msg" : "drink ready"}
// -Error message for invali recepie           {"msg" : "invalid format, turning off"}
// -Response after sending light on command    {"msg" : "light on"}
// -Response after sending light off command   {"msg" : "light off"}

// -CONTINOUS FEEDBACK DURING TAPPING
// -Continous progress message while tapping   {"progress" : 0.00}
// -Continous progress message for pumps       {"pumps" : [1, 0, 1, 0, 1, 0, 1, 0, 1]}


//========================== INCLUDES ===============================================//
#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     22

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  30

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 30

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RBG + NEO_KHZ800);

#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;

//========================== DECLARATIONS ===========================================//
int pumpPin[9] = {4, 3, 5, 6, 7, 9, 8, 10, 11};
int lightPin = 13;
float pumpTime[9];
boolean pumpActive[] = {false, false, false, false, false, false, false, false, false};
long int timer;
boolean isTapping;
int lightStatus;
boolean newMessageArrived;
float totalTime;
float readyLevel = 0;

//========================== SETUP ==================================================//
void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  for (int i = 0 ; i < 9 ; i++) {
    pinMode(pumpPin[i], OUTPUT);
  }
  pinMode(lightPin, OUTPUT);
  pinMode(12, OUTPUT);
  turnOffAll();
  lightStatus = 0;
  isTapping = false;
  newMessageArrived = false;

  Wire.begin();

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  // sensor.startContinuous();

  rainbowFade2White(2, 2, 1);
}

//========================== LOOP ===================================================//
void loop() {
  digitalWrite(12, !digitalRead(11));
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
      toggleLight();
    }
    // Check if any pump should stop
    checkPumps();
  }
}

//========================== METHODS ==============================================//

void sendSerialMessage() {
  Serial.print("{\"progress\" : ");
  Serial.print(readyLevel);
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


void toggleLight(){
  // for (int i=0; i<6 ; i++){
  //   lightStatus = abs(lightStatus - 1);
  //   digitalWrite(lightPin, lightStatus);
  //   Serial.println(lightStatus);
  //   long int pause = millis();
  //   while((millis() - pause) < 300);
  // }
  rainbowFade2White(2, 2, 1);
  
  //Set white
  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
      strip.setPixelColor(i  , strip.Color(50, 50, 50)); // Draw new pixel
      // strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
      strip.show();
      delay(25);
  }
  strip.show();
}



void initRecepie() {
  timer = millis();
  for (int i = 0 ; i < 9 ; i++) {
    if (pumpTime[i] > 0) {
      digitalWrite(pumpPin[i], LOW);
      pumpActive[i] = true;
    } else {
      digitalWrite(pumpPin[i], HIGH);
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
    pumpTime[0] = (float)doc["P1"]*10;
    pumpTime[1] = (float)doc["P2"]*10;
    pumpTime[2] = (float)doc["P3"]*10;
    pumpTime[3] = (float)doc["P4"]*10;
    pumpTime[4] = (float)doc["P5"]*10;
    pumpTime[5] = (float)doc["P6"]*10;
    pumpTime[6] = (float)doc["P7"]*10;
    pumpTime[7] = (float)doc["P8"]*10;
    
    float p9 = (float)doc["P9"]*1;
    pumpTime[8] = max(0.0005*p9*p9 + 0.1901*p9 + 0.9, 0);
 
    totalTime = getTotalTime();
    isTapping = true;
    Serial.println("{\"msg\" : \"recepie ok, starting\"}");
  } else if (strcmp(msgType,"LIGHT") == 0) {
    int action = doc["STATUS"];
    Serial.println(action);
    if (action == 1){
      lightStatus = 1;  
      Serial.println("{\"msg\" : \"light on\"}");
      digitalWrite(lightPin, lightStatus);
    } else if(action == 0){
      lightStatus = 0; 
      Serial.println("{\"msg\" : \"light off\"}");
      digitalWrite(lightPin, lightStatus);
    } else {
      Serial.println("{\"msg\" : \"unknown command\"}");
    }
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
      digitalWrite(pumpPin[i], HIGH);
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
    digitalWrite(pumpPin[i], HIGH);
    pumpActive[i] = false;
    pumpTime[i] = 0;
  }
  isTapping = false;
  totalTime = 0;
}

void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops) {
  int fadeVal=0, fadeMax=100;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for(uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops*65536;
    firstPixelHue += 256) {

    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
        255 * fadeVal / fadeMax)));
    }

    strip.show();
    delay(wait);

    if(firstPixelHue < 65536) {                              // First loop,
      if(fadeVal < fadeMax) fadeVal++;                       // fade in
    } else if(firstPixelHue >= ((rainbowLoops-1) * 65536)) { // Last loop,
      if(fadeVal > 0) fadeVal--;                             // fade out
    } else {
      fadeVal = fadeMax; // Interim loop, make sure fade is at max
    }
  }

  // for(int k=0; k<whiteLoops; k++) {
  //   for(int j=0; j<256; j++) { // Ramp up 0 to 255
  //     // Fill entire strip with white at gamma-corrected brightness level 'j':
  //     strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
  //     strip.show();
  //   }
  //   delay(1000); // Pause 1 second
  //   for(int j=255; j>=0; j--) { // Ramp down 255 to 0
  //     strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
  //     strip.show();
  //   }
  // }

  // delay(500); // Pause 1/2 second
}
void pulseWhite(uint8_t wait) {
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }
}