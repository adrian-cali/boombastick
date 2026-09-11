#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <math.h>

#define BNO_ADDR 0x28
#define SWING_THRESHOLD_SQ 62500.0 // (250.0 * 250.0)
#define REST_DELAY 150
#define BUTTON1 6
#define BUTTON3 4
#define DEBOUNCE_TIME 10

Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO_ADDR);

volatile unsigned long lastHit = 0;
volatile int lastZone = -1;
volatile bool bnoOK = true;

struct DrumZone {
  float yaw, pitch; // <-- CHANGED (Removed 'roll')
};

// Target "locations" (orientations) for each drum
// <-- CHANGED (Removed 'roll' value from each definition)
DrumZone snare  = {2.0, 5.7};
DrumZone hihat  = {273.5, 3.7};
DrumZone tom    = {93.4, 6.1};
DrumZone crash  = {320.8, 54.9};
DrumZone ride   = {58.9, 44.9};

DrumZone zones[5] = {snare, hihat, tom, crash, ride};
const char zoneIDs[5] = {'0', '1', '2', '3', '4'};

// This is the "size" of the hit zone (50.0 * 50.0)
// NOTE: Since we are now in 2D (Yaw/Pitch only), you might
// need to make this number SMALLER if the zones feel too big.
float zoneThresholdSq = 2500.0; 

void setup() {
  Serial.begin(230400);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  Wire.setClock(400000); // Max I2C throughput

  if (!bno.begin()) {
    Serial.println("BNO055 not detected! Running in button-only mode.");
    bnoOK = false;
  } else {
    bno.setExtCrystalUse(true);
  }
}

void loop() {
  unsigned long now = millis();

  // Fast button 3 handling
  static unsigned long lastButton3 = 0;
  static bool button3Held = false;
  bool b3 = (digitalRead(BUTTON3) == LOW);
  if (b3 && !button3Held && now - lastButton3 > DEBOUNCE_TIME) {
    Serial.write('5');
    lastButton3 = now;
    button3Held = true;
  } else if (!b3) {
    button3Held = false;
  }

  if (!bnoOK) return;

  // Only get the Gyroscope vector (fast).
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  
  // Calculate squared speed (avoids slow sqrt() function)
  float swingSpeedSq = gyro.x()*gyro.x() + gyro.y()*gyro.y() + gyro.z()*gyro.z();

  // Check for a hit
  if (digitalRead(BUTTON1) == LOW && 
      swingSpeedSq > SWING_THRESHOLD_SQ && 
      (now - lastHit > REST_DELAY) && 
      gyro.y() > 0) { 
    
    // NOW get the slower orientation data
    sensors_event_t event;
    if (!bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER)) {
      return; 
    }
    
    // Check which hit zone (if any) was struck
    // We still pass 'roll' in, but the function will ignore it
    int zone = detectZoneFast(event.orientation.x, event.orientation.y, event.orientation.z);
    
    if (zone != -1) {
      Serial.write(zoneIDs[zone]);
      lastHit = now; 
      lastZone = zone;
    }
  }
}

/**
 * This function now only checks Yaw and Pitch.
 * It ignores the 'roll' (wrist twist).
 */
int detectZoneFast(float yaw, float pitch, float roll) { // 'roll' is now ignored
  float minDistSq = zoneThresholdSq;
  int closest = -1;

  for (int i = 0; i < 5; i++) {
    float dy = angleDiffFast(yaw, zones[i].yaw);
    float dp = pitch - zones[i].pitch;
    //float dr = roll - zones[i].roll; // <-- REMOVED
    
    // <-- CHANGED: Distance is now only calculated from Yaw and Pitch
    float distSq = dy*dy + dp*dp; 
    
    if (distSq < minDistSq) {
      minDistSq = distSq;
      closest = i;
    }
  }
  return (minDistSq < zoneThresholdSq) ? closest : -1;
}

/**
 * Helper function to find the shortest angle difference
 */
inline float angleDiffFast(float a, float b) {
  float d = fabs(a - b);
  return (d > 180.0f) ? (360.0f - d) : d;
}