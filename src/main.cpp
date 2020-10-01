#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "Module.h"
#include "WiFiScanner.h"
#include "HttpGet.h"

Module *program = new HttpGet();

void setup()
{
  program->run_main();
}

void loop()
{
  program->run_loop();
}
