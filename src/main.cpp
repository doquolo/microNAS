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

#include "cardNotFound.h"
#include "initalStart.h"

// declare sdcard module
const int chipSelect = 5;
File root;

H4 h4;

void h4setup() {
  Serial.begin(115200);
  // init sdcard
  Serial.print("Initializing SD card...");
  // if sd card initaliztion failed
  if (!SD.begin(chipSelect,SPI,4000000U,"/sd", 10U,false)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    // Replace with your network credentials
    const char* ssid     = "pmeidtw >_< - Error";
    // begin softAP
    WiFi.softAP(ssid, NULL);
    // Set web server port number to 80
    AsyncWebServer server(80);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", cardNotFound);
    });

    server.begin();
  }
  // if initalization completed
  else {
    Serial.println("initialization done.");
    // check if setup has been done
    if (!SD.exists("configure.json")) {
      initalStart();
    }

    // load configurtion to acquire AP information
    // Open file for reading
    File file = SD.open("configure.json");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);

    // begin softAP
    WiFi.softAP(doc["ssid"].as<String>().c_str(), doc["password"].as<String>().c_str());

    // Set web server port number to 80
    AsyncWebServer server(80);
    
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
}
