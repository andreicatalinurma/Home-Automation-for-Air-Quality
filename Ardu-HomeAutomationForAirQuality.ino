#if defined(ESP8266)
#include "ESP8266WiFi.h"          //https://github.com/esp8266/Arduino
#else
#include "WiFi.h"
#endif

#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"
#include "ESPAsyncWiFiManager.h"        //https://github.com/tzapu/WiFiManager
#include "DHT.h"

// DEFINING PINS CONSTANTS //
#define RAINANALOG A0
#define DHTPIN D1
#define PIRPIN D2
#define WINDOWCONTACTSENSOR 10//D8
#define OPENWINDOWRELAY D5
#define CLOSEWINDOWRELAY D6
#define FANRELAY D7
#define OPENSWITCH D3
#define CLOSESWITCH D4
////////////////////////////

// Global Variables //
AsyncWebServer server(80);
DNSServer dns;

int pir;
int rain;
float humidity;
float temperature;
bool blockWindow;
bool isWindowClose;
bool waitFinished;
bool isFanStarted;
bool toggledFromUI;
bool window_status;
bool closeFirstTime;
bool isManualTriggeredOpen;
bool isManualTriggeredClose;
bool manuallyTriggerStartCounter;
bool manuallyTriggeredOpenIsTrue;
bool manuallyTriggeredClosedIsTrue;
bool manuallyTriggerUIStartCounter;
bool isOpenBasedOnHumidityNoRainAndNoMovement;
int BASE_TEMP = 20;
int BASE_RAIN = 1000;
int BASE_HUMIDITY = 60;
const long interval = 5000;  
unsigned long previousMillis = 0;
unsigned long startTime = 0;
//////////////////////

// Initialize DHT sensor.
DHT dht(DHTPIN, DHT22); 

void setup() {

  // SET RELAY PINS MODE //
  pinMode(OPENWINDOWRELAY, OUTPUT);
  pinMode(CLOSEWINDOWRELAY, OUTPUT);
  pinMode(FANRELAY, OUTPUT);
  /////////////////////////

  // SET WINDOWCONTACTSENSOR MODE //
  pinMode(WINDOWCONTACTSENSOR, INPUT_PULLUP);
  //////////////////////////////////

  // SET DHT & PIR SENSORS MODE //
  pinMode(PIRPIN, INPUT_PULLUP);
  pinMode(DHTPIN, INPUT);
  ////////////////////////////////

  // SET MANUAL SWITCH PINS MODE //
  pinMode(OPENSWITCH, INPUT_PULLUP);
  pinMode(CLOSESWITCH, INPUT_PULLUP);
  /////////////////////////////////

  // SET RELAY STATE TO HIGH SINCE IT IS LOW BY DEFAULT //
  digitalWrite(OPENWINDOWRELAY, HIGH);
  digitalWrite(CLOSEWINDOWRELAY, HIGH);
  digitalWrite(FANRELAY, HIGH);
  ////////////////////////////////////////////////////////

  // SET FIRST STATE FOR BOOLEANS //
  closeFirstTime = true;
  blockWindow = false;
  isFanStarted = false;
  waitFinished = true;
  toggledFromUI = false;
  isOpenBasedOnHumidityNoRainAndNoMovement = false;
  //////////////////////////////////

  // START READING DHT SENSOR DATA //
  dht.begin();              
  ///////////////////////////////////
  
  // START SERIAL //
  Serial.begin(115200);
  //////////////////
  
  // START WIFI CONFIGURATION PORTAL //
  AsyncWiFiManager wifiManager(&server, &dns);
  /////////////////////////////////////
  
  // CONECT TO WIFI MANUALLY //
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//  }
  /////////////////////////////

  // CONECT TO WIFI TROUGH PORTAL //
    wifiManager.autoConnect("Window Home Automation");
  //////////////////////////////////
  
  // INITIALIZE SPIFFS FOR READING DATA UPLOADED FILES FROM BOARD //
  if (!SPIFFS.begin()) {
    return;
  }
  //////////////////////////////////////////////////////////////////

  // SET ROUTE FOR / WEB PAGE READ FROM SPIFFS //
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String());
  });
  ///////////////////////////////////////////////

  // SET ROUTE FOR STYLE.CSS FILE READ FROM SPIFFS //
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  ///////////////////////////////////////////////////

  // SET ROUTE FOR SCRIPT.JS FILE READ FROM SPIFFS //
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });
  ///////////////////////////////////////////////////

  // SET ROUTE FOR READING OPEN WINDOW (o-w.png) IMAGE FROM SPIFFS //
  server.on("/o-w", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/o-w.png", "image/png");
  });
  ///////////////////////////////////////////////////////////////////

  // SET ROUTE FOR READING CLOSE WINDOW (c-w.png) IMAGE FROM SPIFFS //
  server.on("/c-w", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/c-w.png", "image/png");
  });
  ////////////////////////////////////////////////////////////////////
  
  // SET ROUTE FOR RESET PAGE FROM SPIFFS //
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/reset.html", String());
  });
  //////////////////////////////////////////

  // SET ROUTE FOR STATUS PAGE FROM SPIFFS //
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/status.html",String());
  });
  //////////////////////////////////////////

  // SET ROUTE FOR WINDOWSTATUS PAGE FROM SPIFFS //
  server.on("/windowstatus", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/windowstatus.html",String());
  });
  /////////////////////////////////////////////////
  
  // SET ROUTE FOR CUSTOMIZE PAGE FROM SPIFFS //
  server.on("/customize", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/customize.html",String());
  });
  //////////////////////////////////////////////

  // SET ROUTE FOR ABOUT PAGE FROM SPIFFS //
  server.on("/about", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/about.html",String());
  });
  //////////////////////////////////////////

  // SET ROUTE FOR HELP PAGE FROM SPIFFS //
  server.on("/help", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/help.html",String());
  });
  /////////////////////////////////////////

  // SET ROUTE FOR READING TEMPERATURE ENDPOINT FROM UI //
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(temperature));
  });
  ////////////////////////////////////////////////////////

  // SET ROUTE FOR READING HUMIDITY ENDPOINT FROM UI //
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(humidity));
  });
  ////////////////////////////////////////////////////////

  // SET ROUTE FOR READING RAIN ENDPOINT FROM UI //
  server.on("/rain", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(rain));
  });
  /////////////////////////////////////////////////

  // SET ROUTE FOR READING PIR ENDPOINT FROM UI //
  server.on("/pir", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(pir));
  });
  ////////////////////////////////////////////////

  // SET ROUTE FOR READING WINDOWCONTACTSENSOR ENDPOINT FROM UI //
  server.on("/windowstate", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(isWindowClose));
  });
  ////////////////////////////////////////////////////////////////

  // SET ROUTE FOR READING BASE SENSORS VALUES ENDPOINT FROM UI //
  server.on("/getCustomizeValues", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(BASE_TEMP) + ";" + String(BASE_HUMIDITY) + ";" + String(BASE_RAIN));
  });
  ////////////////////////////////////////////////////////////////

  // SET ROUTE FOR READING BASE SENSORS VALUES ENDPOINT FROM UI //
  server.on("/getBlockWindow", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(blockWindow));
  });
  ////////////////////////////////////////////////////////////////

  // SET ROUTE FOR SETTING BASE SENSORS VALUES ENDPOINT FROM UI AND RETURN THE CUSTOMIZE PAGE //
  server.on("/submitCustomValues", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("input_temperature")) {
      if(request->getParam("input_temperature")->value() != "")
      {
        int input_temperature = request->getParam("input_temperature")->value().toInt();
        if(input_temperature && input_temperature >= 10 && input_temperature <= 49)
        {
          BASE_TEMP = request->getParam("input_temperature")->value().toInt();
        }
      }
    }
    if (request->hasParam("input_humidity")) {
      if(request->getParam("input_humidity")->value() != "")
      {
        int input_humidity = request->getParam("input_humidity")->value().toInt();
        if(input_humidity && input_humidity >= 1 && input_humidity <= 100)
        {
          BASE_HUMIDITY = request->getParam("input_humidity")->value().toInt();
        }
      }
    }
    if (request->hasParam("input_rain")) {
      if(request->getParam("input_rain")->value() != "")
      {
        int input_rain = request->getParam("input_rain")->value().toInt();
        if(input_rain && input_rain >= 200 && input_rain <= 1000)
        {
          BASE_RAIN = request->getParam("input_rain")->value().toInt();
        }
      }
    }
    request->send(SPIFFS, "/customize.html",String());
  });
  //////////////////////////////////////////////////////////////////////////////////////

  // SET ROUTE FOR TOGGLE ENDPOINT FROM UI AND RETURN THE WINDOW STATUS PAGE //
  server.on("/toggleWindow", HTTP_GET, [](AsyncWebServerRequest * request) {
    toggledFromUI = true;
    startTime = millis();
    request->send(SPIFFS, "/windowstatus.html",String());
  });
  /////////////////////////////////////////////////////////////////////////////

  // SET ROUTE FOR BLOCKWINDOW ENDPOINT FROM UI AND RETURN THE WINDOW STATUS PAGE //
  server.on("/blockWindow", HTTP_GET, [](AsyncWebServerRequest * request) {
    blockWindow = !blockWindow;
    request->send(SPIFFS, "/windowstatus.html",String());
  });
  //////////////////////////////////////////////////////////////////////////////////

  // SET ROUTE FOR TOGGLERESET ENDPOINT FROM UI //
  server.on("/toggleReset", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputMessage;
    if (request->hasParam("state"))
    {
      inputMessage = request->getParam("state")->value();
      if (inputMessage == "1"){
        ESP.reset();
      }
    }
    else
    {
      inputMessage = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });
  /////////////////////////////////////////////////

  // START ACCEPTING INCOMMING CONNECTIONS //
  server.begin();
  ///////////////////////////////////////////
}

void loop() {

  unsigned long currentMillis = millis(); // Returns the number of milliseconds passed since the Arduino board began running the current program
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      temperature = newT;
    }
    else {
      temperature = newT;
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      humidity = newH;
    }
    else {
      humidity = newH;
    }

    // read rain sensor values from analog
    rain = analogRead(RAINANALOG);
    // read pir sensor values from digital
    pir = digitalRead(PIRPIN);
    // read windowcontactsensor values from digital
    isWindowClose = (digitalRead(WINDOWCONTACTSENSOR) == LOW);
  }
  // wait 1 second after reads
  delay(1000);

  // WINDOW & FAN LOGIC //
  
  // if blocking the window was not requested
  if(!blockWindow)
  {
    // Close the window if it is open when the board is reseted //
    if(closeFirstTime && !isWindowClose)
    { 
      closeWindow();
    }
    closeFirstTime = false;
    ///////////////////////////////////////////////////////////

    // read manual switches from digital
    isManualTriggeredOpen = (digitalRead(OPENSWITCH) == LOW);
    isManualTriggeredClose = (digitalRead(CLOSESWITCH) == LOW);
    
    // MANUAL OPEN SWITCH LOGIC //
    if(isManualTriggeredOpen)//
    {
        delay(200); 
        digitalWrite(OPENWINDOWRELAY, LOW); // command relay input trough digitalWrite
        stopFan(); // stop the fan since window is open
        manuallyTriggeredOpenIsTrue = true;
        manuallyTriggerStartCounter = true;
        isOpenBasedOnHumidityNoRainAndNoMovement = true;
        startTime = millis(); // Returns the number of milliseconds passed since the Arduino board began running the current program
    }
    else if(manuallyTriggeredOpenIsTrue)
    {
        // set relay state to high after manual switch is released
        manuallyTriggeredOpenIsTrue = false; 
        delay(200);
        digitalWrite(OPENWINDOWRELAY, HIGH);
    } 
    //////////////////////////////

    // MANUAL OPEN SWITCH LOGIC //
    if(isManualTriggeredClose)
    {
      manuallyTriggeredClosedIsTrue = true;
      delay(200);
      digitalWrite(CLOSEWINDOWRELAY, LOW); // command relay input trough digitalWrite
      waitFinished = true;
      manuallyTriggerStartCounter = false;
      manuallyTriggerUIStartCounter = false;
    }
    else if(manuallyTriggeredClosedIsTrue) {
      // set relay state to high after manual switch is released
      manuallyTriggeredClosedIsTrue = false;
      delay(200);
      digitalWrite(CLOSEWINDOWRELAY, HIGH);
    }
    ///////////////////////////////

  // COUNTER LOGIC FOR WAITING SPECIFIED TIME AFTER WINDOW WAS OPENED //
  if(manuallyTriggerStartCounter || manuallyTriggerUIStartCounter)
  {
    waitFinished = wait(120000); //540000 // wait method counter the waiting time
  }
  //////////////////////////////////////////////////////////////////////
  
  // UI TOGGLE BUTTON LOGIC //
   if(toggledFromUI)
    {
      if(isWindowClose && waitFinished)
      {
        stopFan(); // stop the fan if it's running
        openWindow(); // open the window
        toggledFromUI = false;
        manuallyTriggerUIStartCounter = true;
        waitFinished = false;
      }
      else if(!isWindowClose) // if the window is open
      {
        closeWindow(); // close the window
        waitFinished = true;
        toggledFromUI = false;
        manuallyTriggerUIStartCounter = false;
        manuallyTriggerStartCounter = false;
      }
    }
  //////////////////////
  
  // SENSOR LOGIC //
    if(waitFinished == true) // if the time specified by open from toggle is passed start the logic
    {
      manuallyTriggerStartCounter = false;
      manuallyTriggerUIStartCounter = false;
  
      if(temperature < BASE_TEMP || rain < BASE_RAIN) // if the indor temperature is too low or is rainig
      {
        if(!isWindowClose && isOpenBasedOnHumidityNoRainAndNoMovement)
        {
          closeWindow(); // close the window
        }
        if(humidity > BASE_HUMIDITY) // if there is humidity indor
        {
          if(!isFanStarted) // and the fan is not started
          {
            startFan(); // start the fan
          }
        }
        else
        {
          if(isFanStarted) // if there is no humidity
          {
            stopFan(); // stop the fan
          }
        }
      }
      else 
      {
        // logic if the indor temperature is higher
        if(humidity > BASE_HUMIDITY && rain > BASE_RAIN && pir == 0) // if there is humidity inside, is not raining and there is no movement
        {
          if(!isOpenBasedOnHumidityNoRainAndNoMovement)
          {
            openWindow(); // open the window
          }
        }
        else 
        {
          // if the window is open base on humidity, no rain and no movement and the temperature is lower and there is humidity
          if(isOpenBasedOnHumidityNoRainAndNoMovement && temperature < BASE_TEMP && humidity > BASE_HUMIDITY) 
          {
            closeWindow(); // close the window
            startFan(); // start the fan
          }
        }
      }
    }
  }
  else 
  {
    // if block window was requested and the window is open
    if(!isWindowClose)
    {
      closeWindow();// close the window
    }
  }
}
////////////////////////////////

// OPEN WINDOW METHOD //
void openWindow() {
      stopFan(); // stop the fan if the open window was requested
      digitalWrite(OPENWINDOWRELAY, LOW); // set the relay state to low in order to trigger the oppening of the trough digitalWrite
      delay(10000); // wait 10 seconds in order for the actuator to be fully retracted
      digitalWrite(OPENWINDOWRELAY, HIGH); // set the relay state to high in order to stop the oppening command trough digitalWrite
      isOpenBasedOnHumidityNoRainAndNoMovement = true;
}
///////////////////////

void closeWindow() {
      digitalWrite(CLOSEWINDOWRELAY, LOW); // set the relay state to low in order to trigger the closing of the windows trough digitalWrite
      delay(10000); // wait 10 seconds in order for the actuator to be fully retracted
      digitalWrite(CLOSEWINDOWRELAY, HIGH); // set the relay state to high in order to stop the closing command trough digitalWrite
      isOpenBasedOnHumidityNoRainAndNoMovement = false;
}


void startFan() {
    isFanStarted = true;
    digitalWrite(FANRELAY, LOW); // set the relay state to low in order to start the fan, trough digitalWrite
}

void stopFan() {
    isFanStarted = false; // set the relay state to high in order to stop the fan, trough digitalWrite
    digitalWrite(FANRELAY, HIGH);
}

bool wait(unsigned long duration)
{
  unsigned long currentMillis = millis(); // store the current time
  if(currentMillis - startTime >= duration)
  {
    startTime = 0;
    return true;
  }
  // indicate to caller that wait period is in progress
  return false;
}
