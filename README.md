# AprilTag Detection API

This Flask application provides an API for AprilTag detection and direction determination. It's designed to work with ESP32-CAM and ESP8266 devices.

## API Endpoints

### 1. Process Image

**Endpoint:** `/process-image`  
**Method:** POST  
**Description:** Accepts an image from ESP32-CAM, runs AprilTag detection, and stores the result.  

**Request:**
- Form-data with key `image` containing the image file

**Response:**
```json
{
    "direction": "turn_left" | "turn_right" | "forward" | "searching"
}
```

### 2. Get Direction

**Endpoint:** `/get-direction`  
**Method:** GET  
**Description:** Returns the latest movement direction to ESP8266.  

**Response:**
```json
{
    "direction": "turn_left" | "turn_right" | "forward" | "searching"
}
```

## Installation

1. Install the required dependencies:
   ```
   pip install -r requirements.txt
   ```

2. Run the application:
   ```
   python app.py
   ```

## Deployment to Render

1. Push the code to a GitHub repository.

2. Go to [Render](https://render.com) and click "New Web Service".

3. Connect your GitHub repository.

4. Configure the service:
   - Name: Choose a name for your service (e.g., "apriltag-detection")
   - Environment: Python 3
   - Region: Choose the region closest to your devices
   - Branch: main (or your default branch)
   - Build Command: `pip install -r requirements.txt`
   - Start Command: `gunicorn app:app`
   - Plan: Free (or choose a paid plan for better performance)

5. Click "Create Web Service" and wait for the deployment to complete.

6. Once deployed, update the server URLs in both Arduino sketches with your Render service URL:
   - In `esp32_cam_example.ino`: Update `serverUrl` with `https://your-app-name.onrender.com/process-image`
   - In `esp8266_example.ino`: Update `serverUrl` with `https://your-app-name.onrender.com/get-direction`

## How It Works

The application uses the pupil-apriltags library to detect AprilTags in images. When an image is processed:

1. The image is converted to grayscale.
2. AprilTags are detected in the image.
3. The position of the detected tag is compared to the center of the image.
4. A direction is determined based on the tag's position:
   - If the tag is to the left of the tolerance zone: `turn_left`
   - If the tag is to the right of the tolerance zone: `turn_right`
   - If the tag is within the tolerance zone: `forward`
   - If no tag is detected: `searching`
5. The direction is stored in a file (`last_result.json`) and returned in the response.

## Arduino Integration

This repository includes two Arduino sketches for integration with ESP32-CAM and ESP8266 devices:

### ESP32-CAM (Image Capture)

The `esp32_cam_example.ino` sketch demonstrates how to:

1. Initialize the ESP32-CAM camera
2. Connect to WiFi
3. Capture an image
4. Send the image to the `/process-image` endpoint
5. Parse the response to get the direction

Before uploading to your ESP32-CAM, update the following:
- WiFi credentials (SSID and password)
- Server URL with your Render deployment URL

### ESP8266 (Motor Control)

The `esp8266_example.ino` sketch demonstrates how to:

1. Connect to WiFi
2. Fetch the latest direction from the `/get-direction` endpoint
3. Control motors based on the received direction

Before uploading to your ESP8266, update the following:
- WiFi credentials (SSID and password)
- Server URL with your Render deployment URL
- Motor control pins according to your hardware setup