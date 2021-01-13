#include <Arduino.h>

#include "Module.h"

class Blink: public Module
{
public:
  void run_main() {
        Serial.begin(115200);
        pinMode (ledPin, OUTPUT);
  }

  void run_loop() {
    sleep(1);
    digitalWrite(ledPin, LOW);
    Serial.println("OFF");
    sleep(1);
    digitalWrite(ledPin, HIGH);
    Serial.println("ON");
  }

private:
  int ledPin = 5;
};
