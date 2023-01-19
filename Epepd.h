#ifndef HYBRIDEPD_H
#define HYBRIDEPD_H

#define BLACK 0x0000
#define DARKGREY 0x7BEF
#define LIGHTGREY 0xC618
#define WHITE 0xFFFF

#define LUT0 0
#define LUT1 1
#define LUT2 2
#define LUT3 3 // uSefUl! I know!

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <functional>
#include "EpBitmap.h"

class Epepd : public Adafruit_GFX {
public:
    Epepd(EpBitmap &gfxBuffer, int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin);

    void init();

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    void display();

    void hwReset();

    void powerOn();

    void powerOff();

    void hibernate();

    void displayTest();

    void debugPrintBuffer(uint8_t* buffer, uint8_t bitsPerPixel);

    // getPixelLut: given x and y, derive the LUT from the two buffers (red(old, 0x26): bit 1, b/w(new, 0x24): bit 0)
    //              will have access to gfxBuffer, redRam and bwRam. Note that redRam and bwRam will be changed after calling, so only use it if doing single pixel operations
    //              depending on the rotation, this function will get values x from 0 to width OR from 0 and height, and y the other way around
    void writeToDisplay(std::function<uint8_t(Epepd &epepd, int16_t x, int16_t y)> getPixelLut);

    void setRotation(uint8_t r) override;

    uint8_t getRotation(void) const;

private:
    static const int16_t EPD_WIDTH = 280;
    static const int16_t EPD_HEIGHT = 480;
    static const unsigned char lut_4G[];

    // since we now use Adafruit_GFX purely for rendering to a texture, it's drawing functions should not be affected by the rotation, so we have to use our
    // own rotation variable so Adafruit_GFX always thinks the display is the same resolution as the buffer
    uint8_t rotation;

    EpBitmap* gfxBuffer;

    // about 16.8KB each
    EpBitmap redRam; // LUT bit 1, previous (for partial)
    EpBitmap bwRam;  // LUT bit 0, current (for partial)

    bool isPoweredOn = false;
    bool isHibernating = false;

    static uint16_t getLuminance(uint16_t color);

    int16_t csPin, dcPin, rstPin, busyPin;
    SPIClass* spi;
    SPISettings spiSettings;
    bool isWritingData = false;

    void initDisplay();

    struct TransitionResult {
        uint8_t newDisplayBuffer;
        bool red;
        bool bw;
    };

    static uint8_t getBufferPixel(uint8_t* buffer, uint16_t x, uint16_t y);

    static uint8_t getBufferPixel(uint8_t* buffer, uint8_t bitsPerPixel, uint16_t x, uint16_t y);

    void setRamWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

    void writeCommand(uint8_t c);

    void writeDataBegin();

    void writeData(uint8_t d);

    void writeDataCont(uint8_t d);

    void writeDataEnd();

    void waitUntilIdle();
};

#endif
