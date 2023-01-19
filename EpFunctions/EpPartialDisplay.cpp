#include "EpPartialDisplay.h"

EpPartialDisplay::EpPartialDisplay(Epepd &epepd) :
        EpFunction(epepd) {}

uint8_t EpPartialDisplay::getPixelLut(int16_t x, int16_t y) {
    uint8_t newState = (EpFunction::epepd->getGfxBuffer()->getPixel(x, y) & 0x80) ? 1 : 0;
    if (disregardStateMask && !(disregardStateMask->getPixel(x, y) & 0x80)) // disregardStateMask enabled and is dark
        return newState ? LUT01 : LUT10; // turn white : turn black

    if (partialUpdateMask && (partialUpdateMask->getPixel(x, y) & 0x80))  // partialUpdateMask enabled and is light
        return newState ? LUT11 : LUT00; // no change
    if (newState != ((EpFunction::epepd->getBwRam()->getPixel(x, y) & 0x80) ? 1 : 0))
        return newState ? LUT01 : LUT10; // turn white : turn black

    return newState ? LUT11 : LUT00; // no change
}

void EpPartialDisplay::setPartialUpdateMask(EpBitmap* m) {
    this->partialUpdateMask = m;
}

void EpPartialDisplay::setDisregardStateMask(EpBitmap* m) {
    this->disregardStateMask = m;
}

void EpPartialDisplay::execute() {
    epepd->initDisplay();
    epepd->writeToDisplay(*this); // will take "getPixelLut" from "this" EpFunction
    epepd->updateDisplay();
}
