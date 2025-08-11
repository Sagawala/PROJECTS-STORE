#include <HX711.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <EEPROM.h>

// Load cell pin configuration
const int LOADCELL_DOUT_PIN = 25; 
const int LOADCELL_SCK_PIN = 26;   

HX711 scale;

// Calibration factor, to be stored in EEPROM
float calibration_factor; 

bool is_calibrating = false; 

// WiFi configuration
const char* ssid = "cloud"; 
const char* password = "12345678"; 

// Firebase configuration
#define API_KEY "AIzaSyCLgdXzSHCmazyd6_W4QUNOuOlpkKNOxyo"  
#define DATABASE_URL "https://suza-gas-monitor-default-rtdb.europe-west1.firebasedatabase.app/" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false; 

#define AO_PIN 35
#define BUZZER_PIN 13
#define GAS_THRESHOLD 700

// EEPROM address for calibration factor
const int calFactorEEPROMAddress = 0;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); // Initialize EEPROM

  // Load calibration factor from EEPROM
  EEPROM.get(calFactorEEPROMAddress, calibration_factor);
  if (calibration_factor == 0 || isnan(calibration_factor)) { // Check for uninitialized EEPROM
    calibration_factor = 1000.00; // Default calibration factor
    EEPROM.put(calFactorEEPROMAddress, calibration_factor);
  }

  Serial.print("Calibration factor loaded: ");
  Serial.println(calibration_factor);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up anonymously
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Anonymous sign-up successful");
    signupOK = true;
  } else {
    Serial.printf("Sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); // Apply calibration factor
  scale.tare(); // Zero the scale
  Serial.println("Scale initialized and tared");
}

void loop() {
  maintainWiFiConnection();

  if (is_calibrating) {
    handleCalibration();
  } else {
    handleOperation();
  }

  handleGasSensor();
  delay(1000); // Delay between loops
}

void maintainWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Reconnected to WiFi");
  }
}

void handleCalibration() {
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1);
  Serial.println(" grams");
  if (Serial.available()) {
    char input = Serial.read();
    switch(input) {
      case 'a':
        calibration_factor += 10;
        break;
      case 'z':
        calibration_factor -= 10;
        break;
      case 's':
        EEPROM.put(calFactorEEPROMAddress, calibration_factor);
        Serial.print("Calibration factor saved: ");
        Serial.println(calibration_factor);
        break;
      case 'c':
        is_calibrating = false;
        Serial.println("Exiting calibration mode.");
        break;
    }
    scale.set_scale(calibration_factor);
  }
}

void handleOperation() {
  float weight = scale.get_units();
  if (weight < 0) weight = 0; // Adjust negative weight readings

  Serial.print("Weight: ");
  Serial.print(weight, 2);
  Serial.println(" grams");

  String path = "/Sensors/Loadcell/weight";
  if (Firebase.setFloat(fbdo, path.c_str(), weight)) {
    Serial.println("Weight data sent to Firebase");
    handleAlerts(weight);
  } else {
    Serial.print("Failed to send data: ");
    Serial.println(fbdo.errorReason());
  }

  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'c') {
      is_calibrating = true;
      Serial.println("Entering calibration mode.");
    }
  }
}

void handleAlerts(float weight) {
  Firebase.setBool(fbdo, "/alerts/lowGasLevel", weight < 1500);
  Firebase.setBool(fbdo, "/alerts/criticalGasLevel", weight < 500);
}

void handleGasSensor() {
  int gasValue = analogRead(AO_PIN);
  Serial.print("MQ2 sensor AO value: ");
  Serial.println(gasValue);

  if (Firebase.setInt(fbdo, "/Sensors/MQ2/gasLevel", gasValue)) {
    bool isGasLeak = gasValue > GAS_THRESHOLD;
    digitalWrite(BUZZER_PIN, isGasLeak ? HIGH : LOW);
    Firebase.setBool(fbdo, "/alerts/gasLeak", isGasLeak);
    Serial.println("Gas level and alert data sent to Firebase");
  } else {
    Serial.print("Failed to send gas level data: ");
    Serial.println(fbdo.errorReason());
  }
}
