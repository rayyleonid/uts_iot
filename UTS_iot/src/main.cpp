#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>  // Untuk mendapatkan timestamp

// WiFi settings
const char *ssid = "Wokwi-GUEST";             // Replace with your WiFi name
const char *password = "";   // Replace with your WiFi password

// MQTT Broker settings
const char *mqtt_broker = "broker.emqx.io";  // EMQX broker endpoint
const char *mqtt_topic = "RayyanLeonidRamadhan/IOT";     // MQTT topic
const char *mqtt_username = "rayyanleonid";  // MQTT username for authentication
const char *mqtt_password = "rayyanleonid11";  // MQTT password for authentication
const int mqtt_port = 1883;  // MQTT port (TCP)

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// DHT Sensor settings
#define DHTPIN 33
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// GPIO pin assignments
const int LED_GREEN = 5;
const int LED_YELLOW = 18;
const int LED_RED = 12;
const int RELAY_PIN = 7;
const int BUZZER_PIN = 19;

void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void initTime();
String getFormattedTime();

void setup() {
    Serial.begin(115200);

    // Initialize pins
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Initialize DHT sensor
    dht.begin();

    connectToWiFi();
    initTime();  // Inisialisasi waktu
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void initTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // Konfigurasi NTP server
    Serial.print("Waiting for time");
    while (!time(nullptr)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nTime initialized");
}

String getFormattedTime() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char buffer[26];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "esp8266-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic);
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    // Process incoming MQTT messages if needed
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }
    mqtt_client.loop();

    // Baca data dari sensor DHT
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Cek apakah pembacaan sensor berhasil
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Dapatkan timestamp saat ini
    String timestamp = getFormattedTime();

    // Buat payload JSON
    String jsonPayload = "{";
    jsonPayload += "\"timestamp\":\"" + timestamp + "\",";
    jsonPayload += "\"temperature\":" + String(temperature, 2) + ",";
    jsonPayload += "\"humidity\":" + String(humidity, 2);
    jsonPayload += "}";

    // Kirim data ke topik MQTT
    if (mqtt_client.publish(mqtt_topic, jsonPayload.c_str())) {
        Serial.println("Message sent successfully");
        Serial.println("Payload: " + jsonPayload);
    } else {
        Serial.println("Failed to send message");
    }

    // Logika untuk mengontrol LED, buzzer, dan relay
    if (temperature > 35) {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, LOW);
    } else if (temperature >= 30 && temperature <= 35) {
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, LOW);
    } else {
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
    }

    // Kontrol pompa berdasarkan kondisi suhu
    if (temperature > 30) {
        digitalWrite(RELAY_PIN, HIGH);  // Nyalakan pompa
    } else {
        digitalWrite(RELAY_PIN, LOW);   // Matikan pompa
    }

    // Tunggu beberapa detik sebelum mengirim data berikutnya
    delay(5000);
}