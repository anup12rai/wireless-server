
/*
  ==========================================
       JARVIC AI - ESP8266 GROQ CHATBOT
  ==========================================

  Features:
  1. Wi-Fi configuration dashboard
  2. Board name configuration
  3. Wi-Fi SSID and password configuration
  4. Groq AI chatbot through Serial Monitor
  5. Conversation memory (last 3 exchanges)
  6. /clear and /help commands

  Board: NodeMCU ESP8266 / ESP-12E
  Serial baud: 115200

  Required libraries:
  - ESP8266 board package
  - ArduinoJson 6.x
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// ==========================================
// 1. GROQ CONFIGURATION
// ==========================================

const char* GROQ_API_KEY = "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";

const char* GROQ_URL =
  "https://api.groq.com/openai/v1/chat/completions";

const char* GROQ_MODEL = "openai/gpt-oss-20b";

// ==========================================
// 2. ESP8266 ACCESS POINT CONFIGURATION
// ==========================================

const char* AP_SSID = "ESP8266_Config";
const char* AP_PASSWORD = "configure123";

ESP8266WebServer server(80);

// ==========================================
// 3. DEVICE SETTINGS
// ==========================================

String boardName = "esp8266-ani";
String wifiName = "";
String wifiPassword = "";

const char* SETTINGS_FILE = "/settings.json";

// ==========================================
// 4. CHAT MEMORY
// ==========================================

// Stores the last 6 messages:
// 3 user messages + 3 Jarvic responses.

DynamicJsonDocument conversation(2500);

const int MAX_HISTORY_MESSAGES = 6;

// ==========================================
// 5. SERIAL CHAT VARIABLES
// ==========================================

String serialInput = "";

bool wifiConnected = false;

const int MAX_INPUT_LENGTH = 300;

// ==========================================
// 6. SAVE SETTINGS
// ==========================================

bool saveSettings() {

  DynamicJsonDocument doc(512);

  doc["boardName"] = boardName;
  doc["ssid"] = wifiName;
  doc["password"] = wifiPassword;

  File file = LittleFS.open(SETTINGS_FILE, "w");

  if (!file) {
    Serial.println("Failed to save settings.");
    return false;
  }

  serializeJson(doc, file);

  file.close();

  Serial.println("Settings saved.");

  return true;
}

// ==========================================
// 7. LOAD SETTINGS
// ==========================================

void loadSettings() {

  if (!LittleFS.exists(SETTINGS_FILE)) {

    Serial.println("No saved settings found.");

    return;
  }

  File file = LittleFS.open(SETTINGS_FILE, "r");

  if (!file) {

    Serial.println("Failed to open settings.");

    return;
  }

  DynamicJsonDocument doc(512);

  DeserializationError error = deserializeJson(doc, file);

  file.close();

  if (error) {

    Serial.println("Settings file error.");

    return;
  }

  boardName = doc["boardName"] | "esp8266-ani";
  wifiName = doc["ssid"] | "";
  wifiPassword = doc["password"] | "";

  Serial.println("Settings loaded.");
}

// ==========================================
// 8. DASHBOARD STATUS API
// ==========================================

void handleStatus() {

  DynamicJsonDocument doc(512);

  doc["boardName"] = boardName;
  doc["connected"] = WiFi.status() == WL_CONNECTED;
  doc["ssid"] = wifiName;
  doc["localIP"] = WiFi.localIP().toString();
  doc["apIP"] = WiFi.softAPIP().toString();

  String response;

  serializeJson(doc, response);

  server.send(200, "application/json", response);
}

// ==========================================
// 9. DASHBOARD CONFIGURATION API
// ==========================================

void handleConfig() {

  if (!server.hasArg("plain")) {

    server.send(
      400,
      "application/json",
      "{\"success\":false,\"message\":\"Missing JSON data\"}"
    );

    return;
  }

  DynamicJsonDocument doc(768);

  DeserializationError error =
    deserializeJson(doc, server.arg("plain"));

  if (error) {

    server.send(
      400,
      "application/json",
      "{\"success\":false,\"message\":\"Invalid JSON\"}"
    );

    return;
  }

  String newBoardName = doc["boardName"] | "";
  String newSSID = doc["ssid"] | "";
  String newPassword = doc["password"] | "";

  newBoardName.trim();
  newSSID.trim();

  if (newBoardName.length() == 0 ||
      newSSID.length() == 0) {

    server.send(
      400,
      "application/json",
      "{\"success\":false,\"message\":\"Board name and WiFi name are required\"}"
    );

    return;
  }

  boardName = newBoardName;
  wifiName = newSSID;
  wifiPassword = newPassword;

  if (!saveSettings()) {

    server.send(
      500,
      "application/json",
      "{\"success\":false,\"message\":\"Could not save settings\"}"
    );

    return;
  }

  server.send(
    200,
    "application/json",
    "{\"success\":true,\"message\":\"Configuration saved. ESP8266 restarting.\"}"
  );

  delay(1000);

  ESP.restart();
}

// ==========================================
// 10. START WEB SERVER
// ==========================================

void startWebServer() {

  server.on("/status", HTTP_GET, handleStatus);

  server.on("/config", HTTP_POST, handleConfig);

  server.on("/", HTTP_GET, []() {

    server.send(
      200,
      "text/plain",
      "Jarvic ESP8266 is running."
    );

  });

  server.onNotFound([]() {

    server.send(
      404,
      "application/json",
      "{\"message\":\"Endpoint not found\"}"
    );

  });

  server.begin();

  Serial.println("Configuration server started.");
}

// ==========================================
// 11. START ESP8266 HOTSPOT
// ==========================================

void startAccessPoint() {

  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.println();
  Serial.println("================================");
  Serial.println("JARVIC CONFIGURATION HOTSPOT");
  Serial.println("================================");

  Serial.print("Hotspot: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("Dashboard API IP: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("================================");
}

// ==========================================
// 12. CONNECT TO ROUTER WIFI
// ==========================================

void connectToWiFi() {

  if (wifiName.length() == 0) {

    Serial.println();
    Serial.println("WiFi not configured.");
    Serial.println("Connect to ESP8266_Config");
    Serial.println("and configure WiFi using dashboard.");

    wifiConnected = false;

    return;
  }

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(
    wifiName.c_str(),
    wifiPassword.c_str()
  );

  unsigned long startTime = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startTime < 20000
  ) {

    server.handleClient();

    Serial.print(".");

    delay(300);

  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    wifiConnected = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("       JARVIC IS ONLINE");
    Serial.println("================================");

    Serial.print("Board: ");
    Serial.println(boardName);

    Serial.print("WiFi: ");
    Serial.println("Connected");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.println("AI: Ready");
    Serial.println("Memory: Enabled");

    Serial.println();
    Serial.println("Enter your message:");
    Serial.print("> ");

  } else {

    wifiConnected = false;

    Serial.println("WiFi connection failed.");
    Serial.println("Check your WiFi name and password.");

    Serial.println();
    Serial.println("Configuration hotspot remains available.");
  }
}

// ==========================================
// 13. CLEAR CHAT MEMORY
// ==========================================

void clearMemory() {

  conversation.clear();

  conversation.to<JsonArray>();

  Serial.println();
  Serial.println("Jarvic memory cleared.");
  Serial.println("Starting a new conversation.");
  Serial.println();
}

// ==========================================
// 14. DISPLAY HELP
// ==========================================

void showHelp() {

  Serial.println();
  Serial.println("========== JARVIC COMMANDS ==========");

  Serial.println("/clear  - Clear conversation memory");
  Serial.println("/help   - Show available commands");

  Serial.println("=====================================");

  Serial.println();
}

// ==========================================
// 15. SEND MESSAGE TO GROQ
// ==========================================

void askJarvic(String question) {

  if (WiFi.status() != WL_CONNECTED) {

    wifiConnected = false;

    Serial.println("WiFi disconnected. AI unavailable.");

    return;
  }

  if (String(GROQ_API_KEY) == "YOUR_GROQ_API_KEY") {

    Serial.println("Please add your Groq API key in the code.");

    return;
  }

  WiFiClientSecure client;

  // Testing only. Use certificate verification for production.
  client.setInsecure();

  HTTPClient https;

  https.setTimeout(30000);

  Serial.println();
  Serial.println("Jarvic is thinking...");

  if (!https.begin(client, GROQ_URL)) {

    Serial.println("Could not start HTTPS connection.");

    return;
  }

  https.addHeader(
    "Content-Type",
    "application/json"
  );

  https.addHeader(
    "Authorization",
    String("Bearer ") + GROQ_API_KEY
  );

  // Create request JSON
  DynamicJsonDocument requestDoc(4096);

  requestDoc["model"] = GROQ_MODEL;

  requestDoc["temperature"] = 0.7;

  requestDoc["max_completion_tokens"] = 250;

  JsonArray messages =
    requestDoc.createNestedArray("messages");

  // System personality
  JsonObject systemMessage =
    messages.createNestedObject();

  systemMessage["role"] = "system";

  systemMessage["content"] =
    "You are Jarvic, a friendly and intelligent AI assistant. "
    "Answer clearly and naturally. "
    "Use previous conversation messages to understand follow-up questions. "
    "Be helpful and concise.";

  // Add previous conversation
  JsonArray history = conversation.as<JsonArray>();

  for (JsonVariant item : history) {

    JsonObject message =
      messages.createNestedObject();

    message["role"] = item["role"];
    message["content"] = item["content"];

  }

  // Add current question
  JsonObject currentMessage =
    messages.createNestedObject();

  currentMessage["role"] = "user";

  currentMessage["content"] = question;

  // Convert JSON to String
  String requestBody;

  serializeJson(requestDoc, requestBody);

  // Send API request
  int httpCode = https.POST(requestBody);

  if (httpCode == HTTP_CODE_OK) {

    String response = https.getString();

    // Extract only the answer
    StaticJsonDocument<128> filter;

    filter["choices"][0]["message"]["content"] = true;

    DynamicJsonDocument responseDoc(2048);

    DeserializationError error = deserializeJson(
      responseDoc,
      response,
      DeserializationOption::Filter(filter)
    );

    if (!error) {

      String answer =
        responseDoc["choices"][0]["message"]["content"].as<String>();

      Serial.println();
      Serial.println("================================");
      Serial.println("             JARVIC");
      Serial.println("================================");

      Serial.println(answer);

      Serial.println("================================");

      // Save user's question
      JsonObject savedUser =
        history.createNestedObject();

      savedUser["role"] = "user";

      savedUser["content"] = question;

      // Save Jarvic's response
      JsonObject savedAssistant =
        history.createNestedObject();

      savedAssistant["role"] = "assistant";

      savedAssistant["content"] = answer;

      // Keep only the last 6 messages
      while (history.size() > MAX_HISTORY_MESSAGES) {

        history.remove(0);

      }

      Serial.println("Memory: Updated");

    } else {

      Serial.println("Could not read Groq response.");

    }

  } else {

    Serial.print("Groq API HTTP error: ");

    Serial.println(httpCode);

    String errorResponse = https.getString();

    Serial.println(errorResponse.substring(0, 400));

  }

  https.end();

  Serial.println();
  Serial.println("Enter your message:");
  Serial.print("> ");
}

// ==========================================
// 16. HANDLE SERIAL CHAT INPUT
// ==========================================

void handleSerialChat() {

  while (Serial.available()) {

    char receivedChar = Serial.read();

    if (receivedChar == '\n' || receivedChar == '\r') {

      if (serialInput.length() > 0) {

        String question = serialInput;

        serialInput = "";

        question.trim();

        if (question.length() == 0) {
          return;
        }

        // Clear memory
        if (question == "/clear") {

          clearMemory();

          Serial.print("> ");

          return;
        }

        // Help command
        if (question == "/help") {

          showHelp();

          Serial.print("> ");

          return;
        }

        // Send normal message to Groq
        askJarvic(question);

      }

    } else {

      if (serialInput.length() < MAX_INPUT_LENGTH) {

        serialInput += receivedChar;

      }

    }

  }

}

// ==========================================
// 17. SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("       STARTING JARVIC AI");
  Serial.println("================================");

  // Initialize filesystem
  if (!LittleFS.begin()) {

    Serial.println("LittleFS initialization failed.");

  } else {

    Serial.println("LittleFS initialized.");

    loadSettings();

  }

  // Initialize conversation memory
  conversation.to<JsonArray>();

  Serial.println("Jarvic memory initialized.");

  // Start hotspot
  startAccessPoint();

  // Start dashboard server
  startWebServer();

  // Connect to configured WiFi
  connectToWiFi();

}

// ==========================================
// 18. MAIN LOOP
// ==========================================

void loop() {

  // Keep dashboard available
  server.handleClient();

  // Keep WiFi status updated
  if (WiFi.status() != WL_CONNECTED) {

    wifiConnected = false;

  } else {

    wifiConnected = true;

  }

  // Accept chat only when WiFi is connected
  if (wifiConnected) {

    handleSerialChat();

  }

  delay(2);

}
