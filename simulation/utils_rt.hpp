#pragma once
#include <cmath>
#include "hardware/adc.h"
#include "pico/time.h"

inline float temp_from_raw(uint16_t raw){
    constexpr float VREF=3.3f, ADC=4096.f, T0=27.f, V0=0.706f, S=0.001721f;
    float v = (raw * VREF) / ADC;
    return T0 - (v - V0) / S;
}

// Kuantal basamakları yumuşatmak için oversampling (128 okuma)
inline float read_temp_oversampled(int N=128, int us_gap=1500){
    uint32_t sum=0;
    for(int i=0;i<N;++i){ sum += adc_read(); sleep_us(us_gap); }
    return temp_from_raw((uint16_t)(sum/(uint32_t)N));
}

inline float fast_sigmoid(float x){
    float ax = x<0 ? -x : x;
    float y  = x / (1.f + ax);
    return 0.5f*(y+1.f); // ~sigmoid, expf yok -> küçük binary
}
