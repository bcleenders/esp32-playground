#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "Blink.h"
#include "LDR.h"
#include "HttpGet.h"
#include "GameOfLife.h"
#include "LineArt.h"
#include "Module.h"
#include "Telnet.h"
#include "WiFiScanner.h"
#include "WifiScannerLCD.h"

Module *program = new LDR();

void setup() {
    program->run_main();
}

void loop() {
    program->run_loop();
}
