#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "your_SSID"; // Replace with your WiFi SSID
const char* password = "your_PASSWORD"; // Replace with your WiFi Password

// AWS IoT endpoint
const char* mqtt_server = "your_AWS_IOT_ENDPOINT"; // Replace with your AWS IoT Endpoint
const int mqtt_port = 8883;
const char* mqtt_client_id = "your_CLIENT_ID"; // Replace with your AWS IoT Client ID

// AWS IoT certificate paths
const char* certificate = "/path/to/certificate.crt"; // Replace with your Certificate Path
const char* private_key = "/path/to/private.key"; // Replace with your Private Key Path
const char* ca_certificate = "/path/to/AmazonRootCA1.pem"; // Replace with your CA Certificate Path

// MQTT topics
const char* topic = "weather/data";

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

#define DHTPIN 2 // D4 on ESP8266
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  wifiClient.setCertificate(certificate);
  wifiClient.setPrivateKey(private_key);
  wifiClient.setCACert(ca_certificate);

  dht.begin();
  bmp.begin();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = bmp.readPressure();

  if (isnan(h) || isnan(t) || isnan(p)) {
    Serial.println("Failed to read from sensors!");
    return;
  }

  String payload = "{\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + ", \"pressure\": " + String(p) + "}";
  Serial.print("Publishing message: ");
  Serial.println(payload);

  client.publish(topic, (char*) payload.c_str());

  delay(60000); // Publish data every minute
}
