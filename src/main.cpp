// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <H4.h>

// declare sdcard module
const int chipSelect = 5;
File root;

H4 h4;

// Replace with your network credentials
const char* ssid     = "pmeidtw >_<";
const char* password = "uwu012345";

// Set web server port number to 80
AsyncWebServer server(80);

void h4setup() {
  // init serial
  Serial.begin(115200);
  
  // init sdcard
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect,SPI,4000000U,"/sd", 10U,false)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true);
  }
  Serial.println("initialization done.");
  
  // begin softAP
  WiFi.softAP(ssid, password);
  
  // acquire esp32 ip address
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // route
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/index.html", "text/html");
  });

  server.on("/queryAllFiles", HTTP_GET, [](AsyncWebServerRequest *request) {

    String folder = "/media";
    if (request->hasParam("folder")) folder += request->getParam("folder")->value();

    // cerate json object
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument fileinfo (4096);
    
     // list sd directory
    File root = SD.open(folder);
    
    if(!root){
      Serial.println("Failed to open directory");
      return;
    }
    if(!root.isDirectory()){
      Serial.println("Not a directory");
      return;
    }

    JsonArray files = fileinfo.to<JsonArray>();

    File file = root.openNextFile();
    
    while(file){
      if (file.isDirectory()) {
        JsonArray array = files.createNestedArray();
        array.add(String(file.name()));
        array.add("folder");
        array.add("0");
      } 
      else {
        JsonArray array = files.createNestedArray();
        array.add(String(file.name()));
        array.add("file");
        array.add(String(file.size()));
      }
      file = root.openNextFile();
    }
  
    // send json
    serializeJson(fileinfo, *response);
    request->send(response);
  });

  
  server.on("/file", HTTP_GET, [](AsyncWebServerRequest *request) {
     h4.queueFunction(std::bind([](AsyncWebServerRequest *request){ 
      String filename = "";
      String path = "";
      String isDelete = "";
      if (request->hasParam("name")) filename += request->getParam("name")->value();
      if (request->hasParam("path")) path += request->getParam("path")->value();
      if (request->hasParam("action")) isDelete += request->getParam("action")->value();
      

      if (filename != "") {
        filename = "/media" + path + "/" + filename;
        Serial.println(filename);
        if (isDelete == "delete") SD.remove(filename);
        else request->send(SD, filename);
      } 
    },request));
  });

  // begin serving static files
  server.serveStatic("/", SD, "/");
  // begin server
  server.begin();
}
