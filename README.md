# BoomBaStick

**BoomBaStick** is a motion-controlled electronic drumstick that transforms physical stick movements into digital drum sounds. It uses an Adafruit BNO055 IMU sensor to detect swing motion and determine the position of the drumstick, while a Python application processes the sensor commands and plays the corresponding drum samples in real time.

The project combines embedded systems, motion sensing, serial communication, and real-time audio processing to create an interactive and low-latency drumming experience.

---

## Features

- **Motion-Based Drum Detection:** Uses the BNO055 IMU for precise swing and orientation tracking.
- **Multiple Drum Zones:** Configurable spatial zones for Snare, Hi-Hat, Tom, Crash, and Ride.
- **Real-Time Audio Playback:** Low-latency sample playback using Python and Pygame.
- **High-Speed Serial Communication:** Runs at 230,400 baud to minimize input lag.
- **Custom Calibration:** Easy yaw and pitch mapping for physical kit positioning.
- **Button-Based Triggers:** Auxiliary hardware buttons for secondary sounds (e.g., Kick).
- **Optimized Processing:** Gyroscope-first hit detection prevents MCU cycle waste.
- **Multi-Channel Audio:** Pygame mixer channels allow rapid, overlapping hits without clipping.

---

## How It Works

BoomBaStick consists of two main components: the motion detection system and the audio playback system.

### 1. Motion Detection

The microcontroller communicates with the BNO055 9-axis absolute orientation sensor through I2C. The gyroscope is continuously monitored to detect a sufficiently fast swing.

When a valid swing is detected, the system reads the stick's yaw and pitch and compares them against predefined drum-zone positions. The closest matching zone is selected and represented by a single serial character:

| Command | Drum |
|:-------:|------|
| `0`     | Snare |
| `1`     | Hi-Hat |
| `2`     | Tom |
| `3`     | Crash |
| `4`     | Ride |
| `5`     | Kick |

### 2. Audio Playback

A Python application listens to the microcontroller's serial output. When a drum command is received, it triggers the corresponding audio sample using `pygame.mixer`.

The audio system uses multiple mixer channels to minimize playback latency and allow sounds to overlap naturally.

---

## System Architecture

```text
        +----------------------+
        |   Motion / Swing     |
        |     of Drumstick     |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |      BNO055 IMU      |
        |  Gyroscope + Euler   |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |   Drum Zone Detection|
        |   Yaw + Pitch Match  |
        +----------+-----------+
                   |
             Serial Command
                0 - 5
                   |
                   v
        +----------------------+
        |   Python Controller  |
        |      PySerial        |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |    Pygame Mixer      |
        |   Audio Playback     |
        +----------+-----------+
                   |
                   v
             Drum Sound

```

---

## Technology Stack

### Hardware

* Microcontroller / Arduino-compatible development board
* Adafruit BNO055 9-DOF IMU sensor
* Push buttons
* Electronic drumstick hardware chassis

### Embedded Software

* C / C++
* Arduino Framework
* I2C Communication (`Wire` library)
* Adafruit BNO055 library & Adafruit Unified Sensor Driver

### Desktop Software

* Python 3
* PySerial
* Pygame (`pygame.mixer`)

---

## Project Structure

```text
boombastick/
│
├── DOCUMENTATION/
│   └── Embedded.pdf
│
├── ONESTICKCALIBRATION/
│   └── ONESTICKCALIBRATION.ino
│
├── playSound_LEFT/
│   ├── l.py
│   └── SAMPLES/
│       ├── SNARES/
│       ├── HIHATS/
│       ├── TOMS/
│       ├── CYMBALS/
│       └── KICKS/
│
├── playSound_RIGHT/
│   ├── ...
│   └── SAMPLES/
│
├── FINAL CALIBRATION.ino.txt
├── FINAL ONESTICK.ino.txt
└── README.md

```

---

## Drum Zone Calibration

Because the physical position of the drum zones can vary depending on the setup, BoomBaStick includes a dedicated calibration program.

### Calibration Steps

1. Connect the BNO055 sensor to your microcontroller.
2. Upload and run `ONESTICKCALIBRATION/ONESTICKCALIBRATION.ino`.
3. Calibrate the BNO055 sensor by moving it in a figure-8 motion until fully calibrated.
4. Point the drumstick toward the physical **Snare** position and press the calibration button.
5. Repeat the process sequentially for the remaining zones:
* Snare
* Hi-Hat
* Tom
* Crash
* Ride


6. Copy the generated `DrumZone` coordinates into the main program.

The calibration utility records the yaw and pitch values for each drum position and outputs formatted values ready for the main sketch.

---

## Configuration

### Serial Port

Update the COM port in the Python script to match your connected device:

```python
comport = 'COM8'

```

The Python application communicates with the microcontroller at:

```python
serial.Serial(comport, 230400, timeout=0)

```

### Drum Zones

The drum positions are defined using yaw and pitch coordinates:

```cpp
DrumZone snare = {359.5, 10.9};
DrumZone hihat = {242.2, 24.8};
DrumZone tom   = {192.5, 41.1};
DrumZone crash = {247.8, 36.1};
DrumZone ride  = {139.4, 43.7};

```

These values can be updated after running the calibration sketch.

### Hit Detection

The system first checks the gyroscope's movement speed before reading orientation data. This reduces unnecessary orientation calculations and maximizes responsiveness.

A hit is detected when the swing exceeds the configured threshold and satisfies detection conditions. A short rest period is implemented to prevent accidental re-triggers.

---

## Getting Started

### 1. Clone the Repository

```bash
git clone [https://github.com/adrian-cali/boombastick.git](https://github.com/adrian-cali/boombastick.git)
cd boombastick

```

### 2. Install Python Dependencies

```bash
pip install pyserial pygame

```

### 3. Install Arduino Libraries

Install the following via the Arduino IDE Library Manager:

* **Adafruit BNO055**
* **Adafruit Unified Sensor**

### 4. Upload the Calibration Sketch

1. Open `ONESTICKCALIBRATION/ONESTICKCALIBRATION.ino`.
2. Upload the sketch to your microcontroller and calibrate the BNO055 sensor.
3. Record the generated drum-zone coordinates.

### 5. Configure the Main Sketch

1. Update the drum-zone coordinates in `FINAL ONESTICK.ino.txt`.
2. Rename the file:
```bash
mv "FINAL ONESTICK.ino.txt" "FINAL ONESTICK.ino"

```


3. Upload `FINAL ONESTICK.ino` to the microcontroller.

### 6. Configure the Python Controller

1. Open `playSound_LEFT/l.py`.
2. Update the serial port:
```python
comport = 'COM8'

```


3. Ensure the corresponding `SAMPLES/` directory is present in the folder.

### 7. Run the Audio Controller

```bash
python playSound_LEFT/l.py

```

The Python program will now listen for incoming serial commands and play the mapped audio samples.

---

## Audio Mapping

The Python controller maps each serial character to a specific drum sound:

| Command | Sound Triggered |
| --- | --- |
| `0` | Snare |
| `1` | Hi-Hat |
| `2` | Tom |
| `3` | Crash |
| `4` | Ride |
| `5` | Kick |

The audio engine uses multiple Pygame mixer channels to support rapid, overlapping hits without stopping active voices.

---

## Performance Considerations

* **Optimized Sensor Polling:** The embedded program reads the gyroscope first and queries absolute orientation only when a swing crosses the velocity threshold.
* **High-Speed Communication:** Configured to run on a 400 kHz I2C clock and 230,400 baud UART serial.
* **Low-Latency Audio:** The Python audio controller uses non-blocking serial reads alongside a minimized audio buffer to eliminate audible output lag.

---

## Documentation

Detailed documentation and embedded system design schematics are available in the repository:

* [`DOCUMENTATION/Embedded.pdf`](https://www.google.com/search?q=DOCUMENTATION/Embedded.pdf)

---

## Future Improvements

* [ ] Wireless communication between the drumstick and computer (ESP-NOW / BLE)
* [ ] Support for additional, dynamic drum zones
* [ ] User-customizable drum sample sets
* [ ] Software-adjustable hit sensitivity
* [ ] Automated in-app calibration routine
* [ ] Visual UI feedback for hit registration
* [ ] Standard MIDI output support
* [ ] Bluetooth LE MIDI support
* [ ] Standalone operation without a computer (onboard DAC)
* [ ] Multi-stick synchronization handling

```

```
