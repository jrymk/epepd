#ifndef EPGREYSCALEDISPLAY_H
#define EPGREYSCALEDISPLAY_H

#include "../Epepd.h"

/**
 * Displays greyscale bitmaps to the screen
 */
class EpGreyscaleDisplay : public EpFunction {
public:
    EpGreyscaleDisplay(Epepd &epepd);

    enum DisplayMode {
        GC4,
        GC16_FAST, // this mode does not guarantee bwRam contents after display
        GC16, // this mode does not guarantee bwRam contents after display
    };

    void display(EpBitmap* source, EpPlacement &placement, DisplayMode displayMode);

private:
    static const uint8_t lut_GC4[];
    static const uint8_t lut_GC16_FAST_1[];
    static const uint8_t lut_GC16_FAST_2[];
    static const uint8_t lut_GC16_1[];
    static const uint8_t lut_GC16_2[];
    static const uint8_t lut_GC16_3[];
    static const uint8_t lut_64_to_16[];
};

#endif //EPGREYSCALEDISPLAY_H
