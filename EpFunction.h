#ifndef EPFUNCTION_H
#define EPFUNCTION_H

#include <Arduino.h>

class Epepd;

// base class for all the possibilities
class EpFunction {
public:
    EpFunction(Epepd &epepd);

protected:
    Epepd* epepd;
};

#endif //EPFUNCTION_H
