#include <Adafruit_GFX.h>

#include "Module.h"
#include "WROVER_KIT_LCD.h"

#define STEPS_PER_SIDE 40
#define TOTAL_STEPS 160  // 4 * STEPS_PER_SIDE

class LineArt : public Module {
   private:
    WROVER_KIT_LCD tft;
    // this only works in portrait mode (orientation=0 or 3)
    uint16_t height = tft.height();  // (=320)
    uint16_t width = tft.width();    // (=240)

    uint16_t counter = 0;

    uint16_t toX1(uint16_t iteration) {
        iteration = iteration % TOTAL_STEPS;
        uint16_t steps_into_side = iteration % STEPS_PER_SIDE;
        if (0 <= iteration && iteration < STEPS_PER_SIDE) {
            return 0;
        } else if (STEPS_PER_SIDE <= iteration && iteration < 2 * STEPS_PER_SIDE) {
            return steps_into_side * width / STEPS_PER_SIDE;
        } else if (2 * STEPS_PER_SIDE <= iteration && iteration < 3 * STEPS_PER_SIDE) {
            return width;
        } else {  // 3 * stepsPerSize <= iteration && iteration < 4 * STEPS_PER_SIDE
            return width - (steps_into_side * width / STEPS_PER_SIDE);
        };
    }

    uint16_t toY1(uint16_t iteration) {
        iteration = iteration % TOTAL_STEPS;
        uint16_t steps_into_side = iteration % STEPS_PER_SIDE;

        if (0 <= iteration && iteration < STEPS_PER_SIDE) {
            return steps_into_side * height / STEPS_PER_SIDE;
        } else if (STEPS_PER_SIDE <= iteration && iteration < 2 * STEPS_PER_SIDE) {
            return height;
        } else if (2 * STEPS_PER_SIDE <= iteration && iteration < 3 * STEPS_PER_SIDE) {
            return height - (steps_into_side * height / STEPS_PER_SIDE);
        } else {  // 3 * stepsPerSize <= iteration && iteration < 4 * STEPS_PER_SIDE
            return 0;
        };
    }

    uint16_t toX2(uint16_t iteration) {
        return toX1(iteration + STEPS_PER_SIDE);
    }

    uint16_t toY2(uint16_t iteration) {
        return toY1(iteration + STEPS_PER_SIDE);
    }

   public:
    void run_main() {
        Serial.begin(115200);

        delay(10);

        tft.begin();
        tft.setRotation(0);

        tft.fillRect(0, 0, width, height, WROVER_BLACK);

        Serial.printf("width=%i, height=%i", width, height);
    }

    void run_loop() {
        counter = (counter + 1) % TOTAL_STEPS;

        tft.fillRect(0, 0, width, height, WROVER_BLACK);
        delay(1);
        for (int i = 0; i < 35; i++) {
            uint16_t x_1 = toX1(counter + i);
            uint16_t y_1 = toY1(counter + i);

            uint16_t x_2 = toX2(counter + i);
            uint16_t y_2 = toY2(counter + i);

            tft.drawLine(x_1, y_1, x_2, y_2, WROVER_NAVY);
        }

        delay(200);
    }
};
