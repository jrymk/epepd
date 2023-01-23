#include "EpPartialDisplay.h"

const uint8_t EpPartialDisplay::lut_FIX[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* REFRESH BLACK */ 0b00001001, 0b00001001, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO WHITE      */ 0b00001001, 0b00001001, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO BLACK      */ 0b00001001, 0b00001001, 0, 0, 0, 0, 0, 0, 0, 0,
        /* REFRESH WHITE */ 0b00001001, 0b00001001, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 0, 40, 40, /* REPEAT */ 9,
        /* GROUP 2 */ 0, 0, 12, 12, /* REPEAT */ 9,
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

const uint8_t EpPartialDisplay::lut_GC2[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* REFRESH BLACK */ 0b10101000, 0b00010100, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO WHITE      */ 0b00010100, 0b10101000, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO BLACK      */ 0b10101000, 0b01010100, 0, 0, 0, 0, 0, 0, 0, 0,
        /* REFRESH WHITE */ 0b00010100, 0b00101000, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 4, 6, 20, 4, /* REPEAT */ 0,  // 0 2 3 10
        /* GROUP 2 */ 12, 20, 10, 1, /* REPEAT */ 0, // 2 6 10 5
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

const unsigned char EpPartialDisplay::lut_GC2_PARTIAL_1[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* KEEP BLACK */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO WHITE   */ lut_GC2[10], lut_GC2[11], lut_GC2[12], lut_GC2[13], lut_GC2[14], lut_GC2[15], lut_GC2[16], lut_GC2[17], lut_GC2[18], lut_GC2[19],
        /* TO BLACK   */ lut_GC2[20], lut_GC2[21], lut_GC2[22], lut_GC2[23], lut_GC2[24], lut_GC2[25], lut_GC2[26], lut_GC2[27], lut_GC2[28], lut_GC2[29],
        /* KEEP WHITE */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ lut_GC2[50], lut_GC2[51], lut_GC2[52], lut_GC2[53], /* REPEAT */ lut_GC2[54],
        /* GROUP 2 */ lut_GC2[55], lut_GC2[56], lut_GC2[57], lut_GC2[58], /* REPEAT */ lut_GC2[59],
        /* GROUP 3 */ lut_GC2[60], lut_GC2[61], lut_GC2[62], lut_GC2[63], /* REPEAT */ lut_GC2[64],
        /* GROUP 4 */ lut_GC2[65], lut_GC2[66], lut_GC2[67], lut_GC2[68], /* REPEAT */ lut_GC2[69],
        /* GROUP 5 */ lut_GC2[70], lut_GC2[71], lut_GC2[72], lut_GC2[73], /* REPEAT */ lut_GC2[74],
        /* GROUP 6 */ lut_GC2[75], lut_GC2[76], lut_GC2[77], lut_GC2[78], /* REPEAT */ lut_GC2[79],
        /* GROUP 7 */ lut_GC2[80], lut_GC2[81], lut_GC2[82], lut_GC2[83], /* REPEAT */ lut_GC2[84],
        /* GROUP 8 */ lut_GC2[85], lut_GC2[86], lut_GC2[87], lut_GC2[88], /* REPEAT */ lut_GC2[89],
        /* GROUP 9 */ lut_GC2[90], lut_GC2[91], lut_GC2[92], lut_GC2[93], /* REPEAT */ lut_GC2[94],
        /* GROUP10 */ lut_GC2[95], lut_GC2[96], lut_GC2[97], lut_GC2[98], /* REPEAT */ lut_GC2[99],
        /* FRAMERATE */ lut_GC2[100], lut_GC2[101], lut_GC2[102], lut_GC2[103], lut_GC2[104]
};

// bwRam always end up reflecting the final color (except greyscale modes)
const unsigned char EpPartialDisplay::lut_GC2_PARTIAL_2[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* KEEP BLACK    */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* REFRESH WHITE */ lut_GC2[30], lut_GC2[31], lut_GC2[32], lut_GC2[33], lut_GC2[34], lut_GC2[35], lut_GC2[36], lut_GC2[37], lut_GC2[38], lut_GC2[39],
        /* REFRESH BLACK */ lut_GC2[0], lut_GC2[1], lut_GC2[2], lut_GC2[3], lut_GC2[4], lut_GC2[5], lut_GC2[6], lut_GC2[7], lut_GC2[8], lut_GC2[9],
        /* KEEP WHITE    */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ lut_GC2[50], lut_GC2[51], lut_GC2[52], lut_GC2[53], /* REPEAT */ lut_GC2[54],
        /* GROUP 2 */ lut_GC2[55], lut_GC2[56], lut_GC2[57], lut_GC2[58], /* REPEAT */ lut_GC2[59],
        /* GROUP 3 */ lut_GC2[60], lut_GC2[61], lut_GC2[62], lut_GC2[63], /* REPEAT */ lut_GC2[64],
        /* GROUP 4 */ lut_GC2[65], lut_GC2[66], lut_GC2[67], lut_GC2[68], /* REPEAT */ lut_GC2[69],
        /* GROUP 5 */ lut_GC2[70], lut_GC2[71], lut_GC2[72], lut_GC2[73], /* REPEAT */ lut_GC2[74],
        /* GROUP 6 */ lut_GC2[75], lut_GC2[76], lut_GC2[77], lut_GC2[78], /* REPEAT */ lut_GC2[79],
        /* GROUP 7 */ lut_GC2[80], lut_GC2[81], lut_GC2[82], lut_GC2[83], /* REPEAT */ lut_GC2[84],
        /* GROUP 8 */ lut_GC2[85], lut_GC2[86], lut_GC2[87], lut_GC2[88], /* REPEAT */ lut_GC2[89],
        /* GROUP 9 */ lut_GC2[90], lut_GC2[91], lut_GC2[92], lut_GC2[93], /* REPEAT */ lut_GC2[94],
        /* GROUP10 */ lut_GC2[95], lut_GC2[96], lut_GC2[97], lut_GC2[98], /* REPEAT */ lut_GC2[99],
        /* FRAMERATE */ lut_GC2[100], lut_GC2[101], lut_GC2[102], lut_GC2[103], lut_GC2[104]
};

const unsigned char EpPartialDisplay::lut_DU2[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* KEEP BLACK */ 0b00000000, 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO WHITE   */ 0b00000001, 0b00101010, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO BLACK   */ 0b00001010, 0b01010101, 0, 0, 0, 0, 0, 0, 0, 0,
        /* KEEP WHITE */ 0b00000000, 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 0, 20, 20, /* REPEAT */ 0,
        /* GROUP 2 */ 20, 12, 20, 20, /* REPEAT */ 0,
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

const unsigned char EpPartialDisplay::lut_A2[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* KEEP BLACK */ 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO WHITE   */ 0b00001000, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* TO BLACK   */ 0b00000100, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* KEEP WHITE */ 0b00000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 0, 8, 1, /* REPEAT */ 2, // TODO: one long pull or multiple small pulls better?
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

EpPartialDisplay::EpPartialDisplay(Epepd &epepd) : EpFunction(epepd) {}

/// TODO: prep for next windowed update
void EpPartialDisplay::display(EpBitmap &source, EpPlacement &placement, DisplayMode displayMode, EpBitmap* partial, EpBitmap* force, EpRegion* updateRegion) {
    uint64_t start = esp_timer_get_time();
    uint64_t total = esp_timer_get_time();

    if (updateRegion) // make pixels outside updateRegion not update
        memcpy(epepd->getRedRam()->_getBuffer(), epepd->getBwRam()->_getBuffer(), (uint32_t(epepd->EPD_WIDTH) * epepd->EPD_HEIGHT) >> 3);

    EpRegion region = EpRegion::_testAndAlign(updateRegion, epepd->EPD_WIDTH, epepd->EPD_HEIGHT);
    Serial.printf("[epepd] Updating region X from %d to %d, Y from %d to %d, that is %d%% of a full frame!\n",
                  region.x, region.x + region.w, region.y, region.y + region.h, int(100.f * region.h * region.w / epepd->EPD_WIDTH / epepd->EPD_HEIGHT));

    switch (displayMode) {
        case FIX:
            epepd->initDisplay();
            epepd->writeLUT(lut_FIX);
            epepd->writeToDisplay();
            epepd->updateDisplay();
            break;
        case GC2_FULL:
        case DU2:
        case A2: {
            uint8_t src = 0x00; // new data
            uint8_t old; // old data
            uint8_t par = (partial) ? 0x00 : 0xFF; // partial update
            uint8_t fce = 0x00; // force update

            for (int16_t y = region.y; y < region.y + region.h; y++) {
                for (int16_t x = region.x; x < region.x + region.w; x += 8) {
                    for (int16_t dx = 0; dx < 8; dx++) {
                        src |= (source.getPixel(placement.getSourcePos(x + dx, y)) & 0x80) >> dx;
                        if (partial) // TODO: this has potential of reading 8 bits at a time... but it may be misaligned due to placement...
                            par |= (partial->getPixel(placement.getSourcePos(x + dx, y)) & 0x80) >> dx;
                        if (force)
                            fce |= (force->getPixel(placement.getSourcePos(x + dx, y)) & 0x80) >> dx;
                    }

                    old = epepd->getBwRam()->_get8MonoPixels(x, y);
                    // derived from good old Kmap
                    epepd->getRedRam()->_set8MonoPixels(x, y, (~src & fce) | (old & ~fce));
                    epepd->getBwRam()->_set8MonoPixels(x, y, (src & (fce | par)) | (old & ~(fce | par)));
                    src = 0x00;
                    par = (partial) ? 0x00 : 0xFF;
                    fce = 0x00;
                }
            }
            Serial.printf("[epepd] EpPartialDisplay calculate lut took %lldus\n", esp_timer_get_time() - start);

            epepd->initDisplay();
            switch (displayMode) {
                case GC2_FULL:
                    epepd->writeLUT(lut_GC2);
                    break;
                case DU2:
                    epepd->writeLUT(lut_DU2);
                    break;
                case A2:
                    epepd->writeLUT(lut_A2);
            }
            epepd->writeToDisplay();
            epepd->updateDisplay();
            break;
        }
        case GC2_PARTIAL: { /// TODO: use optimized code here too
            uint64_t start = esp_timer_get_time();
            uint8_t src = 0x00; // new data
            uint8_t old; // old data
            uint8_t par = (partial) ? 0x00 : 0xFF; // partial update
            uint8_t fce = 0x00; // force update

            for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
                    src |= (source.getPixel(x, y) & 0x80) >> (x & 0b111);
                    if (partial)
                        par |= (partial->getPixel(x, y) & 0x80) >> (x & 0b111);
                    if (force)
                        fce |= (force->getPixel(x, y) & 0x80) >> (x & 0b111);

                    if ((x & 0b111) == 0b111) {
                        old = epepd->getBwRam()->_get8MonoPixels(x, y);
                        // derived from good old Kmap
                        epepd->getRedRam()->_set8MonoPixels(x, y, old);
                        epepd->getBwRam()->_set8MonoPixels(x, y, (src & (fce | par)) | (old & ~(fce | par)));
                        src = 0x00;
                        par = (partial) ? 0x00 : 0xFF;
                        fce = 0x00;
                    }
                }
            }
            Serial.printf("[epepd] EpPartialDisplay (round 1) write ram took %lldus\n", esp_timer_get_time() - start);

            epepd->initDisplay();
            epepd->writeLUT(lut_GC2_PARTIAL_1);
            epepd->writeToDisplay();
            epepd->updateDisplay();

            start = esp_timer_get_time();
            for (int16_t y = 0; y < epepd->EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < epepd->EPD_WIDTH; x++) {
                    src |= (source.getPixel(x, y) & 0x80) >> (x & 0b111);
                    if (partial)
                        par |= (partial->getPixel(x, y) & 0x80) >> (x & 0b111);
                    if (force)
                        fce |= (force->getPixel(x, y) & 0x80) >> (x & 0b111);

                    if ((x & 0b111) == 0b111) {
                        old = epepd->getBwRam()->_get8MonoPixels(x, y);
                        // derived from good old Kmap
                        epepd->getRedRam()->_set8MonoPixels(x, y, (fce & ~old) | (par & ~fce & src) | (~(par | fce) & old));
                        epepd->getBwRam()->_set8MonoPixels(x, y, (src & (fce | par)) | (old & ~(fce | par)));
                        src = 0x00;
                        par = (partial) ? 0x00 : 0xFF;
                        fce = 0x00;
                    }
                }
            }
            Serial.printf("[epepd] EpPartialDisplay (round 2) write ram took %lldus\n", esp_timer_get_time() - start);

            epepd->writeLUT(lut_GC2_PARTIAL_2);
            epepd->writeToDisplay();
            epepd->updateDisplay();
            break;
        }
    }

    epepd->initDisplay();
    Serial.printf("[epepd] EpPartialDisplay in total took %lldus\n", esp_timer_get_time() - total);
}

