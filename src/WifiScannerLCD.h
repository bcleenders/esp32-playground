#include <Adafruit_GFX.h>  // Core graphics library
#include <WiFi.h>

#include "Arduino.h"
#include "Module.h"
#include "WROVER_KIT_LCD.h"

#ifndef LED_RED
#define LED_RED 0
#endif

#ifndef LED_BLUE
#define LED_BLUE 2
#endif

#ifndef LED_GREEN
#define LED_GREEN 4
#endif

#ifndef SEPARATOR
#define SEPARATOR "----------------------------------------\n"
#endif

#ifndef HEADER
#define HEADER "SSID                | signal - encrypted\n"
//     "    Windekind Wireless_2.4G - -83 dB - *"
#endif

WROVER_KIT_LCD tft;

// this only works in portrait mode (orientation=0 or 3)
uint16_t height = tft.height();  // (=320)
uint16_t width = tft.width();    // (=240)

// scroll control variables
uint16_t scrollTopFixedArea = 0;  // top fixed area

uint16_t yStart = scrollTopFixedArea;
uint16_t yArea = height - scrollTopFixedArea;
uint16_t w_tmp, h_tmp;
int16_t x1_tmp, y1_tmp;
int scrollPosY = scrollTopFixedArea;  // keeps track of the cursor position
int scrollPosX = -1;

int scrollText(const char *str) {
    Serial.print(str);

    if (scrollPosY == -1) {
        scrollPosY = tft.getCursorY();
    }
    scrollPosX = tft.getCursorX();
    if (scrollPosY >= height) {
        scrollPosY = (scrollPosY % height) + scrollTopFixedArea;
    }

    tft.getTextBounds(str, scrollPosX, scrollPosY, &x1_tmp, &y1_tmp, &w_tmp, &h_tmp);
    tft.fillRect(0, scrollPosY, width, h_tmp, WROVER_BLACK);

    tft.setCursor(scrollPosX, scrollPosY);

    // scroll lines
    scrollPosY = -1;
    for (int i = 0; i < h_tmp; i++) {
        yStart++;
        if (yStart == height)
            yStart = scrollTopFixedArea;
        tft.scrollTo(yStart);
        // delay(1);
    }

    tft.print(str);
    scrollPosY = tft.getCursorY();
    return h_tmp;
}

String output;

class WifiScannerLCD : public Module {
   public:
    void run_main() {
        Serial.begin(115200);

        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_GREEN, LOW);

        tft.begin();
        tft.setRotation(0);  // portrait mode is required

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        tft.fillRect(0, scrollPosY, width, height, WROVER_BLACK);
        tft.setTextColor(WROVER_GREENYELLOW);
    }

    void run_loop() {
        scrollText(SEPARATOR);
        scrollText("WiFi Scan start");
        int n = WiFi.scanNetworks();

        char line[41];  // 40 visible characters + a newline

        if (n == 0) {
            scrollText("no networks found\n");
        } else {
            snprintf(line, sizeof(line), "%i networks found\n", n);
            scrollText(line);

            for (int i = 0; i < n; ++i) {
                output = String(i + 1) + ":" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*") + "\n";

                String ssid = WiFi.SSID(i);
                if (ssid.length() > 23) {
                    ssid = ssid.substring(0, 20) + "...";
                }
                int rssi = WiFi.RSSI(i);
                String encryption;
                switch (WiFi.encryptionType(i)) {
                    case WIFI_AUTH_OPEN:
                        encryption = "open";
                        break;
                    case WIFI_AUTH_WEP:
                        encryption = "WEP";
                        break;
                    case WIFI_AUTH_WPA_PSK:
                        encryption = "WPA_PSK";
                        break;
                    case WIFI_AUTH_WPA2_PSK:
                        encryption = "WPA2_PSK";
                        break;
                    case WIFI_AUTH_WPA_WPA2_PSK:
                        encryption = "WPA_WPA2_PSK";
                        break;
                    case WIFI_AUTH_WPA2_ENTERPRISE:
                        encryption = "WPA2_ENTERPRISE";
                        break;
                    default:
                        encryption = "N/A";
                }
                snprintf(line, sizeof(line), "%23s - %3i dB - %1s\n", ssid.c_str(), rssi, ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*"));

                scrollText(line);
                delay(5);
            }
        }

        delay(5000);
    }
};
