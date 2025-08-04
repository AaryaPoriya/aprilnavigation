/*
 * ESP32-CAM Example for AprilTag Detection API
 * 
 * This sketch captures an image with the ESP32-CAM and sends it to the
 * Flask API for AprilTag detection.
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// API endpoint - replace with your actual Render deployment URL
const char* serverUrl = "https://your-app-name.onrender.com/process-image";

// Camera pins for AI Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // Initialize camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Initial settings
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA; // 640x480
    config.jpeg_quality = 10;  // 0-63 lower means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  // Capture an image
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    return;
  }
  
  // Check WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    Serial.println("Sending image to server...");
    
    // Configure the request
    http.begin(serverUrl);
    
    // Set content type for multipart/form-data
    http.addHeader("Content-Type", "multipart/form-data; boundary=AprilTagBoundary");
    
    // Create the multipart form data
    String head = "--AprilTagBoundary\r\nContent-Disposition: form-data; name=\"image\"; filename=\"esp32cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--AprilTagBoundary--\r\n";
    
    // Calculate the total size
    uint32_t imageLen = fb->len;
    uint32_t totalLen = head.length() + imageLen + tail.length();
    
    // Send the request
    uint32_t httpCode = http.POST((uint8_t *)head.c_str(), head.length());
    if (httpCode == HTTP_CODE_OK) {
      // Continue sending the image data
      httpCode = http.POST((uint8_t *)fb->buf, fb->len);
      if (httpCode == HTTP_CODE_OK) {
        // Finish with the tail
        httpCode = http.POST((uint8_t *)tail.c_str(), tail.length());
        
        // Get the response
        if (httpCode == HTTP_CODE_OK) {
          String response = http.getString();
          Serial.println("Server response: " + response);
        } else {
          Serial.printf("HTTP POST tail failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
      } else {
        Serial.printf("HTTP POST image failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    } else {
      Serial.printf("HTTP POST head failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
  }
  
  // Release the frame buffer
  esp_camera_fb_return(fb);
  
  // Wait before next capture
  delay(5000); // Send image every 5 seconds
}