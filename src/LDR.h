#include <Arduino.h>

#include "Module.h"
#include "Constants.h"

#define uSEC_TO_SEC 1000000
#define INTERVAL_SEC 4

#define LOOPS_BEFORE_SYNC 10
#define LOOPS_BEFORE_TIME_SYNC 1000

typedef struct {
  int time;
  int value;
} Datapoint;

// Need to store outside of class, because classes are not stored in RTC memory
RTC_DATA_ATTR int recordCounter = 0;
RTC_DATA_ATTR Datapoint datapoints[LOOPS_BEFORE_SYNC];

RTC_DATA_ATTR int ntpLoopCount = 0;

class LDR: public Module {
public:
  void run_main() {
    Serial.begin(115200);

    if (ntpLoopCount == 0) {
      Serial.println("Should sync NTP now");
      connect_to_wifi();
      //TODO sync with NTP
      //https://github.com/espressif/esp-idf/blob/2bfdd036b2dbd07004c8e4f2ffc87c728819b737/examples/protocols/sntp/main/sntp_example_main.c
    }
    ntpLoopCount = (ntpLoopCount + 1) % LOOPS_BEFORE_TIME_SYNC;

    datapoints[recordCounter].time = 1;
    datapoints[recordCounter].value = analogRead(analogPin);

    Serial.println(datapoints[recordCounter].value);

    recordCounter++;
    if (recordCounter == LOOPS_BEFORE_SYNC) {
      Serial.println("Flush");
      connect_to_wifi();
      //TODO sync to influxdb
      recordCounter = 0;
    }

    // Go to sleep
    esp_sleep_enable_timer_wakeup(INTERVAL_SEC * uSEC_TO_SEC);
    esp_deep_sleep_start();
  }

  void run_loop() {
  }

private:
    const int analogPin = 34;

    int lightInit;  // initial value
    int lightVal;   // light reading

  void connect_to_wifi() {
        WiFi.begin(Constants::ssid, Constants::password);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

        Serial.print("Connected. My IP address is: ");
        Serial.println(WiFi.localIP());
  }
};
