#ifndef EPFUNCTION_H
#define EPFUNCTION_H

#include <Arduino.h>

class Epepd;

// base class for all the possibilities
class EpFunction {
public:
    EpFunction(Epepd &epepd);

    virtual uint8_t getPixelLut(int16_t x, int16_t y);

    // run the function
    virtual void execute() = 0;

protected:
    Epepd* epepd;
};

#endif //EPFUNCTION_H
