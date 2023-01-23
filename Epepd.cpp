#include "Epepd.h"

#define EPEPD_USE_PERCEIVED_LUMINANCE
#define EPEPD_BUSY_TIMEOUT 5000000

// default LUT
const uint8_t Epepd::lut_GC4[] PROGMEM = {
        // 00: VCOM, 01: 15V, 11: 5V, 10: -15V
        /* BLACK     */ 0b00101010, 0b00000110, 0b00010101, 0, 0, 0, 0, 0, 0, 0,
        /* LIGHTGREY */ 0b00100000, 0b00000110, 0b00010000, 0, 0, 0, 0, 0, 0, 0, // light grey has final bw = 1, better reflects the display color
        /* DARK GREY */ 0b00101000, 0b00000110, 0b00010100, 0, 0, 0, 0, 0, 0, 0, // dark grey has final bw = 0, better reflects the display color
        /* WHITE     */ 0b00010100, 0b00000110, 0b00101000, 0, 0, 0, 0, 0, 0, 0, /* VCOM */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* GROUP 1 */ 0, 8, 8, 40, /* REPEAT */ 0,
        /* GROUP 2 */ 0, 0, 32, 32, /* REPEAT */ 2,
        /* GROUP 3 */ 0, 8, 8, 40, /* REPEAT */ 0,
        /* GROUP 4 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 5 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 6 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 7 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 8 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP 9 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* GROUP10 */ 0, 0, 0, 0, /* REPEAT */ 0,
        /* FRAMERATE */ 0x88, 0x88, 0x88, 0x88, 0x88
};


Epepd::Epepd(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin) :
        redRam(EPD_WIDTH, EPD_HEIGHT),
        bwRam(EPD_WIDTH, EPD_HEIGHT),
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
    redRam.allocate();
    bwRam.allocate();
    Serial.printf("[epepd] Display resolution: %d*%d\n", EPD_WIDTH, EPD_HEIGHT);
}

void Epepd::initDisplay() {
    if (waitingForUpdateCompletion) {
        uint64_t start = esp_timer_get_time();
        waitUntilIdle();
        waitingForUpdateCompletion = false;
        Serial.printf("[epepd] EpGreyscaleDisplay waited %lldus while display updating\n", esp_timer_get_time() - start);

    }

    uint64_t start = esp_timer_get_time();
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
        writeData(0x41); // VSH1 = 15V (01)
        writeData(0xA8); // VSH2 = 5V (11)
        writeData(0x32); // VSL = -15V (10)

        writeCommand(0x0C); // Booster Soft-start Control
        writeData(0xAE);
        writeData(0xC7);
        writeData(0xC3);
        writeData(0xC0);
        writeData(0x40); // level 1?

        setBorder(BORDER_LUT0);

        writeCommand(0x21); // display update control
        writeData(0x00); // normal

        writeCommand(0x18); // temperature sensor control
        writeData(0x80); // internal temperature sensor

        writeCommand(0x2C); // write VCOM register
//        writeData(0x44); // -1.7V (Waveshare example)
        writeData(0x00); // 0V   (for partial refresh, source voltage VS is VSS, VCOM VS is DCVCOM, which should stay at 0 as well
//        writeData(0x1C); // -0.7V

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

    powerOn();
    // remember to call writeLUT!
    Serial.printf("[epepd] Init display took %lldus\n", esp_timer_get_time() - start);
}

void Epepd::writeLUT(const uint8_t* data) {
    if (waitingForUpdateCompletion) { // from experimentation, it seems that writing LUT requires BUSY flag = 0
        waitUntilIdle();
        waitingForUpdateCompletion = false;
    }

    uint64_t start = esp_timer_get_time();
    if (data == nullptr)
        data = lut_GC4;

    writeCommand(0x32); // write lut register
    writeDataBegin();
    for (uint16_t i = 0; i < LUT_SIZE; i++) {
        writeDataCont(pgm_read_byte(data + i));
//        Serial.printf("%2X ", pgm_read_byte(data + i));
    }
    writeDataEnd();

    powerOn();
    Serial.printf("[epepd] Write LUT took %lldus\n", esp_timer_get_time() - start);
}

void Epepd::setBorder(BorderStyle border) {
    writeCommand(0x3C);
    switch (border) {
        case BORDER_HI_Z:
            return writeData(0b11000000);
        case BORDER_LUT0:
            return writeData(0b00000000);
        case BORDER_LUT1:
            return writeData(0b00000001);
        case BORDER_LUT2:
            return writeData(0b00000010);
        case BORDER_LUT3:
            return writeData(0b00000011);
    }
}

void Epepd::writeToDisplay() {
    if (waitingForUpdateCompletion) { // from experimentation, it seems that writing to display requires BUSY flag = 0
        waitUntilIdle();
        waitingForUpdateCompletion = false;
    }
    uint64_t start = esp_timer_get_time();
    uint32_t size = uint32_t(EPD_WIDTH) * uint32_t(EPD_HEIGHT) / 8;
    writeCommand(0x26);
    writeDataBegin();
    uint8_t* byte = redRam._getBuffer();
    for (uint32_t b = 0; b < size; b++)
        writeDataCont(*(byte++));
    writeDataEnd();

    writeCommand(0x24);
    writeDataBegin();
    byte = bwRam._getBuffer();
    for (uint32_t b = 0; b < size; b++)
        writeDataCont(*(byte++));
    writeDataEnd();

    Serial.printf("[epepd] Sending two sets of display buffer took %lldus\n", esp_timer_get_time() - start);
}

void Epepd::updateDisplay() {
    if (waitingForUpdateCompletion) {
        waitUntilIdle();
        waitingForUpdateCompletion = false;
    }

    uint64_t start = esp_timer_get_time();
    writeCommand(0x22);
    writeData(0xCF); // display mode 2

    writeCommand(0x20); // activate
//    waitUntilIdle(); // why would I wait for it? I could update the next frame in the meantime!
    waitingForUpdateCompletion = true;
    Serial.printf("[epepd] Display update took %lldus\n", esp_timer_get_time() - start);
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
        if (waitingForUpdateCompletion) {
            waitUntilIdle();
            waitingForUpdateCompletion = false;
        }

        writeCommand(0x22);
        writeData(0x83); // disable analog -> disable clock signal
        /*
         * Operating sequence Parameter      (in Hex)
            Enable clock signal                    80
            Disable clock signal                   01
            Enable clock signal -> Enable Analog   C0
            Disable Analog -> Disable clock signal 03

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
    writeData(0x03); // originX increment, y increment, updated on originX direction

    writeCommand(0x44); // set ram originX address start/end position
    writeData(x & 0xFF);
    writeData(x >> 8);
    writeData((x + w - 1) & 0xFF); // from 0 to 439 is (originX = 0, w = 480)
    writeData((x + w - 1) >> 8);

    writeCommand(0x45); // set ram y address start/end position
    writeData(y & 0xFF);
    writeData(y >> 8);
    writeData((y + h - 1) & 0xFF);
    writeData((y + h - 1) >> 8);

    writeCommand(0x4e); // set ram originX address counter
    writeData(x & 0xFF);
    writeData(x >> 8);

    writeCommand(0x4f); // set ram y address counter
    writeData(y & 0xFF);
    writeData(y >> 8);
}


EpBitmapMono* Epepd::getRedRam() {
    // no need to unpair, just waste slightly more time
//    redRam._linkBitmap(nullptr);
    return &redRam;
}

EpBitmapMono* Epepd::getBwRam() {
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

