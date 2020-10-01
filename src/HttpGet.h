#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "Module.h"
#include "Constants.h"

class HttpGet : public Module
{
public:
    void run_main()
    {
        Serial.begin(115200);

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        delay(3000);

        HttpGet::ConnectToWifi();
    }

    void run_loop()
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            HttpGet::ConnectToWifi();
        }

        HTTPClient client;

        Serial.printf("http GET'ing %s\n", Constants::url);

        client.begin(Constants::url);

        Serial.print("[HTTP] GET...\n");

        // httpCode will be negative on error (e.g. timeout/connection lost)
        // if >0, the code will be the HTTP statuscode
        int httpCode = client.GET();

        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {
                String payload = client.getString();
                Serial.println(payload);
            }
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", client.errorToString(httpCode).c_str());
        }

        client.end();

        // Only run once per 5 seconds
        delay(5000);
    }

private:
    static void ConnectToWifi()
    {
        WiFi.begin(Constants::ssid, Constants::password);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

        Serial.print("Connected. My IP address is: ");
        Serial.println(WiFi.localIP());
    }
};
