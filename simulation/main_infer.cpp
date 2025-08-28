#include <cstdio>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"

#include "model_params.hpp"   // PC’den çıktı
#include "utils_rt.hpp"
#include "slope_buf.hpp"

int main(){
    stdio_init_all();
    while(!stdio_usb_connected()) { sleep_ms(100); }
    setvbuf(stdout,nullptr,_IONBF,0);
    printf("# BOOT OK (inference)\n");
    printf("uptime_ms,temp_c,slope,prob,state\n");

    // ADC init
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    SlopeBuf buf(WIN_S);
    bool alarm=false; int hold=0;

    while(true){
        // 1) Ölçüm (oversampling ile)
        float T = read_temp_oversampled(128, 1500);
        uint32_t ms = to_ms_since_boot(get_absolute_time());
        float ts = ms * 1e-3f;

        // 2) Pencereye ekle + slope hesapla
        buf.push(ts, T);
        float m = buf.slope(); // °C/s

        // 3) Logistic (z-score -> prob)
        float z = (m - MU) / (SIGMA > 1e-9f ? SIGMA : 1.f);
        float prob = fast_sigmoid(A_COEF*z + B_INT);

        // 4) Histerezis + hold
        if (!alarm && prob >= P_ON) {
            if (++hold >= HOLD_S) { alarm = true; hold = 0; }
        } else if (alarm && prob <= P_OFF) {
            if (++hold >= HOLD_S) { alarm = false; hold = 0; }
        } else {
            hold = 0;
        }

        // 5) Log
        printf("%u,%.3f,%.5f,%.2f,%s\n", ms, T, m, prob, alarm?"ALARM":"OK");

        sleep_ms(1000); // 1 Hz
    }
    return 0;
}
