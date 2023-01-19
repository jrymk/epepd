#include "EpFunction.h"
#include "Epepd.h"

EpFunction::EpFunction(Epepd &epepd) {
    this->epepd = &epepd;
}

uint8_t EpFunction::getPixelLut(int16_t x, int16_t y) {
    return (x >> 2 & 1) != (y >> 2 & 1);
}
