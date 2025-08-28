import pandas as pd, numpy as np, json
from sklearn.linear_model import LogisticRegression

# === config ===
CSV_PATH = r".//data//pico_log_20min.csv"   # senin kaydettiğin csv
WIN_S = 120                       # slope penceresi (saniye)
MIN_SAMPLES = 20
P_ON, P_OFF, HOLD_S = 0.60, 0.40, 8
# ==============

def rolling_slope(t, y, win=120, min_samples=20):
    """Kayan pencere ile OLS slope (°C/s)."""
    t = np.asarray(t,float); y = np.asarray(y,float)
    out = np.full(len(t), np.nan)
    for i in range(len(t)):
        j0 = np.searchsorted(t, t[i]-win)
        if i-j0+1 >= min_samples:
            tt, yy = t[j0:i+1], y[j0:i+1]
            tm, ym = tt.mean(), yy.mean()
            dt, dy = tt-tm, yy-ym
            den = (dt*dt).sum()
            if den > 1e-9:
                out[i] = (dt*dy).sum()/den
    return out

df = pd.read_csv(CSV_PATH)
df["t_s"] = df["uptime_ms"]/1000.0

df["slope"] = rolling_slope(df["t_s"], df["temp_c"], WIN_S)
data = df.dropna(subset=["slope"]).copy()

X = data[["slope"]].to_numpy()
y = data["load"].astype(int).to_numpy()

mu = float(X.mean())
sigma = float(X.std(ddof=1) or 1.0)
Xz = (X - mu)/sigma

clf = LogisticRegression()
clf.fit(Xz, y)
a = float(clf.coef_[0,0])
b = float(clf.intercept_[0])
acc = clf.score(Xz, y)

print("\n=== EĞİTİM SONUCU ===")
print(f"örnek: {len(y)}  idle={ (y==0).sum() }  load={ (y==1).sum() }")
print(f"pencere={WIN_S}s  mu={mu:.6g}  sigma={sigma:.6g}")
print(f"a={a:.6g}  b={b:.6g}  acc={acc*100:.1f}%")
print(f"p_on={P_ON}  p_off={P_OFF}  hold={HOLD_S}s")

params = {
    "WIN_S": WIN_S, "mu": mu, "sigma": sigma,
    "a": a, "b": b,
    "p_on": P_ON, "p_off": P_OFF, "hold_sec": HOLD_S
}
with open("kws_edge_params.json","w") as f: json.dump(params,f,indent=2)

hdr = f"""#pragma once
// auto-generated from {CSV_PATH}
static constexpr int   WIN_S   = {WIN_S};
static constexpr float MU      = {mu:.8f}f;
static constexpr float SIGMA   = {sigma:.8f}f;
static constexpr float A_COEF  = {a:.8f}f;
static constexpr float B_INT   = {b:.8f}f;
static constexpr float P_ON    = {P_ON:.2f}f;
static constexpr float P_OFF   = {P_OFF:.2f}f;
static constexpr int   HOLD_S  = {HOLD_S};
"""
open("model_params.hpp","w").write(hdr)

print("\nKaydedildi -> kws_edge_params.json, model_params.hpp")
