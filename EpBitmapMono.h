#ifndef EPBITMAPMONO_H
#define EPBITMAPMONO_H

#include <Arduino.h>
#include "EpBitmap.h"

/*
 * A one-bit variant of the EpBitmap, optimized for byte streaming
 */
class EpBitmapMono : public EpBitmap {
public:
    EpBitmapMono(int16_t w, int16_t h);

    ~EpBitmapMono() override;

    // allocate the bitmap memory, NOT split into blocks
    bool allocate();

    void deallocate() override;

    uint8_t getBitmapPixel(uint32_t x, uint32_t y) override;

    void setBitmapPixel(uint32_t x, uint32_t y, uint8_t color) override;

    /// for pros ///
    uint8_t* _getBuffer();

    uint8_t _get8MonoPixels(int16_t x, int16_t y) override;

    void _set8MonoPixels(int16_t x, int16_t y, uint8_t pixels) override;

private:
    using EpBitmap::allocate;

    uint8_t* buffer;
};

#endif //EPBITMAPMONO_H
