#include <HX711.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h" // Provide the token generation process info
#include "addons/RTDBHelper.h"  // Provide the RTDB payload printing info and other helper functions

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 25;  // DT pin of HX711
const int LOADCELL_SCK_PIN = 26;   // SCK pin of HX711

HX711 scale;

float calibration_factor = 8032.65f; // Default calibration factor
bool is_calibrating = true; // Start in calibration mode

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

bool signupOK = false;  // Since we are doing an anonymous sign-in

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

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
    return;
  }

  // Assign the callback function for the long-running token generation task
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("HX711 calibration and operation sketch");
  Serial.println("Remove all weight from the scale to tare it.");
  scale.set_scale();
  scale.tare(); // Reset the scale to 0
  delay(2000); // Wait for tare to stabilize

  Serial.println("Tare complete. Place a known weight on the scale.");
  Serial.println("Press 'a' to increase calibration factor, 'z' to decrease calibration factor.");
  Serial.println("Press 'c' to switch between calibration and operation mode.");
}

void changeCalibrationFactor() {
  Serial.println("***");
  Serial.print("Current calibration factor: ");
  Serial.println(calibration_factor);
  Serial.println("Enter the new calibration factor and press Enter (e.g., 8032.65):");

  while (!Serial.available()) {
    // Wait for user input
  }

  // Read the new calibration factor from the serial monitor
  calibration_factor = Serial.parseFloat();

  // Apply the new calibration factor to the scale
  scale.set_scale(calibration_factor);
  
  Serial.print("New calibration factor set to: ");
  Serial.println(calibration_factor);
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

  if (is_calibrating) {
    // Calibration mode
    Serial.print("Reading: ");
    Serial.print(scale.get_units(), 0); // Get the raw reading in grams
    Serial.print(" grams");
    Serial.print(" Calibration Factor: ");
    Serial.print(calibration_factor);
    Serial.println();

    if (Serial.available()) {
      char temp = Serial.read();
      if (temp == 'a') {
        calibration_factor += 10; // Adjust this value as needed
        scale.set_scale(calibration_factor);
      } else if (temp == 'z') {
        calibration_factor -= 10; // Adjust this value as needed
        scale.set_scale(calibration_factor);
      } else if (temp == 'c') {
        is_calibrating = false; // Switch to operation mode
        Serial.println("Switching to operation mode.");
      } else if (temp == 'f') {
        changeCalibrationFactor(); // Call function to change calibration factor
      }
    }
  } else {
    // Operation mode
    float weight = scale.get_units(); // Read weight in grams
    if (weight >= 0) {
      Serial.print("Weight: ");
      Serial.print(weight, 2); // Read and print weight in grams with two decimal places
      Serial.println(" grams");
      
      // Send data to Firebase using the fbdo object
      String path = "/loadcell/weight";
      if (Firebase.setFloat(fbdo, path.c_str(), weight)) {
        Serial.println("Data sent to Firebase successfully");
      } else {
        Serial.print("Failed to send data to Firebase: ");
        Serial.println(fbdo.errorReason());
      }
    } else {
      Serial.println("Weight: 0.00 grams"); // Display 0.00 grams for negative readings
    }

    if (Serial.available()) {
      char temp = Serial.read();
      if (temp == 'c') {
        is_calibrating = true; // Switch to calibration mode
        Serial.println("Switching to calibration mode.");
      }
    }

    delay(1000); // Adjust the delay as needed
  }
}
