#include <Adafruit_GFX.h>  // Core graphics library
#include <string.h>

#include "Arduino.h"
#include "Constants.h"
#include "Module.h"
#include "WROVER_KIT_LCD.h"
#include "WiFi.h"

#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <lwip/sockets.h>

#define PORT 9001

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

        char line[100];

        snprintf(line, sizeof line, "Connecting to '%s'...\n", Constants::ssid);
        scrollText(line);

        const char *ip = Telnet::ConnectToWifi();

        snprintf(line, sizeof line, "Connected. IP is: %s\n", ip);
        scrollText(line);

        xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
    }

    void run_loop() {
        delay(100);
    }

    static void tcp_server_task(void *pvParameters) {
        int addr_family = AF_INET;
        char addr_str[128];
        struct sockaddr_in6 dest_addr;
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;

        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);

        int ip_protocol = IPPROTO_IP;

        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) {
            Serial.printf("Unable to create socket: errno %d\n", errno);
            vTaskDelete(NULL);
            return;
        }

        Serial.println("Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            Serial.printf("Socket unable to bind: errno %d\n", errno);
            Serial.printf("IPPROTO: %d\n", addr_family);
            goto CLEAN_UP;
        }
        Serial.printf("Socket bound, port %d\n", PORT);

        err = listen(listen_sock, 1);
        if (err != 0) {
            Serial.printf("Error occurred during listen: errno %d\n", errno);
            goto CLEAN_UP;
        }

        while (1) {
            Serial.printf("Socket listening\n");

            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            uint addr_len = sizeof(source_addr);
            int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
            if (sock < 0) {
                Serial.printf("Unable to accept connection: errno %d\n", errno);
                break;
            }

            // Convert ip address to string
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

            Serial.printf("Socket accepted ip address: %s\n", addr_str);

            //do_retransmit(sock);
            int len;
            char rx_buffer[128];

            do {
                len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                if (len < 0) {
                    Serial.printf("Error occurred during receiving: errno %d\n", errno);
                } else if (len == 0) {
                    Serial.printf("Connection closed\n");
                } else {
                    rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
                    Serial.printf("Received %d bytes: %s\n", len, rx_buffer);

                    // send() can return less bytes than supplied length.
                    // Walk-around for robust implementation.
                    int to_write = len;
                    while (to_write > 0) {
                        int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                        if (written < 0) {
                            Serial.printf("Error occurred during sending: errno %d\n", errno);
                        }
                        to_write -= written;
                    }
                }
            } while (len > 0);

            shutdown(sock, 0);
            close(sock);
        }

        // TODO now do something with the socker


CLEAN_UP: // label for jumping to
        close(listen_sock);
        vTaskDelete(NULL);

    }

   private:
    static const char *ConnectToWifi() {
        WiFi.begin(Constants::ssid, Constants::password);

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }

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
