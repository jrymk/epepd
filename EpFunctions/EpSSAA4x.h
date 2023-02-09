#ifndef EPSSAA4X_H
#define EPSSAA4X_H

#include "../Epepd.h"

class EpSSAA4x : public EpBitmap {
public:
    EpSSAA4x(int16_t w, int16_t h, uint8_t bitsPerPixel);

    uint8_t getPixel(int16_t x, int16_t y) override;

    uint8_t getPixel(std::pair<int16_t, int16_t> coord) override { return getPixel(coord.first, coord.second); }

    void setPixel(int16_t x, int16_t y, uint8_t color) override;

    /// TODO: override primitives drawing from Adafruit_GFX. Modified Adafruit_GFX will be needed...
private:
    uint8_t outputBpp;

    uint8_t getInternalPixel(int16_t x, int16_t y);
};

#endif
