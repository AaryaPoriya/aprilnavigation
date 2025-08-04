import cv2
import numpy as np
from pupil_apriltags import Detector  # Using pupil-apriltags as dt-apriltags was not available

# --------- CONFIGURATION ---------
TOLERANCE = 0.15  # 15% of half screen width = center "dead zone"
CAMERA_INDEX = 0  # 0 is usually the default webcam
# --------------------------------

# Initialize the detector (AprilTag)
# Using tag36h11 family which is the most common and robust AprilTag family
detector = Detector(families='tag36h11')

# Open webcam feed
cap = cv2.VideoCapture(CAMERA_INDEX)
if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

print("Press 'q' to quit.")

# Main processing loop
while True:
    # Capture frame-by-frame
    ret, frame = cap.read()
    if not ret:
        print("Failed to grab frame.")
        break

    # Convert to grayscale for AprilTag detection
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # Detect AprilTags in the grayscale image
    detections = detector.detect(gray)

    # Calculate frame dimensions and tolerance zone
    h, w = frame.shape[:2]  # Height and width of the frame
    center_x = w // 2       # X-coordinate of the center of the frame
    tol_px = int(TOLERANCE * (w / 2))  # Width of the tolerance zone in pixels

    # Default action when no tag is detected
    action = "searching"

    if detections:
        tag = detections[0]  # Use first detected tag
        cx, cy = tag.center
        dx = (cx / (w / 2)) - 1  # normalize: -1 (left), 0 (center), +1 (right)

        # Draw tag corners (green circles)
        for corner in tag.corners.astype(int):
            cv2.circle(frame, tuple(corner), 4, (0, 255, 0), -1)

        # Draw center of tag (yellow circle)
        cv2.circle(frame, (int(cx), int(cy)), 6, (0, 255, 255), -1)

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

    # Draw visual guides
    # Blue centerline (vertical line in the middle of the frame)
    cv2.line(frame, (center_x, 0), (center_x, h), (255, 0, 0), 2)
    # Red tolerance zone boundaries (left and right of centerline)
    cv2.line(frame, (center_x - tol_px, 0), (center_x - tol_px, h), (0, 0, 255), 1)  # Left boundary
    cv2.line(frame, (center_x + tol_px, 0), (center_x + tol_px, h), (0, 0, 255), 1)  # Right boundary

    # Display the action text in the top-left corner
    cv2.putText(frame, f"Action: {action}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 255, 255), 2)

    # Show the frame
    cv2.imshow("AprilTag Webcam Tracker", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Cleanup
cap.release()
cv2.destroyAllWindows()
