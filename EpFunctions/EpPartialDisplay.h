#ifndef EPPARTIALDISPLAY_H
#define EPPARTIALDISPLAY_H

#include "../Epepd.h"

/// @brief Enable partial display on a given display area
class EpPartialDisplay : public EpFunction {
public:
    EpPartialDisplay(Epepd &epepd);

    uint8_t getPixelLut(int16_t x, int16_t y) override;

    // nullptr to disable mask, if using mask, only dark pixels (first bit 0) will be updated
    void setPartialUpdateMask(EpBitmap* m);

    // nullptr to disable mask, if using mask, dark pixels will be updated no matter what (still doesn't flash, use when the old bwRam is lost)
    void setDisregardStateMask(EpBitmap* m);

    // run the function
    void execute() override;

private:
    EpBitmap* partialUpdateMask = nullptr;
    EpBitmap* disregardStateMask = nullptr;
};

#endif //EPPARTIALDISPLAY_H
