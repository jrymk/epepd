#include "EpGreyscaleDisplay.h"

const uint8_t EpGreyscaleDisplay::lut_GC4[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* BLACK     */ 0b00101010, 0b00000110, 0b00010101, 0, 0, 0, 0, 0, 0, 0,
        /* LIGHTGREY */ 0b00100000, 0b00000110, 0b00010000, 0, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
        /* DARK GREY */ 0b00101000, 0b00000110, 0b00010100, 0, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
        /* WHITE     */ 0b00010100, 0b00000110, 0b00101000, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 5, 12, 18, /* REPEAT */ 0,
        /* GROUP 2 */ 0, 0, 18, 18, /* REPEAT */ 1,
        /* GROUP 3 */ 0, 5, 12, 40, /* REPEAT */ 0,
        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
};

//// These are achieved by 2 display passes, the greyscales produced are not the most linear / uniform...
//// You can choose display mode GC16 for the best linearity and uniformity, but of course, at a cost of displaying 3 times,
//// which takes about... only 320ms? Then is there a reason not to use it? (Here's one, dark parts tends to fade white after updating the display)

//// This mode achieves 16 shades of grey in 2 update cycles. It can also "feel" faster, because the display almost reaches its final color in the first cycle

// Starting from grey and do both brightening and darkening produces better contrast across the board, compared to (1) darkening from white, and (2) brightening from black.
// The scale overall looks the best (most balanced) but not strictly increasing (e.g. 1011 < 1100 and 0011 < 0100 not guaranteed)
// Tuning these values don't make that much sense, as just a little temperature change can throw things off (by quite a lot!)
const uint8_t EpGreyscaleDisplay::lut_GC16_FAST_1[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* 00xx */ 0b00000110, 0b10000100, 0b01010100, 0, 0, 0, 0, 0, 0, 0,
        /* 10xx */ 0b00000110, 0b10000100, 0b00001000, 0, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
        /* 01xx */ 0b00000110, 0b10000100, 0b00000100, 0, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
        /* 11xx */ 0b00000110, 0b10000100, 0b00101000, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 0, 15, 15, /* REPEAT */ 1, // clean
        /* GROUP 2 */ 10, 1, 11, 1, /* REPEAT */ 0, // set mid-tone (darken) (increase the third number if too light)
        /* GROUP 3 */ 24, 5, 3, 1, /* REPEAT */ 0, // brighten/darken by bits 0 and 1
        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x77, 0x77, 0x77, 0x77, 0x77
};

const uint8_t EpGreyscaleDisplay::lut_GC16_FAST_2[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* xx00 */ 0b01010000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx10 */ 0b10000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx01 */ 0b01000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx11 */ 0b10100000, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 1, 1, 1, 0, /* REPEAT */ 0, // brighten/darken by bits 2 and 3
        /* GROUP 2 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 3 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
};

const uint8_t EpGreyscaleDisplay::lut_GC16_1[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* 00xx */ 0b10101010, 0b00001001, 0b00000001, 0b00000000, 0, 0, 0, 0, 0, 0,
        /* 10xx */ 0b10100000, 0b00001001, 0b00000001, 0b00101000, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
        /* 01xx */ 0b10101000, 0b00001001, 0b00000001, 0b00001000, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
        /* 11xx */ 0b10000000, 0b00001001, 0b00000001, 0b10101000, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 8, 8, 8, 40, /* REPEAT */ 0,
        /* GROUP 2 */ 0, 0, 18, 18, /* REPEAT */ 2, // clean
        /* GROUP 3 */ 0, 0, 2, 23, /* REPEAT */ 3, // set black
        /* GROUP 4 */ 16, 16, 16, 1, /* REPEAT */ 0, // brighten by bits 0 and 1
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
};

const uint8_t EpGreyscaleDisplay::lut_GC16_2[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* xx00 */ 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx10 */ 0b00101000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx01 */ 0b00001000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx11 */ 0b10101000, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 4, 4, 4, 1, /* REPEAT */ 0, // brighten by bits 2 and 3
        /* GROUP 2 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 3 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
};

// for image retention, high voltage (15V) has to be used. (I can adjust negative voltage to as low as -9V too, but the image fades away quickly...)
// the problem is that framerate of 1/8 is still too slow
const uint8_t EpGreyscaleDisplay::lut_GC16_3[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* xx00 */ 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx10 */ 0b00101000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx01 */ 0b00001000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* xx11 */ 0b10101000, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 1, 1, 1, 1, /* REPEAT */ 0, // brighten by bits 4 and 5
        /* GROUP 2 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 3 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
};

#define EPGREYSCALE_ENABLE_GC16_MAPPING 1

const uint8_t EpGreyscaleDisplay::lut_64_to_16[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 20, 24, 63 // -15V, from black
//        0, 1, 2, 3, 4, 5, 7, 9, 16, 19, 21, 23, 27, 31, 47, 63 // -10V, from black
};

EpGreyscaleDisplay::EpGreyscaleDisplay(Epepd &epepd) : EpFunction(epepd) {}

void EpGreyscaleDisplay::display(EpBitmap* source, EpPlacement &placement, EpGreyscaleDisplay::DisplayMode displayMode) {
    uint64_t start = esp_timer_get_time();
    uint64_t total = esp_timer_get_time();
    if (!source) {
        Serial.printf("[epepd] No source bitmap provided!\n");
        return;
    }

    uint8_t src0 = 0x00; // for red
    uint8_t src1 = 0x00; // for bw

    if (displayMode == GC4 || displayMode == GC16_FAST) {
        for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
            for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
                src0 |= (source->getPixel(placement.getSourcePos(x, y)) & 0x80) >> (x & 0b111);
                src1 |= ((source->getPixel(placement.getSourcePos(x, y)) & 0x40) << 1) >> (x & 0b111); // shifting a negative amount is undefined

                if ((x & 0b111) == 0b111) {
                    epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                    epepd->getBwRam()->_set8MonoPixels(x, y, src0);
                    src0 = 0x00;
                    src1 = 0x00;
                }
            }
        }
        Serial.printf("[epepd] EpGreyscaleDisplay (round 1) write ram took %lldus\n", esp_timer_get_time() - start);


        epepd->initDisplay();
        if (displayMode == GC4)
            epepd->writeLUT(lut_GC4);
        if (displayMode == GC16_FAST)
            epepd->writeLUT(lut_GC16_FAST_1);
        epepd->writeToDisplay();
        epepd->updateDisplay();

        if (displayMode == GC16_FAST) {
            /// round 2

            start = esp_timer_get_time();

            for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
                    src0 |= (source->getPixel(placement.getSourcePos(x, y)) & 0x20) << 2 >> (x & 0b111);
                    src1 |= ((source->getPixel(placement.getSourcePos(x, y)) & 0x10) << 3) >> (x & 0b111); // shifting a negative amount is undefined

                    if ((x & 0b111) == 0b111) {
                        epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                        epepd->getBwRam()->_set8MonoPixels(x, y, src0);
                        src0 = 0x00;
                        src1 = 0x00;
                    }
                }
            }
            Serial.printf("[epepd] EpGreyscaleDisplay (round 2) write ram took %lldus\n", esp_timer_get_time() - start);

            // anything that requires waiting for display BUSY goes last
            epepd->writeLUT(lut_GC16_FAST_2);
            epepd->writeToDisplay();
            epepd->updateDisplay();
        }
    }
    if (displayMode == GC16) {
        for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
            for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
#ifdef EPGREYSCALE_ENABLE_GC16_MAPPING
                uint8_t mapped = lut_64_to_16[source->getPixel(placement.getSourcePos(x, y)) >> 4] << 2; // 8 bit to 4 bit
#else
                uint8_t mapped = source->getPixel(placement.getSourcePos(x, y));
#endif
                src0 |= (mapped & 0x80) >> (x & 0b111);
                src1 |= ((mapped << 1) & 0x80) >> (x & 0b111);

                if ((x & 0b111) == 0b111) {
                    epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                    epepd->getBwRam()->_set8MonoPixels(x, y, src0);
                    src0 = 0x00;
                    src1 = 0x00;
                }
            }
        }
        Serial.printf("[epepd] EpGreyscaleDisplay (round 1) write ram took %lldus\n", esp_timer_get_time() - start);

        epepd->initDisplay();
        epepd->writeLUT(lut_GC16_1);
        epepd->writeToDisplay();
        epepd->updateDisplay();

        start = esp_timer_get_time();
        for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
            for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
#ifdef EPGREYSCALE_ENABLE_GC16_MAPPING
                uint8_t mapped = lut_64_to_16[source->getPixel(placement.getSourcePos(x, y)) >> 4] << 2; // 8 bit to 4 bit
#else
                uint8_t mapped = source->getPixel(placement.getSourcePos(x, y));
#endif
                src0 |= ((mapped << 2) & 0x80) >> (x & 0b111);
                src1 |= ((mapped << 3) & 0x80) >> (x & 0b111);

                if ((x & 0b111) == 0b111) {
                    epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                    epepd->getBwRam()->_set8MonoPixels(x, y, src0);
                    src0 = 0x00;
                    src1 = 0x00;
                }
            }
        }
        Serial.printf("[epepd] EpGreyscaleDisplay (round 2) write ram took %lldus\n", esp_timer_get_time() - start);

        epepd->writeLUT(lut_GC16_2);
        epepd->writeToDisplay();
        epepd->updateDisplay();

        start = esp_timer_get_time();
        for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
            for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
#ifdef EPGREYSCALE_ENABLE_GC16_MAPPING
                uint8_t mapped = lut_64_to_16[source->getPixel(placement.getSourcePos(x, y)) >> 4] << 2; // 8 bit to 4 bit
#else
                uint8_t mapped = source->getPixel(placement.getSourcePos(x, y));
#endif
                src0 |= ((mapped << 4) & 0x80) >> (x & 0b111);
                src1 |= ((mapped << 5) & 0x80) >> (x & 0b111);

                if ((x & 0b111) == 0b111) {
                    epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                    epepd->getBwRam()->_set8MonoPixels(x, y, src0);
                    src0 = 0x00;
                    src1 = 0x00;
                }
            }
        }
        Serial.printf("[epepd] EpGreyscaleDisplay (round 3) write ram took %lldus\n", esp_timer_get_time() - start);

        epepd->writeLUT(lut_GC16_3);
        epepd->writeToDisplay();
        epepd->updateDisplay();
    }

    Serial.printf("[epepd] EpGreyscaleDisplay in total took %lldus\n", esp_timer_get_time() - total);
//    epepd->powerOff();
}
