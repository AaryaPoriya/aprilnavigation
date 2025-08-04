from flask import Flask, request, jsonify
import cv2
import numpy as np
from pupil_apriltags import Detector
import json
import os

app = Flask(__name__)

# --------- CONFIGURATION ---------
TOLERANCE = 0.15  # 15% of half screen width = center "dead zone"
# --------------------------------

# Initialize the detector (AprilTag)
# Using tag36h11 family which is the most common and robust AprilTag family
detector = Detector(families='tag36h11')

# File to store the latest detection result
RESULT_FILE = "last_result.json"

# Initialize with a default direction
def init_result_file():
    if not os.path.exists(RESULT_FILE):
        with open(RESULT_FILE, "w") as f:
            json.dump({"direction": "searching"}, f)

# Function to detect AprilTag and determine direction
def detect_direction(image):
    # Convert to grayscale for AprilTag detection
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # Detect AprilTags in the grayscale image
    detections = detector.detect(gray)
    
    # Calculate frame dimensions and tolerance zone
    h, w = image.shape[:2]  # Height and width of the frame
    center_x = w // 2       # X-coordinate of the center of the frame
    tol_px = int(TOLERANCE * (w / 2))  # Width of the tolerance zone in pixels
    
    # Default action when no tag is detected
    action = "searching"
    
    if detections:
        tag = detections[0]  # Use first detected tag
        cx, cy = tag.center
        dx = (cx / (w / 2)) - 1  # normalize: -1 (left), 0 (center), +1 (right)
        
        # Decide action based on tag position relative to center
        # If tag is to the left of tolerance zone, turn left
        # If tag is to the right of tolerance zone, turn right
        # If tag is within tolerance zone, move forward
        if dx < -TOLERANCE:
            action = "turn_left"
        elif dx > TOLERANCE:
            action = "turn_right"
        else:
            action = "forward"
    
    # Save the result to a file
    with open(RESULT_FILE, "w") as f:
        json.dump({"direction": action}, f)
    
    return action

@app.route('/process-image', methods=['POST'])
def process_image():
    # Check if image file is in the request
    if 'image' not in request.files:
        return jsonify({"error": "No image file in request"}), 400
    
    file = request.files['image']
    
    # Read the image
    img_bytes = file.read()
    nparr = np.frombuffer(img_bytes, np.uint8)
    image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    
    if image is None:
        return jsonify({"error": "Invalid image file"}), 400
    
    # Process the image and get the direction
    direction = detect_direction(image)
    
    return jsonify({"direction": direction})

@app.route('/get-direction', methods=['GET'])
def get_direction():
    try:
        with open(RESULT_FILE) as f:
            return jsonify(json.load(f))
    except FileNotFoundError:
        # Initialize with default if file doesn't exist
        init_result_file()
        with open(RESULT_FILE) as f:
            return jsonify(json.load(f))

# Initialize the result file when the app starts
init_result_file()

if __name__ == '__main__':
    app.run(host='0.0.0.0')