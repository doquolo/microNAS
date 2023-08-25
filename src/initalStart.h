#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

// Set web server port number to 80
AsyncWebServer server(80);

const char inital_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <link rel="shortcut icon" href="server-solid.svg" type="image/x-icon" />
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #202124;
      color: #ffffff;
    }

    header {
      background-color: #1a1a1a;
      color: #ffffff;
      padding: 10px 20px;
    }

    header h1 {
      margin: 0;
    }

    .file-list {
      padding: 20px;
    }

    .file {
      display: flex;
      flex-direction: column;
      align-items: flex-start;
      border-bottom: 1px solid #303030;
      padding: 10px 0;
    }

    .file svg {
      width: 40px;
      height: 40px;
      margin-right: 10px;
      fill: white;
    }

    .file-name {
      margin-bottom: 5px;
      flex: 1;
    }

    .file-details {
      font-size: 12px;
      opacity: 0.7;
    }

    .file-title {
      font-size: 1rem;
      font-weight: bold;
    }

    .file > p, h1 {
        margin: 0rem 0rem 1rem 0rem;
    }

    .input-form {
      display: flex;
      flex-direction: column;
    }

    .section {
      display: flex;
      justify-content: space-between;
      margin-bottom: 0.5rem;
      align-items: center;
    }

    .section > input {
      margin-left: 1rem;
      background-color: #36383c;
      color: white;
      border: none;
      border-radius: 4px;
      padding: 6px 10px;
    }

    .section > label {
      color: white;
      margin-right: 10px;
    }

    .section > input[type="password"],
    .section > input[type="text"] {
      width: 60%;
    }

    .section > input:focus {
      outline: none;
      box-shadow: 0 0 4px 2px #1a73e8;
    }

    button {
      background-color: #1a73e8;
      color: white;
      border: none;
      border-radius: 4px;
      padding: 8px 16px;
      cursor: pointer;
    }

    button:hover {
      background-color: #155fad;
    }
  </style>
  <title>microNAS - Initial Setup</title>
</head>
<body>
  <header>
    <h1 id="title">Setup - microNAS</h1>
  </header>
  <div class="file-list">
    <div class="file">
      <h1 class="file-title">1. Access Point</h1>
      <form onsubmit="return false;" class="input-form">
        <div class="section">
          <label for="ssid_name">SSID</label>
          <input type="text" id="ssid" />
        </div>
        <div class="section">
          <label for="passwd">Password</label>
          <input type="password" id="password" />
        </div>
      </form>
    </div>
  </div>
  <div class="file-list">
    <div class="file" id="storage">
      <h1 class="file-title">2. Storage</h1>
      <!-- fill in with data from sdcard -->
      <!-- <p>Card type: SDHC</p>
      <p>Space: 64 MB used of 1024 MB</p>
      <progress id="file" value="32" max="100"></progress><p> -->
    </div>
  </div>
  <div class="file-list">
    <button onclick="saveInfo()">Save and Reboot</button>
  </div>
  <script>
    const storageData = document.querySelector("#storage");
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
      if (xhr.readyState == XMLHttpRequest.DONE) {
        storageData.textContent = xhr.responseText;
      }
    };
    xhr.open("GET", "/getStorageInfo", true);
    xhr.send(null);
  </script>
  <script>
    const saveInfo = () => {
      let ssid = document.querySelector("#ssid").value
      let password = document.querySelector("#password").value

      var xhr = new XMLHttpRequest();
      xhr.open("GET", `/configure?ssid=${ssid}&password=${password}`, true);
      xhr.send();

      alert("microNAS will now restart to apply the changes!");
      location.replace("/");
    }
  </script>
</body>
</html>
)rawliteral";

// TODO: merging this part into the main code
void initalStart() {

    // Replace with your network credentials
    const char* ssid = "pmeidtw >_< - Welcome";

    // begin softAP
    WiFi.softAP(ssid, NULL);

    // server route
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", inital_html);
    });

    server.on("/configure", HTTP_GET, [](AsyncWebServerRequest *request) {
        String ssid = "";
        String password = "";
        if (request->hasParam("ssid")) ssid += request->getParam("ssid")->value();
        if (request->hasParam("password")) password += request->getParam("password")->value();
        StaticJsonDocument<512> credentials;
        credentials["ssid"] = ssid;
        credentials["password"] = password;
        SD.remove("/credentials.json");
        File file = SD.open("/credentials.json", FILE_WRITE);
        if (!file) {
            Serial.println(F("Failed to create file"));
            return;
        }
        serializeJson(credentials, file);
        file.close();
        // reset esp32
        ESP.restart();
    });

    // TODO: add route for getting sdcard info

    // implement sdcard info
    server.begin();

    // prevent main program from running
    while (true);
}