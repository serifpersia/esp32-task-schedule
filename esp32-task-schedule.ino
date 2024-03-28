/*
  MIT License

  Copyright (c) 2024 serifpersia

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// Local netowork credentials
const char* ssid = "network-ssid";
const char* password = "network-password";

bool isSystemEnabled;
String timeSet;
uint8_t durationSet;

#define LED_PIN 2

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const int utcOffsetHours = 1; // UTC+1 hour
const int daylightOffsetHours = 0; // No daylight saving time

// Calculate the total offset in seconds
const int totalOffsetSeconds = (utcOffsetHours * 3600) + (daylightOffsetHours * 3600);

unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 60000; // Check time every minute

unsigned long activityStartTime = 0;
bool activityStarted = false;

// Get current time after updating NTP client
String currentTime;
String currentHour;
String currentMinute;
String currentSeconds;

AsyncWebServer server(80);
WebSocketsServer webSocket(81);

// Function to load configuration from file
void loadConfig() {
  File configFile = SPIFFS.open("/config.cfg", "r");
  if (!configFile) {
    Serial.println("Failed to open config file for reading");
    return;
  }

  String line;
  while (configFile.available()) {
    line = configFile.readStringUntil('\n');
    int colonIndex = line.indexOf(':');
    if (colonIndex != -1) {
      String key = line.substring(0, colonIndex);
      String value = line.substring(colonIndex + 1);
      value.trim();

      if (key == "Time set") {
        timeSet = value;
      } else if (key == "Duration in seconds") {
        durationSet = value.toInt();
      } else if (key == "System enabled") {
        isSystemEnabled = value.toInt();
      }
    }
  }

  configFile.close();

  Serial.println("Loaded configuration:");
  Serial.println("Time set: " + timeSet);
  Serial.println("Duration in seconds: " + String(durationSet));
  Serial.println("System enabled: " + String(isSystemEnabled));
}


// Function to save configuration to file
void saveConfig() {
  File configFile = SPIFFS.open("/config.cfg", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  configFile.println("Time set: " + timeSet);
  configFile.println("Duration in seconds: " + String(durationSet));
  configFile.println("System enabled: " + String(isSystemEnabled));

  configFile.close();

  Serial.println("Configuration saved:");
  Serial.println("Time set: " + timeSet);
  Serial.println("Duration in seconds: " + String(durationSet));
  Serial.println("System enabled: " + String(isSystemEnabled));
}

void setup() {
  Serial.begin(115200);

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to Wi-Fi. Local IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize NTP client
  timeClient.begin();
  updateTime();

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load configuration from file
  loadConfig();

  // Serve HTML file
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(handleWebSocketMessage);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  webSocket.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - lastCheckTime >= checkInterval) {
    updateTime();
    lastCheckTime = currentMillis;
  }

  // Get current time after updating NTP client
  currentTime = timeClient.getFormattedTime();

  // Check if the current time matches the time set by the user and the system is enabled
  if (currentTime == timeSet + ":00" && isSystemEnabled) {
    // Check if the seconds are in the range [0, 9]
    if (!activityStarted) {
      runActivity();
      activityStarted = true;
      activityStartTime = currentMillis; // Set the activity start time
    }
  } else {
    // If the activity was started and the seconds are not in the range [0, 9], stop it
    if (activityStarted && (currentMillis - activityStartTime >= (durationSet * 1000))) {
      stopActivity();
      activityStarted = false;
    }
  }
}

void updateTime()
{
  timeClient.update();
  timeClient.setTimeOffset(totalOffsetSeconds);
}

void runActivity() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Activity started");
}

void stopActivity() {
  digitalWrite(LED_PIN, LOW);
  Serial.println("Activity stopped");
}

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    // Parse JSON message
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload, length);
    const char* messageType = doc["type"];

    if (strcmp(messageType, "toggle_system") == 0) {
      bool setIsEnabled = doc["enabled"];
      isSystemEnabled = setIsEnabled;
      // Save configuration after updating the variable
      saveConfig();
    }
    if (strcmp(messageType, "set_time") == 0) {
      const char* setTime = doc["time"];
      timeSet = setTime;
      // Save configuration after updating the variable
      saveConfig();
    }
    else if (strcmp(messageType, "set_duration") == 0) {
      uint8_t setDuration = doc["time"];
      durationSet = setDuration;
      // Save configuration after updating the variable
      saveConfig();
    }
    else if (strcmp(messageType, "init_request") == 0) {
      // Send initial state to browser when requested
      sendInitialState();
    }
  }
}

// Function to send the current state of variables to the browser
void sendInitialState() {
  StaticJsonDocument<200> doc;
  doc["type"] = "initial_state";

  doc["time"] = timeSet;
  doc["duration"] = durationSet;
  doc["enabled"] = isSystemEnabled;

  // Serialize JSON document to a String object
  String jsonString;
  serializeJson(doc, jsonString);

  // Broadcast the JSON message
  webSocket.broadcastTXT(jsonString);
}
