#include <Arduino.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

#include "Module.h"
#include "Constants.h"

#define uSEC_TO_SEC 1000000
#define INTERVAL_SEC 30

#define LOOPS_BEFORE_FLUSH 5
#define LOOPS_BEFORE_TIME_SYNC 20

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

    // Flush the first N iterations directly, to quickly get some feedback into Grafana
    if (loopCount < LOOPS_BEFORE_FLUSH || loopCount % LOOPS_BEFORE_FLUSH == 0) {
      flush_to_influxdb();
    }

    loopCount++;
    // Go to sleep
    esp_sleep_enable_timer_wakeup(INTERVAL_SEC * uSEC_TO_SEC);
    esp_deep_sleep_start();

    if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect(true);
    }
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

    long int seconds;
    seconds = (long int) time(NULL);
    Serial.printf("NTP synced. Epoch time: %ld\n", seconds);
  }

  HTTPClient http;
  void flush_to_influxdb() {
    connect_to_wifi();

    int attempt = 0;
    while (attempt < 5 && ! http.begin(Constants::influxdbUrl)) {
      delay(50);
      attempt++;
    }

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Flush the first N+1 loops directly (+1 so we don't report the Nth one twice)
    if (loopCount <= LOOPS_BEFORE_FLUSH) {
      Datapoint datapoint = datapoints[loopCount % LOOPS_BEFORE_FLUSH];
      send_metric(http, datapoint);
    } else {
      // Then only run once per N loops
      for (int i = 0; i < LOOPS_BEFORE_FLUSH; i++) {
        send_metric(http, datapoints[i]);
      }
    }

    http.end();
  }

  int send_metric(HTTPClient& http, Datapoint& datapoint) {
    char payload[100];

    snprintf(payload, sizeof(payload), "esp32 value=%i %ld000000000",
      datapoint.value, datapoint.time);
    Serial.println(payload);

    int attempt = 0;
    int code = -1; // http return code
    while ((code < 200 || code >= 300) && attempt < 5) {
      delay(50 * attempt); // Don't wait on first run, wait a bit longer on every retry.
      code = http.POST(payload);
      attempt++;
    }

    // Just return the last one if it kept failing...
    return code;
  }
};
