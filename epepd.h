#ifndef HYBRIDEPD_H
#define HYBRIDEPD_H

#define BLACK 0x0000
#define DARKGREY 0x7BEF
#define LIGHTGREY 0xC618
#define WHITE 0xFFFF

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <functional>

class epepd : public Adafruit_GFX {
public:
    epepd(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin);

    void init();

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    void display();

    void hwReset();

    void powerOn();

    void powerOff();

    void hibernate();

    void displayTest();

    void debugPrintBuffer(uint8_t* buffer, uint8_t bitsPerPixel);

private:
    static const int16_t WIDTH = 280;
    static const int16_t HEIGHT = 480;
    static const unsigned char lut_4G[];

    // this can't be here...
    uint8_t buffer[int32_t(WIDTH) * int32_t(HEIGHT) / 2]; // the buffer you draw on

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

    // bufferToLUT: given x and y, derive the LUT from the two buffers (red(old, 0x26): bit 1, b/w(new, 0x24): bit 0)
    void writeBufferToMemory(std::function<uint8_t(epepd &buff, int16_t x, int16_t y)> bufferToLUT);

    void setRamWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

    void writeCommand(uint8_t c);

    void writeDataBegin();

    void writeData(uint8_t d);

    void writeDataCont(uint8_t d);

    void writeDataEnd();

    void waitUntilIdle();
};

#endif
