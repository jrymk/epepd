#include "EpIntegerScaling.h"

EpIntegerScaling::EpIntegerScaling(EpBitmap *source, int8_t scale) :
        EpBitmap(
                (scale >= 0 ? source->width * scale : source->width / (-scale)),
                (scale >= 0 ? source->height * scale : source->height / (-scale)),
                8), // bpp for the base EpBitmap doesn't matter
        src(source),
        scale(scale) {}

uint8_t EpIntegerScaling::getPixel(int16_t x, int16_t y) {
    if (!src) {
        Serial.printf("[epepd] No source bitmap provided!\n");
        return 0;
    }
    if (scale > 0)
        return src->getPixel(x / scale, y / scale);

    uint16_t sum = 0;
    for (int dx = 0; dx < -scale; dx++) {
        for (int dy = 0; dy < -scale; dy++)
            sum += src->getPixel(x * (-scale) + dx, y * (-scale) + dy);
    }
    return (sum + (scale * scale / 2)) / (scale * scale); // rounding
}

void EpIntegerScaling::setSourceBitmap(EpBitmap *source) {
    src = source;
}

void EpIntegerScaling::setScale(int8_t s) {
    this->scale = s;
}
