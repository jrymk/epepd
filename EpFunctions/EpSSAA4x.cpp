#include "EpSSAA4x.h"

/*
 * Hmm... can't seem to use arithmetics
 *
 * [0] 00 <-> 0 0 0 0 [0]
 * [1] 01 <-> 0 0 0 1 [1]
 * [2] 10 <-> 0 0 1 1 [2]
 * [3] 11 <-- 0 1 1 1 [3]
 * [3] 11 <-> 1 1 1 1 [4]
 *
 * [0] 000 <-> 00 00 00 00 [0]
 * [1] 001 <-> 00 00 01 01 [2]
 * [2] 010 <-> 01 01 01 01 [4]
 * [3] 011 <-> 01 01 10 10 [6]
 * [4] 100 <-> 01 10 10 10 [7]
 * [5] 101 <-> 10 10 10 10 [8]
 * [6] 110 <-> 10 10 11 11 [10]
 * [7] 111 <-> 11 11 11 11 [12]
 *
 * (!): ordered pattern needed to differentiate
 * [0] 0000 <-> 00 00 00 00 [0]
 * [1] 0001 <-> 00 00 00 01 [1]
 * [2] 0010 <-> 00 00 01 01 [2]
 * [3] 0011 <-> 00 01 01 01 [3] (!)
 * [4] 0100 --> 01 00 01 01 [3] (!)
 * [5] 0101 <-> 01 01 01 01 [4]
 * [6] 0110 <-> 01 01 01 10 [5]
 * [7] 0111 <-> 01 01 10 10 [6]
 * [8] 1000 <-> 01 10 10 10 [7] (!)
 * [9] 1001 --> 10 01 10 10 [7] (!)
 * [10] 1010 <-> 10 10 10 10 [8]
 * [11] 1011 <-> 10 10 10 11 [9]
 * [12] 1100 <-> 10 10 11 11 [10]
 * [13] 1101 <-> 10 11 11 11 [11] (!)
 * [14] 1110 --> 11 10 11 11 [11] (!)
 * [15] 1111 <-> 11 11 11 11 [12]
 */

const uint8_t lut_4bpp_4_to_1[13] PROGMEM = {0, 1, 2, 3, 5, 6, 7, 8, 10, 11, 12, 13, 15};
const uint8_t lut_3bpp_4_to_1[13] PROGMEM = {0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 6, 7, 7};
const uint8_t lut_2bpp_4_to_1[5] PROGMEM = {0, 1, 2, 3, 3};
const uint8_t lut_4bpp_1_to_4[16] PROGMEM = {0, 1, 2, 3, 3, 4, 5, 6, 7, 7, 8, 9, 10, 11, 11, 12};
const uint8_t lut_3bpp_1_to_4[8] PROGMEM = {0, 2, 4, 6, 7, 8, 10, 12};
const uint8_t lut_2bpp_1_to_4[4] PROGMEM = {0, 1, 2, 4};
// elegant? no

EpSSAA4x::EpSSAA4x(int16_t w, int16_t h, uint8_t bitsPerPixel) :
        EpBitmap(2 * w, 2 * h, bitsPerPixel == 2 ? 1 : bitsPerPixel == 3 ? 2 : bitsPerPixel == 4 ? 2 : 1),
        outputBpp(bitsPerPixel) {
    if (bitsPerPixel != 2 && bitsPerPixel != 3 && bitsPerPixel != 4)
        Serial.printf("EpSSAA4x currently only supports BPP of 2, 3, and 4\n");
}

uint8_t EpSSAA4x::getPixel(int16_t x, int16_t y) {
//    Serial.printf("getting ssaa bitmap %d, %d\n", x, y);
//    return getInternalPixel(x, y);
    uint8_t a = getInternalPixel(2 * x, 2 * y);
    uint8_t b = getInternalPixel(2 * x + 1, 2 * y);
    uint8_t c = getInternalPixel(2 * x, 2 * y + 1);
    uint8_t d = getInternalPixel(2 * x + 1, 2 * y + 1);
    if (outputBpp == 4) {
        uint8_t color = pgm_read_byte(lut_4bpp_4_to_1 + (a >> 6) + (b >> 6) + (c >> 6) + (d >> 6)) << 4;
        if (b < a && (color == 0b00110000 || color == 0b01000000 || color == 0b11010000))
            color += 0b00010000;
        return color;
    }
    if (outputBpp == 3)
        return pgm_read_byte(lut_3bpp_4_to_1 + (a >> 6) + (b >> 6) + (c >> 6) + (d >> 6)) << 5;
    if (outputBpp == 2)
        return pgm_read_byte(lut_2bpp_4_to_1 + (a >> 7) + (b >> 7) + (c >> 7) + (d >> 7)) << 6;
//    return EpBitmap::getTransparencyColor();
}

//uint8_t EpSSAA4x::getBitmapPixel(uint32_t x, uint32_t y) {
//    uint16_t a = EpBitmap::getBitmapPixel(2 * x, 2 * y);
//    uint16_t b = EpBitmap::getBitmapPixel(2 * x + 1, 2 * y);
//    uint16_t c = EpBitmap::getBitmapPixel(2 * x, 2 * y + 1);
//    uint16_t d = EpBitmap::getBitmapPixel(2 * x + 1, 2 * y + 1);
//    if (outputBpp == 4) {
//        uint8_t color = pgm_read_byte(lut_4bpp_4_to_1 + (a >> 6) + (b >> 6) + (c >> 6) + (d >> 6)) << 4;
//        if (b < a && (color == 3 || color == 8 || color == 13))
//            color += 0b00010000;
//        return color;
//    }
//    if (outputBpp == 3)
//        return pgm_read_byte(lut_3bpp_4_to_1 + (a >> 6) + (b >> 6) + (c >> 6) + (d >> 6)) << 5;
//    if (outputBpp == 2)
//        return pgm_read_byte(lut_2bpp_4_to_1 + (a >> 7) + (b >> 7) + (c >> 7) + (d >> 7)) << 6;
//    return EpBitmap::getTransparencyColor();
//}
//
//uint16_t EpSSAA4x::getShapePixel(int16_t x, int16_t y) {
//    uint16_t a = EpBitmap::getShapePixel(2 * x, 2 * y);
//    uint16_t b = EpBitmap::getShapePixel(2 * x + 1, 2 * y);
//    uint16_t c = EpBitmap::getShapePixel(2 * x, 2 * y + 1);
//    uint16_t d = EpBitmap::getShapePixel(2 * x + 1, 2 * y + 1);
//    return (a + b + c + d + 2) / 4;
//}

void EpSSAA4x::setPixel(int16_t x, int16_t y, uint8_t color) {
//    uint16_t total;
//    if (outputBpp == 4 || outputBpp == 3) {
//        total = pgm_read_byte(
//                (outputBpp == 4)
//                ? (lut_4bpp_1_to_4 + (color >> 4))
//                : (lut_3bpp_1_to_4 + (color >> 5)));
//
//        if (outputBpp == 4 && ((color & 0b11110000) == 0b01000000 || (color & 0b11110000) == 0b10010000 || (color & 0b11110000) == 0b11100000)) {
//            EpBitmap::setBitmapPixel(2 * x, 2 * y, (total / 4 + 1) << 6); // a
//            total -= total / 4 + 1;
//        }
//        else {
//            EpBitmap::setBitmapPixel(2 * x, 2 * y, (total / 4) << 6); // a
//            total -= total / 4;
//        }
//        EpBitmap::setBitmapPixel(2 * x + 1, 2 * y, (total / 3) << 6); // b
//        total -= total / 3;
//
//        EpBitmap::setBitmapPixel(2 * x, 2 * y + 1, (total / 2) << 6); // c
//        total -= total / 2;
//
//        EpBitmap::setBitmapPixel(2 * x + 1, 2 * y + 1, total << 6); // d
//    }
//    if (outputBpp == 2) {
//        total = pgm_read_byte(lut_2bpp_1_to_4 + (color >> 6));
//        EpBitmap::setBitmapPixel(2 * x + 1, 2 * y + 1, (total / 4) << 7);
//        total -= total / 4;
//        EpBitmap::setBitmapPixel(2 * x, 2 * y, (total / 3) << 7);
//        total -= total / 3;
//        EpBitmap::setBitmapPixel(2 * x + 1, 2 * y, (total / 2) << 7);
//        total -= total / 2;
//        EpBitmap::setBitmapPixel(2 * x, 2 * y + 1, total << 7);
//    }
}

uint8_t EpSSAA4x::getInternalPixel(int16_t x, int16_t y) {
//    if (blendMode == BITMAP_ONLY)
//        return EpBitmap::getBitmapPixel(x, y);
//    if (blendMode == SHAPES_ONLY) {
//        uint16_t shapesColor = EpBitmap::getShapePixel(x, y);
//        return (shapesColor == 0xFFFF) ? transparencyColor : shapesColor;
//    }
//
//    uint8_t bitmapColor;
//    uint16_t shapesColor;
//    ///// takes about 60ms per typical usage update
//    shapesColor = EpBitmap::getShapePixel(x, y);
//    ///// takes about 60ms per typical usage update
//    bitmapColor = EpBitmap::getBitmapPixel(x, y);
//
//    switch (blendMode) {
//        case BITMAP_ADD_SHAPES:
//            return (shapesColor == 0xFFFF) ? bitmapColor : shapesColor;
//        case BITMAP_SUBTRACT_SHAPES:
//            return (shapesColor == 0xFFFF) ? bitmapColor : transparencyColor;
//        case BITMAP_INTERSECT_SHAPES:
//            return (shapesColor == 0xFFFF) ? transparencyColor : bitmapColor;
//        case SHAPES_ADD_BITMAP:
//            return (bitmapColor == transparencyColor) ? ((shapesColor == 0xFFFF) ? transparencyColor : shapesColor) : bitmapColor;
//        case SHAPES_SUBTRACT_BITMAP:
//            return (bitmapColor == transparencyColor) ? ((shapesColor == 0xFFFF) ? transparencyColor : shapesColor) : transparencyColor;
//        case SHAPES_INTERSECT_BITMAP:
//            return (bitmapColor == transparencyColor) ? transparencyColor : ((shapesColor == 0xFFFF) ? transparencyColor : shapesColor);
//    }
}

