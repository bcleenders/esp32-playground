#include <Arduino.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

#include "Module.h"
#include "Constants.h"

#define uSEC_TO_SEC 1000000
#define INTERVAL_SEC 4

#define LOOPS_BEFORE_FLUSH 5
#define LOOPS_BEFORE_TIME_SYNC 10

typedef struct {
  long int time;
  int value;
} Datapoint;

// Need to store outside of class, because classes are not stored in RTC memory
RTC_DATA_ATTR uint loopCount = 0;
RTC_DATA_ATTR Datapoint datapoints[LOOPS_BEFORE_FLUSH];

class LDR: public Module {
public:
  void run_main() {
    Serial.begin(115200);

    if (loopCount % LOOPS_BEFORE_TIME_SYNC == 0) {
      setClock();
    }

    datapoints[loopCount % LOOPS_BEFORE_FLUSH].time = (long int) time(NULL);
    datapoints[loopCount % LOOPS_BEFORE_FLUSH].value = analogRead(analogPin);

    // Flush every iteration for the first 10 runs, to get quick feedback.
    // After that, only flush every Nth loop
    if (loopCount < 10 || loopCount % LOOPS_BEFORE_FLUSH == 0) {
      flush_to_influxdb();
    }

    loopCount++;
    // Go to sleep
    esp_sleep_enable_timer_wakeup(INTERVAL_SEC * uSEC_TO_SEC);
    esp_deep_sleep_start();
  }

  void run_loop() {
  }

private:
  const int analogPin = 34;

  void connect_to_wifi() {
        WiFi.begin(Constants::ssid, Constants::password);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

        Serial.print("Connected. My IP address is: ");
        Serial.println(WiFi.localIP());
  }

  void setClock() {
    connect_to_wifi();

    configTime(0, 0, Constants::ntpServer);
    time_t nowSecs = time(NULL);
    while (nowSecs < 8 * 3600 * 2) {
      delay(500);
      yield();
      nowSecs = time(NULL);
    }

    print_time();
  }

  void print_time() {
    long int seconds;
    seconds = (long int) time(NULL);
    Serial.printf("Seconds since epoch: %ld\n", seconds);
  }

  void flush_to_influxdb() {
    connect_to_wifi();

    char payload[100];

    HTTPClient http;
    http.begin(Constants::influxdbUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Flush the first 10 runs separately (since we trigger on each iteration)
    if (loopCount < 10) {
      Datapoint datapoint = datapoints[loopCount % LOOPS_BEFORE_FLUSH];
      snprintf(payload, sizeof(payload), "esp32 value=%i %ld000000000",
        datapoint.value, datapoint.time);

      Serial.println(payload);

      int httpCode = http.POST(payload);
    } else {
      for (int i = 0; i < LOOPS_BEFORE_FLUSH; i++) {
        Datapoint datapoint = datapoints[i];
        snprintf(payload, sizeof(payload), "esp32 value=%i %ld000000000",
          datapoint.value, datapoint.time);
        Serial.println(payload);

        http.POST(payload);
      }
    }
  }
};
