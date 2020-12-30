#include "WROVER_KIT_LCD.h"
#include <Adafruit_GFX.h>  // Core graphics library
#include "Arduino.h"
#include <WString.h>

#include "Module.h"
#include "WiFi.h"
#include "Constants.h"

class Telnet : public Module {
  public:
    void run_main() {
        Serial.begin(115200);

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        tft.begin();
        tft.setRotation(0);  // portrait mode is required
        tft.fillRect(0, scrollPosY, width, height, WROVER_BLACK);
        tft.setTextColor(WROVER_GREENYELLOW);
        scrollText("Starting...\n");
    }

    void run_loop() {
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Connecting to Wifi");
          const char *ip = Telnet::ConnectToWifi();
          char line[100];
          snprintf(line, sizeof line, "Connected. IP is: %s\n", ip);
          scrollText(line);
        }

        delay(100);
    }

   private:
    static const char* ConnectToWifi() {
        WiFi.begin(Constants::ssid, Constants::password);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

        Serial.print("Connected. My IP address is: ");
        Serial.println(WiFi.localIP());

        return WiFi.localIP().toString().c_str();
    }

    WROVER_KIT_LCD tft;
    // this only works in portrait mode (orientation=0 or 3)
    uint16_t height = tft.height();  // (=320)
    uint16_t width = tft.width();    // (=240)

    // scroll control variables
    uint16_t scrollTopFixedArea = 0;  // top fixed area

    uint16_t yStart = scrollTopFixedArea;
    uint16_t yArea = height - scrollTopFixedArea;
    uint16_t w_tmp, h_tmp;                // currently unused
    int16_t x1_tmp, y1_tmp;               // currently unused
    int scrollPosY = scrollTopFixedArea;  // keeps track of the cursor position
    int scrollPosX = -1;

    String output = "";

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
            delay(5);
        }

        tft.print(str);
        scrollPosY = tft.getCursorY();
        return h_tmp;
    }
};
