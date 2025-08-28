#pragma once

struct SlopeBuf {
    int win;                 // saniye (model_params.hpp: WIN_S)
    static const int MAXN=180; // emniyet (180s'e kadar)
    float t[MAXN], y[MAXN];  // zaman (s), sıcaklık (°C)
    int n=0;

    explicit SlopeBuf(int win_s): win(win_s){ if(win>MAXN) win=MAXN; }

    void push(float ts, float val){
        if(n < win){ t[n]=ts; y[n]=val; ++n; }
        else { // küçük pencere -> basit kaydırma
            for(int i=1;i<win;++i){ t[i-1]=t[i]; y[i-1]=y[i]; }
            t[win-1]=ts; y[win-1]=val;
        }
    }

    // Merkezlenmiş OLS slope (°C/s)
    float slope() const {
        if(n < 20) return 0.f;
        float tm=0.f, ym=0.f;
        for(int i=0;i<n;++i){ tm+=t[i]; ym+=y[i]; }
        tm/=n; ym/=n;
        float num=0.f, den=0.f;
        for(int i=0;i<n;++i){
            float dt=t[i]-tm, dy=y[i]-ym;
            num += dt*dy; den += dt*dt;
        }
        if(den<=1e-9f) return 0.f;
        return num/den;
    }
};
