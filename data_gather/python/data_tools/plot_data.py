import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

CSV_PATH = r".//data//pico_log_20min.csv"   # kendi dosya adını yaz

def rolling_slope(t, y, win=120, min_samples=20):
    """120s pencere ile OLS slope (°C/s) hesapla"""
    t = np.asarray(t, float)
    y = np.asarray(y, float)
    out = np.full(len(t), np.nan)
    for i in range(len(t)):
        j0 = np.searchsorted(t, t[i]-win)
        if i - j0 + 1 >= min_samples:
            tt, yy = t[j0:i+1], y[j0:i+1]
            tm, ym = tt.mean(), yy.mean()
            dt, dy = tt-tm, yy-ym
            den = (dt*dt).sum()
            if den > 1e-9:
                out[i] = (dt*dy).sum()/den
    return out

def main():
    df = pd.read_csv(CSV_PATH)
    df["t_s"] = df["uptime_ms"] / 1000.0

    df["slope"] = rolling_slope(df["t_s"], df["temp_c"], win=120)

    plt.figure(figsize=(10,6))
    plt.subplot(2,1,1)
    idle = df[df["load"]==0]
    load = df[df["load"]==1]
    plt.plot(idle["t_s"], idle["temp_c"], "b.", label="idle")
    plt.plot(load["t_s"], load["temp_c"], "r.", label="load")
    plt.ylabel("Temperature (°C)")
    plt.legend()
    plt.grid(True)

    plt.subplot(2,1,2)
    plt.plot(df["t_s"], df["slope"], "k-")
    plt.axhline(0, color="gray", linestyle="--")
    plt.ylabel("Slope (°C/s)")
    plt.xlabel("Time (s)")
    plt.grid(True)

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
