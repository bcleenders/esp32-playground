#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Module.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"

class PWM : public Module {
   public:
    void run_main() {
        Serial.begin(115200);

        ledc_timer_config_t timer_config = create_default_timer_config();
        ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

        ledc_channel_config_t channel_conf = initialize_channel_config();
        ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
    }

    void run_loop() {
        sleep(5000);

        // sleep(5000);
        // ledc_stop(LEDC_HIGH_SPEED_MODE, chan, LOW);
    }

   private:
    ledc_channel_t chan = LEDC_CHANNEL_0;
    static const uint8_t ledPin = 16;

    static ledc_timer_config_t create_default_timer_config(void) {
        ledc_timer_config_t ledc_time_config;
        memset(&ledc_time_config, 0, sizeof(ledc_timer_config_t));
        ledc_time_config.speed_mode = LEDC_HIGH_SPEED_MODE;
        ledc_time_config.timer_num = LEDC_TIMER_0;
        /*
        * maximal frequency is 80000000 / 2^duty_resolution
        * available duty levels are (2^duty_resolution)-1, where duty_resolution can be 1-15
        *
        * 113500 Hz is achievable with duty_resolution <= 9
        * (max freq for dr=9 is 156250 Hz)
        */
        ledc_time_config.duty_resolution = LEDC_TIMER_9_BIT;
        ledc_time_config.freq_hz = 113500;
        return ledc_time_config;
    }

    static ledc_channel_config_t initialize_channel_config(void) {
        ledc_channel_config_t config;
        memset(&config, 0, sizeof(ledc_channel_config_t));
        config.gpio_num = ledPin;
        config.speed_mode = LEDC_HIGH_SPEED_MODE;
        config.channel = LEDC_CHANNEL_0;
        config.intr_type = LEDC_INTR_DISABLE;
        config.timer_sel = LEDC_TIMER_0;
        /*
        * range is 2^duty_resolution
        * to set to high value 50% of the time, set to 2^duty_resolution / 2
        */
        config.duty = 256;  // 2**9 / 2
        config.hpoint = 0;
        return config;
    }
};
