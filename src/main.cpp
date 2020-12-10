#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "HttpGet.h"
#include "Module.h"
#include "WiFiScanner.h"
#include "WifiScannerLCD.h"

Module *program = new WifiScannerLCD();

void setup() {
    program->run_main();
}

void loop() {
    program->run_loop();
}
