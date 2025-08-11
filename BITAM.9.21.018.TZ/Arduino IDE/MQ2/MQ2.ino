#include <WiFi.h>
#include <FirebaseESP32.h>

// Wi-Fi credentials
const char* ssid = "cloud";  // Replace with your WiFi SSID
const char* password = "12345678";  // Replace with your WiFi password

// Firebase credentials
#define FIREBASE_HOST "https://gass-3233f-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyCS2M-4GMLnAbQyO22_o_jgvFPTZU0X53c"  // Replace with your Firebase Database Secret

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

#define AO_PIN 35 // ESP32's pin GPIO35 connected to AO pin of the MQ2 sensor

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Warming up the MQ2 sensor");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  unsigned long startAttemptTime = millis();

  // Check if the connection is established within 30 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(1000);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
  } else {
    Serial.println("Failed to connect to WiFi");
    // Add code to handle the failure to connect to WiFi (e.g., retry, enter deep sleep, etc.)
    return;
  }

  // Firebase configuration
  config.host = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;

  // Connect to Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  delay(20000);  // Wait for the MQ2 to warm up
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
      delay(1000);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected to WiFi");
    } else {
      Serial.println("Failed to reconnect to WiFi");
      return;
    }
  }

  int gasValue = analogRead(AO_PIN);

  Serial.print("MQ2 sensor AO value: ");
  Serial.println(gasValue);

  // Send data to Firebase
  if (Firebase.setInt(firebaseData, "/gasLevel", gasValue)) {
    Serial.println("Data sent to Firebase");
  } else {
    Serial.print("Firebase setInt failed: ");
    Serial.println(firebaseData.errorReason());
  }

  delay(1000); // Adjust the delay as needed
}
