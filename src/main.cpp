#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "Module.h"
#include "WiFiScanner.h"

Module *program = new WifiScanner();

void setup()
{
  program->run_main();
}

void loop()
{
  program->run_loop();
}
