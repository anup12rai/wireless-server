
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// ===================================
// JARVIC CONFIGURATION
// ===================================

#define EEPROM_SIZE 256

const char* AP_SSID = "JARVIC-SETUP";
const char* AP_PASSWORD = "Jarvic123";

ESP8266WebServer server(80);

// ===================================
// SAVED WIFI INFORMATION
// ===================================

struct WiFiConfig {

    char magic[4];

    char boardName[33];

    char ssid[33];

    char password[64];

};

WiFiConfig config;

// ===================================
// WIFI EVENT HANDLERS
// ===================================

WiFiEventHandler gotIPHandler;
WiFiEventHandler disconnectedHandler;


// ===================================
// SAVE CONFIGURATION
// ===================================

void saveConfiguration(
    String boardName,
    String ssid,
    String password
) {

    memset(&config, 0, sizeof(config));

    strcpy(config.magic, "JRV");

    boardName.toCharArray(config.boardName, 33);

    ssid.toCharArray(config.ssid, 33);

    password.toCharArray(config.password, 64);

    EEPROM.put(0, config);

    EEPROM.commit();

    Serial.println("Configuration saved.");

}


// ===================================
// LOAD CONFIGURATION
// ===================================

bool loadConfiguration() {

    EEPROM.get(0, config);

    if (strncmp(config.magic, "JRV", 3) != 0) {

        return false;

    }

    config.boardName[32] = '\0';

    config.ssid[32] = '\0';

    config.password[63] = '\0';

    return true;

}


// ===================================
// CONNECT TO ROUTER
// ===================================

void connectToWiFi() {

    if (strlen(config.ssid) == 0) {

        Serial.println("No Wi-Fi credentials found.");

        return;

    }

    Serial.println();

    Serial.println("Connecting to Wi-Fi...");

    Serial.print("Board Name: ");

    Serial.println(config.boardName);

    Serial.print("Wi-Fi Name: ");

    Serial.println(config.ssid);

    WiFi.begin(
        config.ssid,
        config.password
    );

}


// ===================================
// RECEIVE WIFI CREDENTIALS
// ===================================

void handleConnect() {

    if (!server.hasArg("boardName") ||
        !server.hasArg("ssid") ||
        !server.hasArg("password")) {

        server.send(
            400,
            "application/json",
            "{\"success\":false,\"message\":\"Missing fields\"}"
        );

        return;

    }

    String boardName = server.arg("boardName");

    String ssid = server.arg("ssid");

    String password = server.arg("password");

    if (boardName.length() == 0 ||
        boardName.length() > 32 ||
        ssid.length() == 0 ||
        ssid.length() > 32 ||
        password.length() < 8 ||
        password.length() > 63) {

        server.send(
            400,
            "application/json",
            "{\"success\":false,\"message\":\"Invalid credentials\"}"
        );

        return;

    }

    Serial.println();

    Serial.println("New Wi-Fi configuration received.");

    Serial.print("Board: ");

    Serial.println(boardName);

    Serial.print("New Wi-Fi: ");

    Serial.println(ssid);

    saveConfiguration(
        boardName,
        ssid,
        password
    );

    server.send(
        200,
        "application/json",
        "{\"success\":true,\"message\":\"Credentials received\"}"
    );

    delay(200);

    WiFi.disconnect();

    connectToWiFi();

}


// ===================================
// WIFI CONNECTION EVENTS
// ===================================

void setupWiFiEvents() {

    gotIPHandler = WiFi.onStationModeGotIP(
        [](const WiFiEventStationModeGotIP& event) {

            Serial.println();

            Serial.println("==============================");

            Serial.println("JARVIC WIFI CONNECTED");

            Serial.print("Switched to Wi-Fi: ");

            Serial.println(WiFi.SSID());

            Serial.print("IP Address: ");

            Serial.println(WiFi.localIP());

            Serial.println("==============================");

        }
    );


    disconnectedHandler = WiFi.onStationModeDisconnected(
        [](const WiFiEventStationModeDisconnected& event) {

            Serial.println();

            Serial.println("Wi-Fi disconnected!");

            Serial.print("Disconnected from: ");

            Serial.println(event.ssid);

            Serial.println("Trying to reconnect...");

        }
    );

}


// ===================================
// SETUP WIFI ACCESS POINT
// ===================================

void startAccessPoint() {

    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP(
        AP_SSID,
        AP_PASSWORD
    );

    Serial.println();

    Serial.println("JARVIC SETUP NETWORK STARTED");

    Serial.print("Wi-Fi Name: ");

    Serial.println(AP_SSID);

    Serial.print("Setup IP: ");

    Serial.println(WiFi.softAPIP());

}


// ===================================
// MAIN SETUP
// ===================================

void setup() {

    Serial.begin(115200);

    delay(1000);

    Serial.println();

    Serial.println("================================");

    Serial.println("       JARVIC ESP8266");

    Serial.println("================================");

    EEPROM.begin(EEPROM_SIZE);

    WiFi.persistent(false);

    WiFi.setAutoReconnect(true);

    setupWiFiEvents();

    startAccessPoint();

    server.on(
        "/connect",
        HTTP_POST,
        handleConnect
    );

    server.on(
        "/",
        HTTP_GET,
        []() {

            server.send(
                200,
                "text/plain",
                "JARVIC ESP8266 is running"
            );

        }
    );

    server.begin();

    Serial.println("HTTP Server Started.");

    if (loadConfiguration()) {

        Serial.println("Saved configuration found.");

        connectToWiFi();

    } else {

        Serial.println("Waiting for Wi-Fi configuration.");

    }

}


// ===================================
// MAIN LOOP
// ===================================

void loop() {

    server.handleClient();

}
