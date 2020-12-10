#include <Adafruit_GFX.h>

#include "Module.h"
#include "WROVER_KIT_LCD.h"

class GameOfLife : public Module {
   private:
    WROVER_KIT_LCD tft;
    // this only works in portrait mode (orientation=0 or 3)
    uint16_t height = tft.height();  // (=320)
    uint16_t width = tft.width();    // (=240)

    uint32_t currCells[240][10];
    uint32_t nextCells[240][10];

    /**
     * Get the value of the cell at position (x,y).
     * Corrects overflow/underflow of positions, so (-1,-1) is a valid value
     * (and refers to the corner furthest away from (0,0)).
     */
    boolean cell(int x, int y) {
        x = (x + width) % width;
        y = (y + height) % height;

        uint16_t yPos_big = y / 32;    // bits per uint32_t field
        uint16_t yPos_small = y % 32;  // bit position within the uint32
        uint32_t bitField = currCells[x][yPos_big];

        return (bitField >> yPos_small) & 1;
    }

    boolean alive(uint16_t x, uint16_t y) {
        uint16_t aliveNeighbours = 0;

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (!(dx == 0 && dy == 0)) {  // Skip the cell itself
                    if (cell(x + dx, y + dy)) {
                        // Serial.printf("(%u;%u)'s neighbour (%u;%u) is alive'", x, y, x + dx, y + dy);
                        aliveNeighbours++;
                    }
                }
            }
        }

        /* 
         * Rules:
         * - Any live cell with two or three live neighbours survives.
         * - Any dead cell with three live neighbours becomes a live cell.
         * - All other live cells die in the next generation. Similarly, all other dead cells stay dead.
         */
        switch (aliveNeighbours) {
            case 0 ... 1:  // Dies - loneliness
                return false;
            case 2:  // Survives or remains dead
                return cell(x, y);
            case 3:  // Becomes alive
                return true;
            case 4 ... 8:  // Dies - overcrowded
                return false;
            default:  // Guard for invalid state
                Serial.printf("Invalid state: %u alive neighbours\n", aliveNeighbours);
                exit(1);
        }
    }

    /**
     * Use the current values of `currCells` to calculate the values of
     * the next iteration, and write them to `nextCells`.
     */
    void calcNext() {
        for (uint16_t x = 0; x < width; x++) {
            for (uint16_t y_big = 0; y_big < 10; y_big++) {
                uint32_t bitSet = 0;

                for (uint16_t y_small = 0; y_small < 32; y_small++) {
                    // Check if field should become alive
                    if (alive(x, y_big * 32 + y_small)) {
                        // Mark bit as 1 to signal aliveness
                        bitSet |= (1UL << y_small);
                    }
                }

                // We've computed all bits in the bitset, now write it
                nextCells[x][y_big] = bitSet;
            }
        }
    }

    /**
     * Draws each cell from 'next' on the screen, and moves it into cells.
     */
    void drawAndMove() {
        for (int x = 0; x < width; x++) {
            for (int y_big = 0; y_big < 10; y_big++) {
                for (int y_small = 0; y_small < 32; y_small++) {
                    boolean oldVal = (currCells[x][y_big] >> y_small) & 1UL;
                    boolean newVal = (nextCells[x][y_big] >> y_small) & 1UL;

                    // Serial.printf("processing %u %u %u\n", x, y_big, y_small);

                    if (oldVal != newVal) {  // Only redraw if the value changed (optimization)
                        if (newVal) {
                            tft.drawPixel(x, y_big * 32 + y_small, WROVER_GREEN);
                        } else {
                            tft.drawPixel(x, y_big * 32 + y_small, WROVER_BLACK);
                        }
                    }
                }
                // Move field into old cells
                currCells[x][y_big] = nextCells[x][y_big];
            }
        }
    }

    void reset() {
        tft.fillRect(0, 0, width, height, WROVER_BLACK);

        tft.setTextColor(WROVER_GREEN);
        tft.setRotation(1);
        // Regular font is 5x8 pixels - make it bigger
        tft.setTextSize(2);
        tft.setCursor(30, 112);
        tft.print("Starting Game Of Life");
        tft.setRotation(0);

        delay(2000);

        tft.fillRect(0, 0, width, height, WROVER_BLACK);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                nextCells[x][y] = esp_random();
            }
        }

        drawAndMove();

        // A slightly longer delay at the start
        delay(2000);
    }

    boolean isRepeating() {
        return false;
    }

   public:
    void run_main() {
        Serial.begin(115200);

        delay(10);

        tft.begin();
        tft.setRotation(0);

        reset();
    }

    void run_loop() {
        calcNext();
        drawAndMove();

        if (isRepeating()) {
            reset();
        }

        delay(50);
    }
};
