#include <HX711.h>
#include <WiFi.h>
#include <FirebaseESP32.h>


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 25;  // DT pin of HX711
const int LOADCELL_SCK_PIN = 26;   // SCK pin of HX711

HX711 scale;

float calibration_factor = 8032.65f; // New calibration factor for 20kg load cell
bool is_calibrating = true; // Start in calibration mode

FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  Serial.println("HX711 calibration and operation sketch");
  Serial.println("Remove all weight from the scale to tare it.");

  scale.set_scale();
  scale.tare(); // Reset the scale to 0
  delay(2000); // Wait for tare to stabilize

  Serial.println("Tare complete. Place a known weight on the scale.");
  Serial.println("Press 'a' to increase calibration factor, 'z' to decrease calibration factor.");
  Serial.println("Press 'c' to switch between calibration and operation mode.");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  // Wait for connection with a timeout of 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    
    // Initialize Firebase
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
  } else {
    Serial.println("Failed to connect to WiFi");
  }
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
    Serial.println("WiFi not connected. Attempting to reconnect...");
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();

    // Wait for connection with a timeout of 10 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi reconnected");
      
      // Reinitialize Firebase
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      Firebase.reconnectWiFi(true);
    } else {
      Serial.println("Failed to reconnect to WiFi");
      delay(10000); // Wait for 10 seconds before retrying
      return; // Skip the rest of the loop and try reconnecting again
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
      Serial.print(weight / 1000.0, 2); // Read and print weight in kilograms with two decimal places
      Serial.println(" kg");
      
      // Send data to Firebase
      String path = "/loadcell/weight";
      if (Firebase.setFloat(firebaseData, path, weight / 1000.0)) {
        Serial.println("Data sent to Firebase successfully");
      } else {
        Serial.println("Failed to send data to Firebase");
        Serial.println(firebaseData.errorReason());
      }
    } else {
      Serial.println("Weight: 0.00 kg"); // Display 0.00 kg for negative readings
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
