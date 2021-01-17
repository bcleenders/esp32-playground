#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "Blink.h"
#include "EInk.h"
#include "GameOfLife.h"
#include "HttpGet.h"
#include "HumiditySensor.h"
#include "LineArt.h"
#include "Module.h"
#include "Telnet.h"
#include "WiFiScanner.h"
#include "WifiScannerLCD.h"

Module *program = new EInk();

void setup() {
    program->run_main();
}

void loop() {
    program->run_loop();
}
