#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h" // Provide the token generation process info
#include "addons/RTDBHelper.h"  // Provide the RTDB payload printing info and other helper functions

// Wi-Fi credentials
const char* ssid = "cloud";  // Replace with your WiFi SSID
const char* password = "12345678";  // Replace with your WiFi password

// Firebase project API Key
#define API_KEY "AIzaSyDIF5h2S0BOBw_COB2qy6AkL_7Jm9w1tGQ"  // Replace with your Firebase project API Key

// Firebase Realtime Database URL
#define DATABASE_URL "https://gascylinderproject-7f7a9-default-rtdb.asia-southeast1.firebasedatabase.app/" // Replace with your Firebase Realtime Database URL

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;  // Since we are doing an anonymous sign-in

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
    return;
  }

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up anonymously
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Anonymous sign-up successful");
    signupOK = true;
  } else {
    Serial.printf("Sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long-running token generation task
  config.token_status_callback = tokenStatusCallback;

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

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    int gasValue = analogRead(AO_PIN);

    Serial.print("MQ2 sensor AO value: ");
    Serial.println(gasValue);

    // Send data to Firebase using the fbdo object
    if (Firebase.setInt(fbdo, "/gasLevel", gasValue)) {
      Serial.println("Data sent to Firebase");
    } else {
      Serial.print("Firebase setInt failed: ");
      Serial.println(fbdo.errorReason());
    }
  }
}
