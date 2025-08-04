/*
 * ESP8266 Example for AprilTag Detection API
 * 
 * This sketch retrieves the latest direction from the Flask API
 * and can be used to control a robot or vehicle based on the direction.
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// API endpoint - replace with your actual Render deployment URL
const char* serverUrl = "https://your-app-name.onrender.com/get-direction";

// Motor control pins (example)
const int LEFT_MOTOR_PIN1 = D1;
const int LEFT_MOTOR_PIN2 = D2;
const int RIGHT_MOTOR_PIN1 = D3;
const int RIGHT_MOTOR_PIN2 = D4;

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // Initialize motor control pins
  pinMode(LEFT_MOTOR_PIN1, OUTPUT);
  pinMode(LEFT_MOTOR_PIN2, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
  
  // Stop motors initially
  stopMotors();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("Getting direction from server...");
    
    // Configure the request
    http.begin(client, serverUrl);
    
    // Send the request
    int httpCode = http.GET();
    
    // Check for successful response
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Server response: " + payload);
      
      // Parse JSON response
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Get the direction
        const char* direction = doc["direction"];
        
        // Control motors based on direction
        if (strcmp(direction, "turn_left") == 0) {
          turnLeft();
          Serial.println("Turning left");
        } 
        else if (strcmp(direction, "turn_right") == 0) {
          turnRight();
          Serial.println("Turning right");
        } 
        else if (strcmp(direction, "forward") == 0) {
          moveForward();
          Serial.println("Moving forward");
        } 
        else {
          // Default to stop if direction is "searching" or unknown
          stopMotors();
          Serial.println("Stopping (searching for tag)");
        }
      } else {
        Serial.println("Failed to parse JSON");
        stopMotors();
      }
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
      stopMotors();
    }
    
    http.end();
  }
  
  // Wait before next request
  delay(500); // Check direction every 500ms
}

// Motor control functions
void moveForward() {
  digitalWrite(LEFT_MOTOR_PIN1, HIGH);
  digitalWrite(LEFT_MOTOR_PIN2, LOW);
  digitalWrite(RIGHT_MOTOR_PIN1, HIGH);
  digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

void turnLeft() {
  digitalWrite(LEFT_MOTOR_PIN1, LOW);
  digitalWrite(LEFT_MOTOR_PIN2, LOW);
  digitalWrite(RIGHT_MOTOR_PIN1, HIGH);
  digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

void turnRight() {
  digitalWrite(LEFT_MOTOR_PIN1, HIGH);
  digitalWrite(LEFT_MOTOR_PIN2, LOW);
  digitalWrite(RIGHT_MOTOR_PIN1, LOW);
  digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

void stopMotors() {
  digitalWrite(LEFT_MOTOR_PIN1, LOW);
  digitalWrite(LEFT_MOTOR_PIN2, LOW);
  digitalWrite(RIGHT_MOTOR_PIN1, LOW);
  digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}