#include <HX711.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h" 
#include "addons/RTDBHelper.h"  


const int LOADCELL_DOUT_PIN = 25; 
const int LOADCELL_SCK_PIN = 26;   

HX711 scale;

float calibration_factor = 8032.65f; 
bool is_calibrating = false; 

const char* ssid = "cloud"; 
const char* password = "12345678"; 


#define API_KEY "AIzaSyDIF5h2S0BOBw_COB2qy6AkL_7Jm9w1tGQ"  


#define DATABASE_URL "https://gascylinderproject-7f7a9-default-rtdb.asia-southeast1.firebasedatabase.app/" 


FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false; 

#define AO_PIN 35
#define BUZZER_PIN 13
#define GAS_THRESHOLD 3000 

void setup() {
 
  Serial.begin(115200);
  Serial.println("Warming up the MQ2 sensor");


  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 


  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  unsigned long startAttemptTime = millis();

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

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("HX711 calibration and operation sketch");
  Serial.println("Remove all weight from the scale to tare it.");
  scale.set_scale();
  scale.tare(); 
  delay(2000); 

  Serial.println("Tare complete. Starting in operation mode.");
  Serial.println("Press 'c' to switch to calibration mode.");
  delay(20000); 

void changeCalibrationFactor() {
  Serial.println("***");
  Serial.print("Current calibration factor: ");
  Serial.println(calibration_factor);
  Serial.println("Enter the new calibration factor and press Enter (e.g., 8032.65):");

  while (!Serial.available()) {
    
  }

 
  calibration_factor = Serial.parseFloat();

  
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
  
    Serial.print("Reading: ");
    Serial.print(scale.get_units(), 0); 
    Serial.print(" grams");
    Serial.print(" Calibration Factor: ");
    Serial.print(calibration_factor);
    Serial.println();

    if (Serial.available()) {
      char temp = Serial.read();
      if (temp == 'a') {
        calibration_factor += 10; 
        scale.set_scale(calibration_factor);
      } else if (temp == 'z') {
        calibration_factor -= 10; 
        scale.set_scale(calibration_factor);
      } else if (temp == 'c') {
        is_calibrating = false; 
        Serial.println("Switching to operation mode.");
      } else if (temp == 'f') {
        changeCalibrationFactor(); 
      }
    }
  } else {
    
    float weight = scale.get_units();
    if (weight >= 0) {
      Serial.print("Weight: ");
      Serial.print(weight, 2); 
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
      Serial.println("Weight: 0.00 grams"); 
    }

    if (Serial.available()) {
      char temp = Serial.read();
      if (temp == 'c') {
        is_calibrating = true; 
        Serial.println("Switching to calibration mode.");
        Serial.println("Press 'a' to increase calibration factor, 'z' to decrease calibration factor.");
        Serial.println("Press 'c' to switch back to operation mode.");
      }
    }

    delay(1000); 
  }

  // MQ2 Operation
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    int gasValue = analogRead(AO_PIN);

    Serial.print("MQ2 sensor AO value: ");
    Serial.println(gasValue);

    // Check gas level and sound buzzer if threshold is exceeded
    if (gasValue > GAS_THRESHOLD) {
      digitalWrite(BUZZER_PIN, HIGH); 
      Serial.println("Gas level exceeded threshold! Buzzer activated.");
    } else {
      digitalWrite(BUZZER_PIN, LOW); 
    }

    // Send data to Firebase using the fbdo object
    if (Firebase.setInt(fbdo, "/MQ2/gasLevel", gasValue)) {
      Serial.println("Data sent to Firebase");
    } else {
      Serial.print("Firebase setInt failed: ");
      Serial.println(fbdo.errorReason());
    }
  }
}
