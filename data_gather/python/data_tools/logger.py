import serial
import time
import csv

# --- CONFIG ---
PORT = "COM3"          
BAUD = 9600
CSV_PATH = "pico_log_20min.csv"
PHASE_IDLE_SEC = 10*60  
PHASE_LOAD_SEC = 10*60 

def read_and_log(ser, writer, duration_sec):
    t_end = time.time() + duration_sec
    while time.time() < t_end:
        line = ser.readline().decode(errors="ignore").strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split(",")
        if len(parts) == 3:
            writer.writerow(parts)
            print(parts)

def main():
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Connected to {PORT} @ {BAUD}")
    time.sleep(2)
    ser.reset_input_buffer()

    with open(CSV_PATH, "w", newline="") as f:
        wr = csv.writer(f)
        wr.writerow(["uptime_ms","temp_c","load"])  # header

        # Faz 1: IDLE (L0)
        print("Phase 1: IDLE (10 min)")
        ser.write(b"L0\n")
        read_and_log(ser, wr, PHASE_IDLE_SEC)

        # Faz 2: LOAD (L1)
        print("Phase 2: LOAD (10 min)")
        ser.reset_input_buffer()
        ser.write(b"L1\n")
        read_and_log(ser, wr, PHASE_LOAD_SEC)

    ser.close()
    print(f"Done. Saved: {CSV_PATH}")

if __name__ == "__main__":
    main()
