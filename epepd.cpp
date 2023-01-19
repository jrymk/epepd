#include "Epepd.h"

//#define USE_PERCEIVED_LUMINANCE
#define BUSY_TIMEOUT 5000000
#define RESET_DURATION 10

const unsigned char Epepd::lut_4G[] PROGMEM =
        {
//                ph0    1     2     3     4     5     6     7     8     9
                0x20, 0x48, 0x48, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut11
                0x02, 0x48, 0x48, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut10
                0x08, 0x48, 0x48, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut01
                0x40, 0x48, 0x48, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut00
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs vcom
                //00    01    10    11    rp
                0x02, 0x03, 0x01, 0x02, 0x00, // ph0
                0x09, 0x02, 0x09, 0x02, 0x00, // ph2
//                0x00, 0x00, 0x00, 0x00, 0x00, // ph2
                0x02, 0x01, 0x02, 0x01, 0x02, // ph1
                0x0A, 0x01, 0x03, 0x19, 0x00, // ph3
                0x01, 0x00, 0x00, 0x00, 0x00, // ph4
                0x00, 0x00, 0x00, 0x00, 0x00, // ph5
                0x00, 0x00, 0x00, 0x00, 0x00, // ph6
                0x00, 0x00, 0x00, 0x00, 0x00, // ph7
                0x00, 0x00, 0x00, 0x00, 0x00, // ph8
                0x00, 0x00, 0x00, 0x00, 0x00, // ph9
                0x22, 0x22, 0x22, 0x22, 0x22

//
//                0x14, 0x06, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//4
//                0x20, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//3
//                0x28, 0x06, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//2
//                0x2A, 0x06, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//1
//                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//5
//                0x00, 0x02, 0x02, 0x0A, 0x00, 0x00, 0x00, 0x08, 0x08, 0x02,//6
//                0x00, 0x02, 0x02, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//7
//                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//8
//                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//9
//                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//10
//                0x22, 0x22, 0x22, 0x22, 0x22
        };

Epepd::Epepd(EpBitmap &gfxBuffer, int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin) :
        Adafruit_GFX(gfxBuffer.width(), gfxBuffer.height()),
        gfxBuffer(&gfxBuffer),
        redRam(EPD_WIDTH, EPD_HEIGHT, 1),
        bwRam(EPD_WIDTH, EPD_HEIGHT, 1),
        spi(&SPI),
        spiSettings(40000000, MSBFIRST, SPI_MODE0) {
    this->csPin = csPin;
    this->dcPin = dcPin;
    this->rstPin = rstPin;
    this->busyPin = busyPin;
}

void Epepd::init() {
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    pinMode(dcPin, OUTPUT);
    digitalWrite(dcPin, HIGH);
    hwReset();
    pinMode(busyPin, INPUT);
    spi->begin();
    redRam.allocate(4200); // fits in 4 blocks
    bwRam.allocate(4200);
    Serial.printf("Display resolution: %d*%d\nAdafruit_GFX resolution: %d*%d\n", EPD_WIDTH, EPD_HEIGHT, Adafruit_GFX::width(), Adafruit_GFX::height());
}

void Epepd::displayTest() {
    delay(100);
    initDisplay();
    writeCommand(0x24);
    writeDataBegin();
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    writeDataEnd();

    writeCommand(0x26);
    writeDataBegin();
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    for (uint32_t i = 0; i < uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    writeDataEnd();

    writeCommand(0x22);
    writeData(0xcf); // display mode 2 with ping pong

    writeCommand(0x20); // activate
    waitUntilIdle();
}

void Epepd::initDisplay() {
    if (isHibernating)
        hwReset();
    delay(10);
    writeCommand(0x12); // sw reset
    delay(10);

    writeCommand(0x46);
    writeData(0xF7);
    waitUntilIdle();
    writeCommand(0x47);
    writeData(0xF7);
    waitUntilIdle();

    writeCommand(0x01); // driver output control
    writeData(0xDF);
    writeData(0x01); // 1DF = 480
    writeData(0x00); // scan order

    writeCommand(0x03); // gate driving voltage control
    writeData(0x00); // 20V

    writeCommand(0x04); // source driving voltage control
    writeData(0x41); // 15V
    writeData(0xA8); // 5V
    writeData(0x32); // -15V

    writeCommand(0x0C); // Booster Soft-start Control
    writeData(0xAE);
    writeData(0xC7);
    writeData(0xC3);
    writeData(0xC0);
    writeData(0x40); // level 1?

    writeCommand(0x3C); // border waveform control
    writeData(0x00);

    writeCommand(0x21); // display update control
    writeData(0x00); // normal

    writeCommand(0x18); // temperature sensor control
    writeData(0x80); // internal temperature sensor

    writeCommand(0x2C); // write VCOM register
    writeData(0x44); // -1.7

    writeCommand(0x37); // write register for display option
    writeData(0x00);
    writeData(0xff);
    writeData(0xff);
    writeData(0xff);
    writeData(0xff);
    writeData(0x4f); // enable ping pong for mode 2
    writeData(0xff);
    writeData(0xff);
    writeData(0xff);
    writeData(0xff);

    setRamWindow(0, 0, EPD_WIDTH, EPD_HEIGHT);

    writeCommand(0x32); // write lut register
    writeDataBegin();
    for (uint16_t i = 0; i < sizeof(lut_4G); i++) {
        spi->transfer(pgm_read_byte(lut_4G + i));
//        Serial.printf("%2X ", pgm_read_byte(lut_4G + i));
    }
    Serial.printf("\n");
    writeDataEnd();

    writeCommand(0x3C); // border waveform control
    writeData(0x01); // LUT1 for white

    powerOn();
}

void Epepd::drawPixel(int16_t x, int16_t y, uint16_t color) {
    gfxBuffer->setPixel(x, y, getLuminance(color) >> 8); // EpBitmap takes 8-bit color that may get truncated even more
}


void Epepd::display() {
    initDisplay();

    writeToDisplay([](Epepd &epepd, int16_t x, int16_t y) {
//        int blackPixels = 0;
//        for (int dx = -1; dx <= 1; dx++)
//            for (int dy = -1; dy <= 1; dy++)
//                if (epepd.gfxBuffer->getPixel(x + dx, y + dy)) blackPixels++;
//        const uint8_t defBlack[] = {LUT0, LUT0, LUT1, LUT1, LUT2, LUT2, LUT2, LUT2, LUT3, LUT3};
//        const uint8_t defWhite[] = {LUT0, LUT0, LUT0, LUT0, LUT0, LUT1, LUT1, LUT1, LUT2, LUT3};
//        return (epepd.gfxBuffer->getPixel(x, y) ? defBlack[blackPixels] : defWhite[blackPixels]);

        int blackPixels = 0;
        for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++)
                if (epepd.gfxBuffer->getPixel(x * 2 + dx, y * 2 + dy)) blackPixels++;
        const uint8_t def[] = {LUT0, LUT1, LUT1, LUT2, LUT3};
        return def[blackPixels];
    });

    writeCommand(0x22);
    writeData(0xCF); // display mode 2 with ping pong

    writeCommand(0x20); // activate
    waitUntilIdle();
}

uint16_t Epepd::getLuminance(uint16_t color) {
#ifdef USE_PERCEIVED_LUMINANCE
    float r = float(color & 0b1111100000000000);
    float g = float(color & 0b0000011111100000 << 5);
    float b = float(color & 0b0000000000011111 << 11);
    return std::min(uint16_t((0.299f * r * r + 0.587f * g * g + 0.114f * b * b) * 65536.f), uint16_t(0xFFFF));
#else
    return (color & 0b1111100000000000) + (color & 0b0000011111100000 << 5) + (color & 0b0000000000011111 << 11);
#endif
}

void Epepd::writeToDisplay(std::function<uint8_t(Epepd &epepd, int16_t x, int16_t y)> getPixelLut) {
    // rotation is handled here
    // (the gfxBuffer is a graphics buffer, not a display buffer anymore, you draw right-side-up stuff on the buffer, and you decide which way to map to the display)

    // try to not switch in the loop, with the cost of program size probably
    uint64_t start = esp_timer_get_time();
    switch (getRotation()) {
        case 1:
            for (int16_t y = 0; y < EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < EPD_WIDTH; x++) {
                    uint8_t lut = getPixelLut(*this, EPD_HEIGHT - y - 1, x);
                    redRam.setPixel(x, y, lut << 6);
                    bwRam.setPixel(x, y, lut << 7);
                }
            }
            break;
        case 2:
            for (int16_t y = 0; y < EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < EPD_WIDTH; x++) {
                    uint8_t lut = getPixelLut(*this, EPD_WIDTH - x - 1, EPD_HEIGHT - y - 1);
                    redRam.setPixel(x, y, lut << 6);
                    bwRam.setPixel(x, y, lut << 7);
                }
            }
            break;
        case 3:
            for (int16_t y = 0; y < EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < EPD_WIDTH; x++) {
                    uint8_t lut = getPixelLut(*this, y, EPD_WIDTH - x - 1);
                    redRam.setPixel(x, y, lut << 6);
                    bwRam.setPixel(x, y, lut << 7);
                }
            }
            break;
        default: // no rotation
            for (int16_t y = 0; y < EPD_HEIGHT; y++) {
                for (int16_t x = 0; x < EPD_WIDTH; x++) {
                    uint8_t lut = getPixelLut(*this, x, y);
                    redRam.setPixel(x, y, lut << 6);
                    bwRam.setPixel(x, y, lut << 7);
                }
            }
    }
    Serial.printf("getPixelLut took %lldus\n", esp_timer_get_time() - start);

    start = esp_timer_get_time();
    uint32_t size = uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 8;
    writeCommand(0x26);
    writeDataBegin();
    redRam.streamBytesReset();
    for (uint32_t b = 0; b < size; b++)
        writeDataCont(redRam.streamBytesNext());
    writeDataEnd();

    writeCommand(0x24);
    writeDataBegin();
    bwRam.streamBytesReset();
    for (uint32_t b = 0; b < size; b++)
        writeDataCont(bwRam.streamBytesNext());
    writeDataEnd();

    Serial.printf("Sending two sets of display buffer took %lldus\n", esp_timer_get_time() - start);
}

uint8_t Epepd::getBufferPixel(uint8_t* buffer, uint16_t x, uint16_t y) {
    return buffer[uint32_t(y) * EPD_WIDTH + x];
};

uint8_t Epepd::getBufferPixel(uint8_t* buffer, uint8_t bitsPerPixel, uint16_t x, uint16_t y) {
    return (buffer[(uint32_t(y) * EPD_WIDTH + uint32_t(x)) * bitsPerPixel / 8] >> (bitsPerPixel - (uint32_t(y) * EPD_WIDTH + x) * bitsPerPixel % 8)) & ((1 << bitsPerPixel) - 1);
}

void Epepd::writeCommand(uint8_t c) {
    spi->beginTransaction(spiSettings);
    digitalWrite(dcPin, LOW);
    digitalWrite(csPin, LOW);
    spi->transfer(c);
    digitalWrite(csPin, HIGH);
    digitalWrite(dcPin, HIGH);
    spi->endTransaction();
}

void Epepd::writeDataBegin() {
    if (isWritingData)
        Serial.printf("ERROR: already writing data, did you forgot to end?\n");
    spi->beginTransaction(spiSettings);
    digitalWrite(csPin, LOW);
    isWritingData = true;
}

void Epepd::writeData(uint8_t d) {
    writeDataBegin();
    writeDataCont(d);
    writeDataEnd();
}

void Epepd::writeDataCont(uint8_t d) {
    spi->transfer(d);
}

void Epepd::writeDataEnd() {
    digitalWrite(csPin, HIGH);
    spi->endTransaction();
    isWritingData = false;
}

void Epepd::waitUntilIdle() {
    uint64_t start = esp_timer_get_time();
    while (true) {
        delay(1);
        if (digitalRead(busyPin) != HIGH) break;
        if (esp_timer_get_time() > start + BUSY_TIMEOUT) {
            Serial.printf("Busy timed out\n");
            break;
        }
#if defined(ESP8266) || defined(ESP32)
        yield();
#endif
    }
    Serial.printf("Wait took %lldus\n", esp_timer_get_time() - start);
}

void Epepd::setRamWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    writeCommand(0x11); // data entry mode setting
    writeData(0x03); // x increment, y increment, updated on x direction

    writeCommand(0x44); // set ram x address start/end position
    writeData(x & 0xFF);
    writeData(x >> 8);
    writeData((x + w - 1) & 0xFF); // from 0 to 439 is (x = 0, w = 480)
    writeData((x + w - 1) >> 8);

    writeCommand(0x45); // set ram y address start/end position
    writeData(y & 0xFF);
    writeData(y >> 8);
    writeData((y + h - 1) & 0xFF);
    writeData((y + h - 1) >> 8);

    writeCommand(0x4e); // set ram x address counter
    writeData(x & 0xFF);
    writeData(x >> 8);

    writeCommand(0x4f); // set ram y address counter
    writeData(y & 0xFF);
    writeData(y >> 8);
}

void Epepd::hwReset() {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(RESET_DURATION);
    digitalWrite(rstPin, LOW);
    delay(RESET_DURATION);
    digitalWrite(rstPin, HIGH);
    delay(RESET_DURATION);
    isHibernating = false;
}

void Epepd::hibernate() {
    powerOff();
    writeCommand(0x10); // deep sleep mode
    writeData(0x03); // enter deep sleep
    isHibernating = true;
}

void Epepd::powerOn() {
    if (!isPoweredOn) {
        writeCommand(0x22);
        writeData(0xc0); // enable clock signal -> enable analog

        writeCommand(0x20);
        waitUntilIdle();
    }
    isPoweredOn = true;
}

void Epepd::powerOff() {
    if (isPoweredOn) {
        writeCommand(0x22);
        writeData(0x03); // disable analog -> disable clock signal

        writeCommand(0x20);
        waitUntilIdle();
    }
    isPoweredOn = false;
}

void Epepd::debugPrintBuffer(uint8_t* buffer, uint8_t bitsPerPixel) {
    for (uint16_t y = 0; y < EPD_HEIGHT; y++) {
        for (uint16_t x = 0; x < EPD_WIDTH; x++) {
            float value = getBufferPixel(buffer, bitsPerPixel, x, y);
//            Serial.printf("pixel %d %d is %d\n", x, y, getBufferPixel(buffer, bitsPerPixel, x, y));
            float maxValue = float(int(1 << bitsPerPixel));
            char set[] = "'-.,^=!/*IE%X#$&@W";
            Serial.printf("%c", set[int(value / maxValue * sizeof(set))]);
        }
        Serial.printf("\n");
    }
}

void Epepd::setRotation(uint8_t r) {
    rotation = r;
}

uint8_t Epepd::getRotation(void) const {
    return rotation;
}

