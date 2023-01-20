#ifndef EPPARTIALDISPLAY_H
#define EPPARTIALDISPLAY_H

#include "../Epepd.h"

/**
 * Displays bitmap to the screen using partial update in a given area
 * Different LUTs can be selected to provide different display quality
 */
class EpPartialDisplay : public EpFunction {
public:
    EpPartialDisplay(Epepd &epepd);

    enum DisplayMode {
        GC2_FULL,    // fast, high quality, will flash once (not partial display)
        GC2_PARTIAL, // fast, high quality, will flash once for updated pixels in mask, requires two update cycles
        DU,          // faster, low quality, good for setting the display to all white
        A2,          // fastest, low quality, good for motion
    };

    /**
     * Displays a bitmap onto the screen
     * @param source The source bitmap image
     * @param displayMode Update with the given display mode
     * @param partial Mask for partial update, bright pixels will be updated if pixels changed, set to null to enable partial update on the whole screen
     * @param force Mask for force update, bright pixels will be updated regardless of previous image, set to null to disable
     * Refer README.md for descriptions of the display modes
     */
    void display(EpBitmap* source, DisplayMode displayMode, EpBitmap* partial = nullptr, EpBitmap* force = nullptr);

private:
    static const unsigned char lutGC1_FULL[];
    static const unsigned char lutGC1_PARTIAL_1[];
    static const unsigned char lutGC1_PARTIAL_2[];
    static const unsigned char lutGC1_DU1[];
    static const unsigned char lutGC1_A2[];
};

#endif //EPPARTIALDISPLAY_H
