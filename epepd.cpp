#include "Epepd.h"

#define EPEPD_USE_PERCEIVED_LUMINANCE
#define EPEPD_BUSY_TIMEOUT 5000000

const unsigned char Epepd::lut_4G[] PROGMEM =
        {
                // 1G GC MOD (one flash, fastest, looks pretty good)
                0x2A, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//1
                0x05, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//2
                0x2A, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//3
                0x05, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//4
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//5
                0x00, 0x01, 0x01, 0x03, 0x00,
                0x01, 0x03, 0x03, 0x01, 0x00,//6
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00,//7
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00,//8
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00,//9
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00,//10
                0x22, 0x22, 0x22, 0x22, 0x22
        };


Epepd::Epepd(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin) :
        redRam(EPD_WIDTH, EPD_HEIGHT, 1),
        bwRam(EPD_WIDTH, EPD_HEIGHT, 1),
        spi(&SPI), // GxEPD2 uses 4Mhz, Waveshare example 2Mhz, we're using 40Mhz! (tbh it's barely faster)
        spiSettings(40000000, MSBFIRST, SPI_MODE0) {
    this->csPin = csPin;
    this->dcPin = dcPin;
    this->rstPin = rstPin;
    this->busyPin = busyPin;
}

void Epepd::init() {
    Serial.printf("Start init\n");
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    pinMode(dcPin, OUTPUT);
    digitalWrite(dcPin, HIGH);
    hwReset();
    pinMode(busyPin, INPUT);
    spi->begin();
    redRam.setBitmapShapeBlendMode(EpBitmap::BITMAP_ONLY);
    bwRam.setBitmapShapeBlendMode(EpBitmap::BITMAP_ONLY);
    redRam.allocate(4096);
    bwRam.allocate(4096);
    redRam._linkBitmap(&bwRam); // remaining bits will be sent to bwRam
    Serial.printf("[epepd] Display resolution: %d*%d\n", EPD_WIDTH, EPD_HEIGHT);
}

void Epepd::initDisplay() {
    if (isHibernating) {
        hwReset();
        delay(10);
        writeCommand(0x12); // sw reset
        delay(10);
        waitUntilIdle();

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
        writeData(0x0f); // enable ping pong for display mode 2 (4F = on ; 0F = off)
        writeData(0xff);
        writeData(0xff);
        writeData(0xff);
        writeData(0xff);

        isHibernating = false;
    }

    setRamWindow(0, 0, EPD_WIDTH, EPD_HEIGHT);

    // remember to call writeLUT!
}

void Epepd::writeLUT(const uint8_t* data, uint16_t size) {
    if (data == nullptr) {
        data = lut_4G;
        size = sizeof(lut_4G);
    }
    writeCommand(0x3C); // Border Waveform Control /// TODO: figure out
    writeData(0x01); // LUT1, for white

    writeCommand(0x32); // write lut register
    writeDataBegin();
    for (uint16_t i = 0; i < size; i++) {
        writeDataCont(pgm_read_byte(data + i));
//        Serial.printf("%2X ", pgm_read_byte(data + i));
    }
    writeDataEnd();

    powerOn();
}

void Epepd::writeToDisplay() {
    Serial.printf("[epepd] Writing to display\n");
    uint64_t start = esp_timer_get_time();
    uint32_t size = uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 8;
    writeCommand(0x26);
    writeDataBegin();
    redRam._streamBytesOutBegin();
    for (uint32_t b = 0; b < size; b++)
        writeDataCont(redRam._streamOutBytesNext());
    writeDataEnd();

    writeCommand(0x24);
    writeDataBegin();
    bwRam._streamBytesOutBegin();
    for (uint32_t b = 0; b < size; b++)
        writeDataCont(bwRam._streamOutBytesNext());
    writeDataEnd();

    Serial.printf("[epepd] Sending two sets of display buffer took %lldus\n", esp_timer_get_time() - start);
}

void Epepd::updateDisplay() {
    writeCommand(0x22);
    writeData(0xCF); // display blendMode 2

    writeCommand(0x20); // activate
    waitUntilIdle();
}

void Epepd::hwReset() {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(20);
    digitalWrite(rstPin, LOW);
    delay(2);
    digitalWrite(rstPin, HIGH);
    delay(20);
}

void Epepd::hibernate() {
    powerOff();
    writeCommand(0x10); // deep sleep blendMode
    writeData(0x03); // enter deep sleep
    isHibernating = true;
}

void Epepd::powerOn() {
    if (isHibernating)
        return initDisplay(); // you probably shouldn't go this path
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
        writeData(0x83); // disable analog -> disable clock signal
        /*
         * Operating sequence Parameter  (in Hex)
            Enable clock signal                   80
            Disable clock signal                  01
            Enable clock signal " Enable Analog   C0
            Disable Analog " Disable clock signal 03

           Not sure where 0x83 came from (got from GxEPD2), but with 0x03 it will busy timeout
           Maybe I'm reading the wrong datasheet? Maybe it isn't actually sleeping (although the wake command works after powering off)
         */
        writeCommand(0x20);
        waitUntilIdle();
    }
    isPoweredOn = false;
}

void Epepd::setRamWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    writeCommand(0x11); // data entry blendMode setting
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


EpBitmap* Epepd::getDisplayRam() {
    redRam._linkBitmap(&bwRam);
    return &redRam;
}

EpBitmap* Epepd::getRedRam() {
    // no need to unpair, just waste slightly more time
//    redRam._linkBitmap(nullptr);
    return &redRam;
}

EpBitmap* Epepd::getBwRam() {
    return &bwRam;
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
    spi->beginTransaction(spiSettings);
    digitalWrite(csPin, LOW);
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
}

void Epepd::waitUntilIdle() {
    uint64_t start = esp_timer_get_time();
    while (true) {
        delay(1);
        if (digitalRead(busyPin) != HIGH) break;
        if (esp_timer_get_time() > start + EPEPD_BUSY_TIMEOUT) {
            Serial.printf("[epepd] Busy timed out\n");
            break;
        }
        yield();
    }
//    Serial.printf("[epepd] Wait took %lldus\n", esp_timer_get_time() - start);
}

