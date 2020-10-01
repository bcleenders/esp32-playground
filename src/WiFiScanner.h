#include <Arduino.h>
#include <WiFi.h>

#include "Module.h"

int ledPin = 2;

class WifiScanner : public Module
{
public:
    void run_main()
    {
        Serial.begin(115200);

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        Serial.println("Setup done");

        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);
    }

    void run_loop()
    {
        Serial.println("Starting scan");

        int n = WiFi.scanNetworks();
        Serial.println("Scan done!");

        char line[64];
        snprintf(line, sizeof(line), "%3i networks found.", n);
        Serial.println(line);

        Serial.println("------------------------------------------------------");
        Serial.println("| SSID                    | signal | encryption      |");
        Serial.println(" ----------------------------------------------------");

        for (int i = 0; i < n; i++)
        {
            String ssid = WiFi.SSID(i);
            if (ssid.length() > 23)
            {
                ssid = ssid.substring(0, 20) + "...";
            }

            int rssi = WiFi.RSSI(i);
            String encryption;

            switch (WiFi.encryptionType(i))
            {
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

            // String
            snprintf(line, sizeof(line), "| %23s | %3i dB | %15s |", ssid.c_str(), rssi, encryption.c_str());
            Serial.println(line);
        }
        Serial.println("------------------------------------------------------");
        Serial.println("");

        digitalWrite(ledPin, HIGH);
        delay(250);
        digitalWrite(ledPin, LOW);
        delay(4750);
    }
};
