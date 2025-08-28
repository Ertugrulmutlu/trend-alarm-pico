#include <cstdio>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "utils.hpp"

static bool load_flag = false;

int main() {
    stdio_init_all();

    // Wait until USB serial is connected (optional)
    while (!stdio_usb_connected()) { sleep_ms(100); }
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("# BOOT OK\n");

    // Init ADC and internal temp sensor
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // ADC4 = internal temp sensor

    // Print CSV header
    printf("uptime_ms,temp_c\n");
    int c = getchar_timeout_us(0);

    while (true) {
        int c = getchar_timeout_us(0);
        if (c == 'L'){
            int c2 =getchar_timeout_us(1000);
            if (c2== '1') load_flag = true;
            else if (c2== '0') load_flag = false;
            (void)getchar_timeout_us(1000);
        }
        float temp_c = Utils::read_internal_temp_c();
        uint32_t uptime = to_ms_since_boot(get_absolute_time()); 
        printf("%u,%.3f,%d\n", uptime, temp_c, (int)load_flag);
        if (load_flag) {
            Utils::heavy_work_ms(1000); 
            //sleep_ms(100);
        } else {
            sleep_ms(1000);
        }     
    }
    return 0;
}
