#ifndef EPINTEGERSCALING_H
#define EPINTEGERSCALING_H

#include "../Epepd.h"

class EpIntegerScaling : public EpBitmap {
public:
    // Use negative numbers for downscale
    EpIntegerScaling(EpBitmap* source, int8_t scale);

    uint8_t getPixel(int16_t x, int16_t y) override;

    void setSourceBitmap(EpBitmap* source);

    // Use negative numbers for downscale
    void setScale(int8_t scale);

private:
    EpBitmap* src;
    int8_t scale;
};


#endif
