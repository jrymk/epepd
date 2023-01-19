#ifndef EPBITMAP_H
#define EPBITMAP_H

#include <Arduino.h>

class EpBitmap {
public:
    EpBitmap(int16_t w, int16_t h, uint8_t bitsPerPixel);

    int16_t width() const;

    int16_t height() const;

    bool isAllocated() const;

    // allocate the bitmap memory, split into blocks, blockSize in bytes (you are not going to allocate multiple blocks over 64K on the esp32, so that's the biggest you can do)
    bool allocate(uint16_t blockSize);

    // do we need pixels over 8 bit wide?
    // get the color of a particular pixel, aligned left
    uint8_t getPixel(int16_t x, int16_t y);

    void setPixel(int16_t x, int16_t y, uint8_t color);

    uint8_t outOfBoundsColor = 0b10100000;

    void streamBytesReset();

    // keep count of bytes read yourself, it will certainly crash if you go over!
    uint8_t streamBytesNext();

private:
    int16_t WIDTH, HEIGHT;
    uint8_t BPP;

    bool allocated = false;
    uint8_t** blocks;
    uint16_t blockSize; // in bytes
    /// TODO: vector mode

    uint16_t streamBytesCurrentBlock;
    uint32_t streamBytesCurrentByte;
};


#endif //EPBITMAP_H
