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
        GC16 // this mode does not guarantee bwRam contents after display
    };

    void display(EpBitmap* source, DisplayMode displayMode);

private:
    static const uint8_t lut_GC4[];
    static const uint8_t lut_GC16_1[];
    static const uint8_t lut_GC16_2[];
};

#endif //EPGREYSCALEDISPLAY_H
