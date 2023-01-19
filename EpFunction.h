#ifndef EPFUNCTION_H
#define EPFUNCTION_H

class Epepd;

// base class for all the possibilities
class EpFunction {
public:
    EpFunction(Epepd &epepd);

    virtual void init() = 0;

    // run the function
    virtual void execute() = 0;

private:
    Epepd* epepd;
};

#endif //EPFUNCTION_H
