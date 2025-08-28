# Pico Edge Trend Alarm (RP2040)

This project demonstrates a **trend-based early alarm system** running entirely on a Raspberry Pi Pico. The system uses the internal temperature sensor, trains a **logistic regression model** on PC, and then deploys the model back to the Pico for **real-time inference**. The alarm is triggered not by absolute temperature but by the **trend (slope of ΔT/Δt)**, which is a common technique in predictive maintenance.

---

## Project Structure

```
pico-edge-trend-alarm/
├─ datagather/
│  ├─ cpp/               # Pico firmware for data collection
│  └─ python/            # PC-side logger for serial to CSV
│
├─ train/                # Model training (logistic regression)
│  ├─ train_logreg.py
│  └─ requirements.txt
│
├─ simulation/           # Edge inference firmware
│  ├─ main_infer.cpp
│  ├─ slope_buf.hpp
│  ├─ utils_rt.hpp
│  └─ model_params.hpp   # Generated from training phase
│
├─ data/
│  └─ pico_log_20min.csv # Example dataset
│
├─ requirement.txt 
└─ README.md
```

---

## Requirements

### Hardware

* Raspberry Pi Pico (RP2040)
* USB cable

### Software

* Pico SDK + ARM GCC toolchain
* CMake + Ninja
* Python 3.9+

Install Python dependencies:

```bash
cd train
pip install -r requirements.txt
```

---

## 1. Data Gathering

### 1.1 Flash Pico with Data Gather Firmware

```bash
cd datagather/cpp
mkdir build && cd build
cmake ..
ninja
```

Copy the generated `.uf2` file to Pico (BOOTSEL mode).

### 1.2 Log Data on PC

```bash
cd datagather/python
python logger.py
```

* Adjust the serial **PORT** in `logger.py`.
* Output will be saved as `pico_log_20min.csv` (move it to `data/`).

---

## 2. Training the Model

```bash
cd train
python train_logreg.py
```

* Input: `data/pico_log_20min.csv`
* Feature: **slope (ΔT/Δt)** computed with a 120s sliding window.
* Model: Logistic Regression.
* Outputs:

  * `kws_edge_params.json` (for reference)
  * `model_params.hpp` (C++ header for inference)

---

## 3. Edge Inference on Pico

### 3.1 Copy Parameters

Move the generated `model_params.hpp` to `simulation/`.

### 3.2 Build Inference Firmware

```bash
cd simulation
mkdir build && cd build
cmake -G "Ninja" ..
ninja
```

Flash the `.uf2` file to Pico.

### 3.3 Monitor Output

Serial logs:

```
uptime_ms,temp_c,slope,prob,state
2494,24.330,0.00000,0.32,OK
...
41185,24.798,0.02881,0.63,ALARM
```

* **OK → ALARM** when slope trend suggests heating (prob ≥ 0.6 for 8s).
* **ALARM → OK** when prob ≤ 0.4 for 8s.

---

## How It Works

1. **Data Gathering**: Pico reads internal temperature and logs via USB.
2. **Feature Extraction**: 120s sliding window slope calculation (°C/s).
3. **Training**: Logistic Regression on PC, parameters exported.
4. **Edge Inference**: Pico computes slope, normalizes, applies logistic regression + hysteresis.

This approach catches **early heating trends** rather than waiting for absolute temperature thresholds.

---

## Troubleshooting

* `Unknown CMake command "pico_enable_stdio_usb"` → Ensure `pico_sdk_init()` is called and `pico_sdk_import.cmake` is included correctly.
* No serial output → Check `stdio_init_all()` and that USB stdio is enabled.
* Alarm too early → Adjust `P_ON` (e.g., 0.65) or `HOLD_S` (e.g., 10).
* ADC steps too coarse → Use oversampling (64–128 samples) + EMA.

---

⭐ If you like this project, please give it a star on GitHub!

📖 For readers looking for **more technical depth**, check out the accompanying blog series:

* Part 1: Data Gathering on Pico
* Part 2: Training Logistic Regression
* Part 3: Real-Time Edge Inference
  
---
## License

MIT License
