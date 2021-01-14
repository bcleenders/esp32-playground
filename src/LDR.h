#include <Arduino.h>

#include "Module.h"

class LDR: public Module {
public:
  void run_main() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
  }

  void run_loop() {
    lightVal = analogRead(analogPin); // read the current light levels
    Serial.printf("lightVal = %i\n", lightVal);
    sleep(1);
  }

private:
    const int analogPin = 34;

    int lightInit;  // initial value
    int lightVal;   // light reading
};
