#ifndef EPEPD_H
#define EPEPD_H

#define EPEPD_SHOW_VERBOSE 1

#define LUT0 0
#define LUT1 1
#define LUT2 2
#define LUT3 3
#define LUT00 0
#define LUT01 1
#define LUT10 2
#define LUT11 3 // uSefUl! I know!

#define IS_LIGHT(c) (c & 0x80)
#define IS_DARK(c) !(c & 0x80)
#define IS_WHITE(c) ((c & 0xC0) == 0xC0)
#define IS_LIGHTGREY(c) ((c & 0x80) == 0x80)
#define IS_DARKGREY(c) ((c & 0x40) == 0x40)
#define IS_BLACK(c) ((c & 0x00) == 0x00)

#define LUT_SIZE 105

#include <SPI.h>
#include <functional>
#include "EpBitmap.h"
#include "EpBitmapFast.h"
#include "EpFunction.h"

class Epepd {
public:
    Epepd(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin);

    /// EPAPER DISPLAY ///
    const int16_t EPD_WIDTH = 280;
    const int16_t EPD_HEIGHT = 480;

    // initialize pinModes, SPI, and bitmap buffers
    void init();

    // initialize display settings after hibernation
    void initDisplay();

    void writeLUT(const uint8_t* data);

    enum BorderStyle {
        BORDER_HI_Z,
        BORDER_LUT0,
        BORDER_LUT1,
        BORDER_LUT2,
        BORDER_LUT3,
    };

    void setBorder(BorderStyle border);

    // EpFunctions may do this themselves for efficiency (for basic ones like partialDisplay, you can stream bytes and do bitwise operation by byte)
    void writeToDisplay();

    void updateDisplay();

    // to (basically) completely shutdown the display
    void hibernate();

    // to recover from powerOff
    void powerOn();

    // to turn off panel voltage generator so the image doesn't fade
    void powerOff();

    // get LUT bitmap (2 bit, redRam + bwRam)
    // You can call Adafruit_GFX draw commands on this bitmap to draw directly, though the contents may (very well) be
    // changed after running display EpFunctions (like partialDisplay)
    // But each buffer is only 33.6KB, you should probably just allocate your own
    EpBitmap* getDisplayRam();

    // get redRam bitmap (1 bit)
    EpBitmap* getRedRam();

    // get bwRam bitmap (1 bit)
    EpBitmap* getBwRam();

private:
    static const unsigned char lut_GC4[];

    // about 16.8KB each
    EpBitmap redRam; // LUT bit 1, previous (for partial)
    EpBitmap bwRam;  // LUT bit 0, current (for partial)

    bool isPoweredOn = false;  // is clock signal active? (disable voltage generation to avoid screen fade)
    bool isHibernating = true; // hibernating requires hwReset + initDisplay to restore

    int16_t csPin, dcPin, rstPin, busyPin;
    SPIClass* spi;
    SPISettings spiSettings;

    struct TransitionResult {
        uint8_t newDisplayBuffer;
        bool red;
        bool bw;
    };

    // to recover from hibernation
    void hwReset();

    // when I finally make bitmap functions fast enough, this may do something for the performance
    void setRamWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

    void writeCommand(uint8_t c);

    void writeData(uint8_t d);

    void writeDataBegin();

    void writeDataCont(uint8_t d);

    void writeDataEnd();

    void waitUntilIdle();
};

#endif
