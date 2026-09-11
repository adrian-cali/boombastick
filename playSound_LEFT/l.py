import serial
import time
from pygame import mixer

# ----------------------- CHANGEEEE -----------------------
comport = 'COM8'
# ---------------------------------------------------------
# comport = 'COM8'


# === AUDIO SETUP ===
# OPTIMIZATION: Initialize with a smaller buffer for low latency.
# Default is often 1024 or 2048. 256 is much faster.
# If you hear clicks/pops, try 512.
mixer.init(frequency=44100, buffer=256)
mixer.set_num_channels(16)

print("Loading samples...")
try:
    snare = mixer.Sound(r'SAMPLES\SNARES\07_Snare_16_SP.wav')
    hihat = mixer.Sound(r'SAMPLES\HIHATS\Boom-Bap Hat CL 53.wav')
    tom = mixer.Sound(r'SAMPLES\TOMS\TOM.wav')
    crash = mixer.Sound(r'SAMPLES\CYMBALS\07_Perc_05_SP.wav')
    ride = mixer.Sound(r'SAMPLES\CYMBALS\MachineRide.wav')
    kick = mixer.Sound(r'SAMPLES\KICKS\Boom-Bap Kick 53.wav')
except FileNotFoundError as e:
    print(f"\n--- ERROR ---")
    print(f"Could not load sample file: {e.filename}")
    print("Please check your 'SAMPLES' folder path and file names.")
    print("Exiting.")
    exit()


drums = [snare, hihat, tom, crash, ride, kick]
DRUM_NAMES = ['Snare', 'Hi-Hat', 'Tom', 'Crash', 'Ride', 'Kick']

for d in drums:
    d.set_volume(1.0)

# === SERIAL SETUP ===
# OPTIMIZATION: Set timeout=0 for fully non-blocking reads.

ser = serial.Serial(comport, 230400, timeout=0)
    

time.sleep(1) # Give serial connection time to establish
print(f"Listening on {comport}...")

# OPTIMIZATION: Pre-calculate the ASCII offset for '0'
CMD_OFFSET = ord('0')

while True:
    try:
        # OPTIMIZATION: Read all waiting bytes from the buffer at once.
        if ser.in_waiting > 0:
            data_bytes = ser.read(ser.in_waiting)
            
            # OPTIMIZATION: Process each byte directly
            for byte in data_bytes:
                # 'byte' is an int (e.g., 48 for '0').
                # Subtracting the offset is much faster than decoding.
                idx = byte - CMD_OFFSET
                
                if 0 <= idx <= 5:
                    drums[idx].play()
                    
                    # NOTE: This print() statement adds a small amount of latency.
                    # For maximum performance, you can comment it out.
                    print(f"Played: {DRUM_NAMES[idx]}")
                # else: ignore any bytes that aren't 0-5

        # A tiny sleep prevents the loop from consuming 100% CPU
        # on some systems. 0.001 is a good starting point.
        # You can remove it if latency is still too high.
        time.sleep(0.001) 

    except KeyboardInterrupt:
        print("\nExiting...")
        break
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        # Add a small delay to prevent error spam
        time.sleep(0.1) 

ser.close()
mixer.quit()
print("Serial port closed. Goodbye.")
