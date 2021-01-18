#include <Arduino.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Adafruit_EPD.h"
#include "Adafruit_ThinkInk.h"
#include "Constants.h"
#include "DHT.h"
#include "Module.h"

#define DHT_TYPE DHT22
// This is pin A-5
#define DHT_PIN 4
// This is pin A-4
#define LDR_PIN 36


#define TIMEZONE_ADJUSTMENT -18000 // UTC minus 5 hours * 60 minutes * 60 seconds
#define uSEC_TO_SEC 1000000
#define INTERVAL_SEC 30

#define CACHE_SIZE 50
#define LOOPS_BEFORE_TIME_SYNC 20

typedef struct {
    long int time;
    float temperature;
    int humidity;  // 0 - 100%
    int lightIntensity;
    float batteryVolt;
} Datapoint;

// Need to store outside of class, because classes are not stored in RTC memory
RTC_DATA_ATTR uint pointsPerSync = 1;
RTC_DATA_ATTR uint loopCount = 0;
RTC_DATA_ATTR int unsyncedDatapoints = 0;
RTC_DATA_ATTR Datapoint datapoints[CACHE_SIZE];

DHT dht(DHT_PIN, DHT_TYPE);

// Pin outs for ESP32 featherwing
#ifndef EPD_PORTS
    #define EPD_PORTS
    #define SRAM_CS 32
    #define EPD_CS 15
    #define EPD_DC 33
    #define EPD_RESET -1  // can set to -1 and share with microcontroller Reset!
    #define EPD_BUSY -1   // can set to -1 to not use a pin (will wait a fixed delay)
#endif

class HumiditySensor : public Module {
   public:
    void run_main() {
        Serial.begin(115200);
        while (!Serial) {
            delay(10);
        }

        // Triggers on the first iteration
        if (loopCount % LOOPS_BEFORE_TIME_SYNC == 0) {
            setClock();
        }

        bool success = measure(&datapoints[unsyncedDatapoints]);

        if (success) {
            int pos = unsyncedDatapoints;

            // We now have 1 more datapoint to sync, never more than the cache can fit.
            unsyncedDatapoints = min(unsyncedDatapoints + 1, CACHE_SIZE - 1);

            updateDisplay(datapoints[pos]);
        }

        if (unsyncedDatapoints >= pointsPerSync) {
            boolean success = flush_to_influxdb();

            if (success) {
                unsyncedDatapoints = 0;

                // Slowly reduce the number of times we sync
                if (pointsPerSync < 15) {
                    pointsPerSync++;
                    Serial.printf("Raised pointsPerSync to %i\n", pointsPerSync);
                }
            }
        }

        loopCount++;
        // Go to sleep
        esp_sleep_enable_timer_wakeup(INTERVAL_SEC * uSEC_TO_SEC);
        esp_deep_sleep_start();

        if (WiFi.status() == WL_CONNECTED) {
            WiFi.disconnect(true);
        }
    }

    void run_loop() {}

   private:
    // 2.9" Grayscale Featherwing or Breakout:
    ThinkInk_290_Grayscale4_T5 display = ThinkInk_290_Grayscale4_T5(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

    bool measure(Datapoint* datapoint) {
        // Measure
        dht.begin();

        int attempt = 0;
        while (attempt < 5) {
            float temperature = dht.readTemperature();
            float humidity = dht.readHumidity();

            bool isValid = ! (isnan(temperature) || isnan(humidity) || temperature > 150);
            if (isValid) {
                    datapoint->time = (long int)time(NULL);
                    datapoint->temperature = temperature;
                    datapoint->humidity = (int)humidity;
                    datapoint->lightIntensity = analogRead(LDR_PIN);
                    datapoint->batteryVolt = 0.0;

                    return true;
            }

            delay(50 * attempt);
            attempt++;
        }

        return false;
    }

    bool connect_to_wifi() {
        WiFi.begin(Constants::ssid, Constants::password);

        int attempt = 0;
        while (WiFi.status() != WL_CONNECTED && attempt < 30) {
            Serial.println("Connecting to WiFi..");

            delay(50 * attempt);
            attempt++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("Connected. My IP address is: ");
            Serial.println(WiFi.localIP());
        }

        return WiFi.status() == WL_CONNECTED;
    }

    bool setClock() {
        if (!connect_to_wifi()) {
            return false;
        }

        configTime(0, 0, Constants::ntpServer);
        time_t nowSecs = time(NULL);
        while (nowSecs < 8 * 3600 * 2) {
            delay(500);
            yield();
            nowSecs = time(NULL);
        }

        long int seconds;
        seconds = (long int)time(NULL);
        Serial.printf("NTP synced. Epoch time: %ld\n", seconds);
    }

    void updateDisplay(Datapoint& datapoint) {
        struct tm  ts;
        char       buf[80];
        long ldatetime = datapoint.time + TIMEZONE_ADJUSTMENT;
        ts = *localtime(&ldatetime);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S EDT", &ts);

        // Write to e-ink display
        display.begin(THINKINK_GRAYSCALE4);
        display.clearBuffer();
        display.cp437(true); // Enable Code Page 437-compatible charset.
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.setTextWrap(true);
        display.setTextColor(EPD_BLACK);
        display.printf(
            " \n"
            " Hello there!\n"
            " \n"
            " Time:        %ld\n"
            " Time:        %s\n"
            " Humidity:    %i%%\n"
            " Temperature: %.2f C\n"
            " Light:       %-4i / %i%%\n" // append spaces so the / aligns with next line
            " Battery:     %-3.1fV / %i%%",
            datapoint.time,
            buf,
            datapoint.humidity,
            datapoint.temperature,
            datapoint.lightIntensity, datapoint.lightIntensity * 100 / 4095,
            datapoint.batteryVolt, voltageToPercent(datapoint.batteryVolt)
            );
        display.display();
        display.powerDown();
    }

    HTTPClient http;
    bool flush_to_influxdb() {
        if (! connect_to_wifi()) {
            // Could not connect to wifi; cannot sync
            return false;
        }

        int attempt = 0;
        while (attempt < 5 && !http.begin(Constants::influxdbUrl)) {
            delay(50);
            attempt++;
        }

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        bool success = true;

        // Sync all unsynced data points.
        // If previous syncs failed, this may be higher than pointsPerSync.
        for (int i = 0; i < unsyncedDatapoints; i++) {
            success = success && send_metric(http, datapoints[i]);
        }

        http.end();

        return success;
    }

    bool send_metric(HTTPClient& http, Datapoint& datapoint) {
        char payload[400];

        snprintf(payload, sizeof(payload),
                 "esp32 temperature=%.3f,humidity=%i,light_intensity=%i %ld000000000\n",
                 datapoint.temperature,
                 datapoint.humidity,
                 100 * datapoint.lightIntensity / 4095,
                 datapoint.time);

        Serial.println(payload);

        int attempt = 0;
        while (attempt < 5) {
            delay(50 * attempt);  // Don't wait on first run, wait a bit longer on every retry.
            attempt++;

            int httpCode = http.POST(payload);
            if(200 <= httpCode && httpCode < 300) {
                return true;
            }
        }
        return false;
    }

    int voltageToPercent(float v) {
        // TODO
        return 0;
    }
};
