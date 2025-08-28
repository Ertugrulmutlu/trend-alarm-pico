#pragma once
#include "hardware/adc.h"
#include "pico/time.h"
#include <cmath>

class Utils {
public:
    static void heavy_work_ms(uint32_t ms){
        absolute_time_t until = make_timeout_time_ms(ms);
        volatile float acc = 0.0f;
        while (!time_reached(until)){
            for(int i =0; i <50000; ++i){
                acc += sinf((float)i) * cosf((float)i) + sqrtf((float)(i + 1));
            }
        }
       (void)acc;  
    }
    static float read_internal_temp_c() {
        constexpr float VREF = 3.3f;
        constexpr float ADC_SCALE = 4096.0f;   // 12-bit
        constexpr float T0 = 27.0f;            // °C
        constexpr float V_AT_T0 = 0.706f;      // Volt
        constexpr float SLOPE = 0.001721f;     // V/°C

        uint16_t raw = adc_read();
        float v = (raw * VREF) / ADC_SCALE;
        return T0 - (v - V_AT_T0) / SLOPE;
    }
};
