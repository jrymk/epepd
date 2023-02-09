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
        FIX,         // fix burn in caused by accidents
        INIT,        // clears the screen
        GC2_FULL,    // fast, high quality, will flash once (not partial display)
        GC2_PARTIAL, // fast, high quality, will flash once for updated pixels in partialUpdateMask, requires two update cycles
        DU2,         // faster, low quality, good for setting the display to all white
        A2,          // fastest, low quality, good for motion
    };

    // clears the display
    void clear();

    /**
     * Displays a bitmap onto the screen
     * @param source The source bitmap image
     * @param placement How the source image (and masks) is placed onto the target bitmap
     * @param displayMode Update with the given display mode
     * @param partial Mask for partial update, placement applied, bright pixels will be updated if pixels changed, set to null to enable partial update on the whole screen
     * @param force Mask for force update, placement applied, bright pixels will be updated regardless of previous image, set to null to disable
     * @param updateRegion The region that will be scanned, in target bitmap coords (placement not applied).
     *                     Source image and masks are disregarded outside the updateRegion and assumed unchanged. Behavior depends on the display mode.
     *                     This is NOT for setting the partial update area. Pixels outside the region may be scanned (and updated if partialUpdateMask set incorrectly) as well.
     * Refer README.md for descriptions of the display modes
     */
    void display(EpBitmap *source, EpPlacement &placement, DisplayMode displayMode, EpBitmap *partial = nullptr, EpBitmap *force = nullptr,
                 EpRegion *updateRegion = nullptr);

private:
    static const uint8_t lut_FIX[];
    static const uint8_t lut_INIT[];
    static const uint8_t lut_GC2[];
    static const uint8_t lut_GC2_PARTIAL_1[];
    static const uint8_t lut_GC2_PARTIAL_2[];
    static const uint8_t lut_DU2[];
    static const uint8_t lut_A2[];
};

#endif //EPPARTIALDISPLAY_H
