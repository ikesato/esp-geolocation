#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif (defined(ESP32))
  #include <WiFiClientSecure.h>
#endif
#include <ArduinoJson.h>

const char *ssid = <YOUR SSID>;
const char *pass = <YOUR PASSWORD>;
const char *apikey = <YOUR APIKEY>;

void setup() {
    Serial.begin(115200);

    // connect to WiFi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nconnected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // scan Access Points
    int numAp = WiFi.scanNetworks();
    if (numAp == 0) {
        Serial.println("Error: no WiFi access points found");
        return;
    }
    String json = "{\n";
    json += "\"considerIp\": \"false\",\n";
    json += "\"wifiAccessPoints\": [\n";
    for (int i=0; i<numAp; i++) {
        json += "{\n";
        json += "\"macAddress\": \"" + WiFi.BSSIDstr(i) + "\",\n";
        json += "\"signalStrength\": " + String(WiFi.RSSI(i)) + "\n}";
        if (i<numAp-1) {
            json += ",";
        }
        json += "\n";
    }
    json += "]}\n";
    Serial.println("request body:");
    Serial.println(json);
    Serial.println();

    // request to Google Geolocation API
    WiFiClientSecure client;
    if (!client.connect("www.googleapis.com", 443)) {
        Serial.println("Error: failed to connect geolocation server");
        return;
    }
    String request = "";
    request += "POST /geolocation/v1/geolocate?key=" + String(apikey) + " HTTP/1.1\r\n";
    request += "Host: www.googleapis.com\r\n";
    request += "User-Agent: Arduino\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + String(json.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += json;
    client.print(request);

    // wait for response
    int time = millis();
    while (client.available() == 0) {
        if (millis() - time >= 30000) {
            Serial.println("Error: Timeout");
            client.stop();
            return;
        }
    }

    // read HTTP response header
    Serial.println("response headers:");
    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
        Serial.println(line);
    }
    Serial.println();

    // read response json
    String response = "";
    while (client.available()) {
        response += client.readString();
    }
    Serial.println("response json:");
    Serial.println(response);
    Serial.println();

    // parse json
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    Serial.print("lat: ");
    Serial.println((double)root["location"]["lat"], 8);
    Serial.print("lng: ");
    Serial.println((double)root["location"]["lng"], 8);
    Serial.print("accuracy: ");
    Serial.println((double)root["accuracy"], 2);
}

void loop() {
}
