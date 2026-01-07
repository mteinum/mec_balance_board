/*
 * Balance Board Stability Monitor
 * For M5Stack AtomS3 with MPU6886 IMU
 * 
 * Author: Morten Teinum
 * Date: January 2026
 * License: MIT
 * 
 * Real-time stability feedback for shooting sports training
 * on MEC Balance Board or similar wobble boards.
 */

#include "M5AtomS3.h"
#include <math.h>

// ---------- CONFIG ----------

// How aggressively to smooth tilt (0.0–1.0).
// Higher = slower to react, but more stable.
const float SMOOTH_ALPHA = 0.90f;

// Tilt thresholds (in degrees of combined X/Y tilt)
const float GREEN_MAX_DEG  = 1.0f;  // < 1° = very stable
const float YELLOW_MAX_DEG = 3.0f;  // 1–3° = some sway, >3° = red

// Sampling interval in ms
const uint32_t SAMPLE_INTERVAL_MS = 20;  // ~50 Hz

// ---------- STATE ----------
float smoothedTiltDeg = 0.0f;
uint32_t lastSampleMs = 0;

// For avoiding unnecessary full redraws
int lastZone = -1;   // 0 = green, 1 = yellow, 2 = red

// Calibration state
float calibrationTiltX = 0.0f;  // Reference tilt around X axis
float calibrationTiltY = 0.0f;  // Reference tilt around Y axis
bool isCalibrated = false;

// Compute combined tilt from accelerometer in degrees
float computeTiltMagnitudeDeg(float ax, float ay, float az) {
    // Tilt around X and Y relative to gravity
    float tiltX = atan2f(ax, az) * 180.0f / PI;
    float tiltY = atan2f(ay, az) * 180.0f / PI;

    // Apply calibration offset if calibrated
    if (isCalibrated) {
        tiltX -= calibrationTiltX;
        tiltY -= calibrationTiltY;
    }

    // Combined magnitude
    return sqrtf(tiltX * tiltX + tiltY * tiltY);
}

// Calibrate the current orientation as the center/zero point
void calibrate(float ax, float ay, float az) {
    calibrationTiltX = atan2f(ax, az) * 180.0f / PI;
    calibrationTiltY = atan2f(ay, az) * 180.0f / PI;
    isCalibrated = true;
    
    // Visual feedback
    AtomS3.Display.fillScreen(BLUE);
    AtomS3.Display.setTextDatum(middle_center);
    AtomS3.Display.setTextColor(WHITE, BLUE);
    AtomS3.Display.setTextSize(2);
    AtomS3.Display.drawString(
        "CALIBRATED",
        AtomS3.Display.width() / 2,
        AtomS3.Display.height() / 2
    );
    delay(500);
    lastZone = -1;  // Force full redraw
}

void drawScreen(int zone, float tiltDeg) {
    uint16_t bgColor;
    const char* statusText;
    uint16_t textColor;

    switch (zone) {
        case 0: // green
            bgColor     = GREEN;
            textColor   = BLACK;
            statusText  = "STABLE";
            break;
        case 1: // yellow
            bgColor     = YELLOW;
            textColor   = BLACK;
            statusText  = "MOVING";
            break;
        default: // 2: red
            bgColor     = RED;
            textColor   = WHITE;
            statusText  = "WOBBLY";
            break;
    }

    // Full background
    AtomS3.Display.fillScreen(bgColor);

    AtomS3.Display.setTextDatum(middle_center);
    AtomS3.Display.setTextColor(textColor, bgColor);

    // Big status text
    AtomS3.Display.setTextSize(2);
    AtomS3.Display.drawString(
        statusText,
        AtomS3.Display.width() / 2,
        AtomS3.Display.height() / 2 - 10
    );

    // Smaller numeric readout
    AtomS3.Display.setTextSize(1);
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f deg", tiltDeg);
    AtomS3.Display.drawString(
        buf,
        AtomS3.Display.width() / 2,
        AtomS3.Display.height() / 2 + 15
    );
}

void setup() {
    auto cfg = M5.config();
    AtomS3.begin(cfg);

    AtomS3.Display.setTextDatum(middle_center);
    AtomS3.Display.fillScreen(BLACK);
    AtomS3.Display.setTextColor(WHITE, BLACK);
    AtomS3.Display.setTextSize(1);
    AtomS3.Display.drawString(
        "Balance Monitor",
        AtomS3.Display.width() / 2,
        AtomS3.Display.height() / 2
    );

    delay(1000);  // little splash
}

void loop() {
    AtomS3.update();
    uint32_t now = millis();

    // Check for button press to calibrate
    if (AtomS3.BtnA.wasPressed()) {
        // Update IMU to get current reading
        AtomS3.Imu.update();
        auto data = AtomS3.Imu.getImuData();
        calibrate(data.accel.x, data.accel.y, data.accel.z);
    }

    if (now - lastSampleMs < SAMPLE_INTERVAL_MS) {
        return;
    }
    lastSampleMs = now;

    // Update IMU and read data
    bool imuUpdated = AtomS3.Imu.update();
    if (!imuUpdated) {
        return;
    }

    auto data = AtomS3.Imu.getImuData();
    float ax = data.accel.x;  // in g
    float ay = data.accel.y;
    float az = data.accel.z;

    // Compute instantaneous tilt
    float instTiltDeg = computeTiltMagnitudeDeg(ax, ay, az);

    // Exponential smoothing
    smoothedTiltDeg = SMOOTH_ALPHA * smoothedTiltDeg
                    + (1.0f - SMOOTH_ALPHA) * instTiltDeg;

    // Decide zone
    int zone;
    if (smoothedTiltDeg < GREEN_MAX_DEG) {
        zone = 0;  // green
    } else if (smoothedTiltDeg < YELLOW_MAX_DEG) {
        zone = 1;  // yellow
    } else {
        zone = 2;  // red
    }

    // Only redraw full screen if zone changed
    if (zone != lastZone) {
        drawScreen(zone, smoothedTiltDeg);
        lastZone = zone;
    } else {
        // Just update the numeric text to reduce flicker
        // (overwrite the bottom text region)
        uint16_t bgColor = (zone == 0 ? GREEN : zone == 1 ? YELLOW : RED);
        uint16_t textColor = (zone == 2 ? WHITE : BLACK);

        AtomS3.Display.setTextDatum(middle_center);
        AtomS3.Display.setTextColor(textColor, bgColor);
        AtomS3.Display.setTextSize(1);

        // Clear a small band at the bottom
        int cx = AtomS3.Display.width() / 2;
        int cy = AtomS3.Display.height() / 2 + 15;
        AtomS3.Display.fillRect(0, cy - 8, AtomS3.Display.width(), 16, bgColor);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f deg", smoothedTiltDeg);
        AtomS3.Display.drawString(buf, cx, cy);
    }
}
