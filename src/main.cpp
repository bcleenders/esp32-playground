#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "GameOfLife.h"
#include "HttpGet.h"
#include "LineArt.h"
#include "Module.h"
#include "WiFiScanner.h"
#include "WifiScannerLCD.h"

Module *program = new GameOfLife();

void setup() {
    program->run_main();
}

void loop() {
    program->run_loop();
}
