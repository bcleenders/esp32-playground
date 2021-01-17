//#include <Arduino.h>

#include <Adafruit_GFX.h>  // Core graphics library

#include "Adafruit_EPD.h"
#include "Adafruit_ThinkInk.h"
#include "Module.h"

// Pin outs for ESP32 featherwing
#define SRAM_CS 32
#define EPD_CS 15
#define EPD_DC 33
#define EPD_RESET -1  // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY -1   // can set to -1 to not use a pin (will wait a fixed delay)

// 2.9" Grayscale Featherwing or Breakout:
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);


// Runs on the huzzah esp32, so run with:
// pio run -e featheresp32 --target upload --target monitor
class EInk : public Module {
   public:
    void run_main() {
        Serial.begin(115200);
        while (!Serial) {
            delay(10);
        }

        // alternate modes!
        display.begin(THINKINK_GRAYSCALE4);

        display.setTextSize(1);
        display.setTextWrap(true);

        // Top half is light, bottom half is dark
        display.fillRect(0, 0, display.width(), display.height() / 2, EPD_WHITE);
        display.fillRect(0, display.height() / 2, display.width(), display.height() / 2, EPD_BLACK);

        display.setTextColor(EPD_BLACK); // EPD_WHITE also available
        display.setCursor(0, 0);
        display.print("Hello there, reader.");

        display.setCursor(0, display.height() / 4);
        display.setTextColor(EPD_DARK);
        display.print("I am an e-ink screen. Read me!\n\n");

        display.setCursor(0, 2 * display.height() / 4);
        display.setTextColor(EPD_LIGHT);
        display.print("This is the second message. It is even more awesome, and you should DEFINITELY read it.\n\n");

        display.setCursor(0, 3 * display.height() / 4);
        display.setTextColor(EPD_WHITE);
        display.print("Well, this was fun.");
        display.display();
    }

    void run_loop() {
        delay(10000);
    }
};
