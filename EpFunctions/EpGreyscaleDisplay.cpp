#include "EpGreyscaleDisplay.h"

const uint8_t EpGreyscaleDisplay::lut_GC4[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* BLACK     */ 0b00101010, 0b00000110, 0b00010101, 0, 0, 0, 0, 0, 0, 0,
        /* LIGHTGREY */ 0b00100000, 0b00000110, 0b00010000, 0, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
        /* DARK GREY */ 0b00101000, 0b00000110, 0b00010100, 0, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
        /* WHITE     */ 0b00010100, 0b00000110, 0b00101000, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 8, 8, 40, /* REPEAT */ 0,
        /* GROUP 2 */ 0, 0, 32, 32, /* REPEAT */ 2,
        /* GROUP 3 */ 0, 8, 8, 40, /* REPEAT */ 0,
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
//// You can choose display mode GC16_3PASS for the best linearity and uniformity, but of course, at a cost of displaying 3 times, which can take about a whole second
//// With 3 passes, you can then display 64 shades of grey, but that again, will suffer from linearity problems

//// or if you want to do in 2 passes, try which one you like the most:

//// OPTION 1 / 3
//// Bad contrast at the bright end, best for dark image details. Greyscale strictly increasing
//const uint8_t EpGreyscaleDisplay::lut_GC16_1[] PROGMEM = {
//        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
//        /* 00xx */ 0b10101010, 0b00001001, 0b00000001, 0b00000000, 0, 0, 0, 0, 0, 0,
//        /* 10xx */ 0b10100000, 0b00001001, 0b00000001, 0b00101000, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
//        /* 01xx */ 0b10101000, 0b00001001, 0b00000001, 0b00001000, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
//        /* 11xx */ 0b10000000, 0b00001001, 0b00000001, 0b10101000, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* GROUP 1 */ 8, 8, 8, 40, /* REPEAT */ 0,
//        /* GROUP 2 */ 0, 0, 32, 32, /* REPEAT */ 3, // clean
//        /* GROUP 3 */ 0, 0, 0, 55, /* REPEAT */ 0, // set black
//        /* GROUP 4 */ 4, 4, 4, 1, /* REPEAT */ 0, // brighten by bits 0 and 1
//        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* FRAMERATE */ 0x77, 0x77, 0x77, 0x77, 0x77
//};
//
//const uint8_t EpGreyscaleDisplay::lut_GC16_2[] PROGMEM = {
//        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
//        /* xx00 */ 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* xx10 */ 0b10100000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* xx01 */ 0b10000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* xx11 */ 0b10101000, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* GROUP 1 */ 1, 1, 1, 1, /* REPEAT */ 0, // brighten by bits 2 and 3
//        /* GROUP 2 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 3 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
//};

//// OPTION 2 / 3
//// Bad contrast at the dark end, best for bright image details. Greyscale strictly increasing
//const uint8_t EpGreyscaleDisplay::lut_GC16_1[] PROGMEM = {
//        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
//        /* 00xx */ 0b10101010, 0b00000110, 0b00000010, 0b01010100, 0, 0, 0, 0, 0, 0,
//        /* 10xx */ 0b10100000, 0b00000110, 0b00000010, 0b00000100, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
//        /* 01xx */ 0b10101000, 0b00000110, 0b00000010, 0b00010100, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
//        /* 11xx */ 0b10000000, 0b00000110, 0b00000010, 0b00000000, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* GROUP 1 */ 8, 8, 8, 40, /* REPEAT */ 0,
//        /* GROUP 2 */ 0, 0, 32, 32, /* REPEAT */ 3, // clean
//        /* GROUP 3 */ 0, 0, 0, 55, /* REPEAT */ 0, // set white
//        /* GROUP 4 */ 8, 5, 4, 1, /* REPEAT */ 0, // darken by bits 0 and 1
//        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* FRAMERATE */ 0x77, 0x77, 0x77, 0x77, 0x77
//};
//
//const uint8_t EpGreyscaleDisplay::lut_GC16_2[] PROGMEM = {
//        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
//        /* xx00 */ 0b01010100, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* xx10 */ 0b00000100, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* xx01 */ 0b00010100, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* xx11 */ 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        /* GROUP 1 */ 1, 1, 1, 1, /* REPEAT */ 0, // darken by bits 2 and 3
//        /* GROUP 2 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 3 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
//        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
//};

//// OPTION 3 / 3
//// Better contrast across the board. The scale overall looks the best (most balanced) but not strictly increasing (e.g. 1011 < 1100 and 0011 < 0100 not guaranteed)
//// Tuning these values don't make that much sense, as just a little temperature change can throw things off (by quite a lot!)
const uint8_t EpGreyscaleDisplay::lut_GC16_1[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* 00xx */ 0b10101010, 0b00000110, 0b00000100, 0b01010100, 0, 0, 0, 0, 0, 0,
        /* 10xx */ 0b10100000, 0b00000110, 0b00000100, 0b00001000, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
        /* 01xx */ 0b10101000, 0b00000110, 0b00000100, 0b00000100, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
        /* 11xx */ 0b10000000, 0b00000110, 0b00000100, 0b00101000, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 8, 8, 8, 40, /* REPEAT */ 0,
        /* GROUP 2 */ 0, 0, 32, 32, /* REPEAT */ 3, // clean
        /* GROUP 3 */ 0, 0, 25, 1, /* REPEAT */ 0, // set mid-tone (darken) (increase the third number if too light)
        /* GROUP 4 */ 20, 8, 4, 1, /* REPEAT */ 0, // brighten/darken by bits 0 and 1
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x77, 0x77, 0x77, 0x77, 0x77
};

const uint8_t EpGreyscaleDisplay::lut_GC16_2[] PROGMEM = {
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
//// END OF GC16


EpGreyscaleDisplay::EpGreyscaleDisplay(Epepd &epepd) : EpFunction(epepd) {}

void EpGreyscaleDisplay::display(EpBitmap* source, EpGreyscaleDisplay::DisplayMode displayMode) {
    uint64_t start = esp_timer_get_time();
    uint8_t src0 = 0x00; // for red
    uint8_t src1 = 0x00; // for bw

    for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
        for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
            src0 |= (source->getPixel(x, y) & 0x80) >> (x & 0b111);
            src1 |= ((source->getPixel(x, y) & 0x40) << 1) >> (x & 0b111); // shifting a negative amount is undefined

            if ((x & 0b111) == 0b111) {
                epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                epepd->getBwRam()->_set8MonoPixels(x, y, src0); // (keeping bw the display color. bw will be bit 0 of src)
                src0 = 0x00;
                src1 = 0x00;
            }
        }
    }
    Serial.printf("[epepd] EpGreyscaleDisplay (round 1) write ram took %lldus\n", esp_timer_get_time() - start);


    epepd->initDisplay();
    if (displayMode == GC4)
        epepd->writeLUT(lut_GC4);
    if (displayMode == GC16)
        epepd->writeLUT(lut_GC16_1);
    epepd->writeToDisplay();
    epepd->updateDisplay();

    if (displayMode == GC16) {
        /// round 2

        start = esp_timer_get_time();

        for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
            for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
                src0 |= (source->getPixel(x, y) & 0x20) << 2 >> (x & 0b111);
                src1 |= ((source->getPixel(x, y) & 0x10) << 3) >> (x & 0b111); // shifting a negative amount is undefined

                if ((x & 0b111) == 0b111) {
                    epepd->getRedRam()->_set8MonoPixels(x, y, src1);
                    epepd->getBwRam()->_set8MonoPixels(x, y, src0); // (keeping bw the display color. bw will be bit 0 of src)
                    src0 = 0x00;
                    src1 = 0x00;
                }
            }
        }
        Serial.printf("[epepd] EpGreyscaleDisplay (round 2) write ram took %lldus\n", esp_timer_get_time() - start);

        // anything that requires waiting for display BUSY goes last
        epepd->writeLUT(lut_GC16_2);
        epepd->writeToDisplay();
        epepd->updateDisplay();
    }

//    epepd->powerOff();
}
