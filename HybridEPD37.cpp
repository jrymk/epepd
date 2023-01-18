#include "HybridEPD37.h"

#define BUSY_TIMEOUT 5000000
#define RESET_DURATION 10

const unsigned char HybridEPD37::lut_4G[] PROGMEM =
        {
//                ph0    1     2     3     4     5     6     7     8     9
                0x40, 0x48, 0x48, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut00
                0x08, 0x48, 0x48, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut01
                0x02, 0x48, 0x48, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut10
                0x20, 0x48, 0x48, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // vs lut11
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

HybridEPD37::HybridEPD37(int16_t csPin, int16_t dcPin, int16_t rstPin, int16_t busyPin) :
        Adafruit_GFX(WIDTH, HEIGHT),
        spi(&SPI),
        spiSettings(40000000, MSBFIRST, SPI_MODE0) {
    this->csPin = csPin;
    this->dcPin = dcPin;
    this->rstPin = rstPin;
    this->busyPin = busyPin;
}

void HybridEPD37::init() {
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    pinMode(dcPin, OUTPUT);
    digitalWrite(dcPin, HIGH);
    hwReset();
    pinMode(busyPin, INPUT);
    spi->begin();
}

void HybridEPD37::displayTest() {
    delay(100);
    initDisplay();
    writeCommand(0x24);
    writeDataBegin();
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    writeDataEnd();

    writeCommand(0x26);
    writeDataBegin();
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0x00);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
        writeDataCont(0xFF);
    writeDataEnd();

    writeCommand(0x22);
    writeData(0xcf); // display mode 2 with ping pong

    writeCommand(0x20); // activate
    waitUntilIdle();
}

void HybridEPD37::initDisplay() {
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

    setRamWindow(0, 0, WIDTH, HEIGHT);

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

void HybridEPD37::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= width() || y < 0 || y >= height())
        return;
    switch (getRotation()) {
        case 1:
            std::swap(x, y);
            x = WIDTH - x - 1;
            break;
        case 2:
            x = WIDTH - x - 1;
            y = HEIGHT - y - 1;
            break;
        case 3:
            std::swap(x, y);
            y = HEIGHT - y - 1;
            break;
    }
    uint16_t i = (x >> 1) + y * (width() >> 1);
    uint16_t pixelData = getLuminance(color) >> 12; // take the last 4 bits
    buffer[i] &= ~(0xF << ((1 - (x & 1)) * 4));
    buffer[i] |= (pixelData << ((1 - (x & 1)) * 4));
}


void HybridEPD37::display() {
    initDisplay();
//    debugPrintBuffer(buffer, 4);
    writeBufferToMemory([](HybridEPD37 &buff, int16_t x, int16_t y) {
//        Serial.printf("calc lut %d, %d is %d\n", x, y, (HybridEPD37::getBufferPixel(buff.buffer, 4, x, y)) ? 0b11 : 0b00);
//        return (HybridEPD37::getBufferPixel(buff.buffer, x, y)) ? 0b11 : 0b00;
        return (((x & 0b1000) + (y & 0b1000)) & 0b1000 ? 0b01 : 0b10);
    });

    writeCommand(0x22);
    writeData(0xCF); // display mode 2 with ping pong

    writeCommand(0x20); // activate
    waitUntilIdle();
}

uint16_t HybridEPD37::getLuminance(uint16_t color) {
#ifdef USE_PERCEIVED_LUMINANCE
    float r = float(color & 0b1111100000000000);
    float g = float(color & 0b0000011111100000 << 5);
    float b = float(color & 0b0000000000011111 << 11);
    return std::min(uint16_t((0.299f * r * r + 0.587f * g * g + 0.114f * b * b) * 65536.f), uint16_t(0xFFFF));
#else
    return (color & 0b1111100000000000) + (color & 0b0000011111100000 << 5) + (color & 0b0000000000011111 << 11);
#endif
}

void HybridEPD37::writeBufferToMemory(std::function<uint8_t(HybridEPD37 &buff, int16_t x, int16_t y)> bufferToLUT) {
    uint64_t start = esp_timer_get_time();
    writeCommand(0x26);
    writeDataBegin();
    for (int32_t y = 0; y < HEIGHT; y++) {
        for (int32_t xB = 0; xB < WIDTH; xB += 8) {
            uint8_t data = 0;
            for (int8_t b = 0; b < 8; b++) {
                uint8_t lut = bufferToLUT(*this, int16_t(xB + b), int16_t(y));
                data |= ((lut & 0b10) ? (1 << (7 - ((y * WIDTH + xB + b) & 0b111))) : 0);
            }
            writeDataCont(data);
        }
    }
    writeDataEnd();

    writeCommand(0x24);
    writeDataBegin();
    for (int32_t y = 0; y < HEIGHT; y++) {
        for (int32_t xB = 0; xB < WIDTH; xB += 8) {
            uint8_t data = 0;
            for (int8_t b = 0; b < 8; b++) {
                uint8_t lut = bufferToLUT(*this, int16_t(xB + b), int16_t(y));
                data |= ((lut & 0b01) ? (1 << (7 - ((y * WIDTH + xB + b) & 0b111))) : 0);
            }
            writeDataCont(data);
        }
    }
    writeDataEnd();

    Serial.printf("Sending two sets of display buffer took %lldus\n", esp_timer_get_time() - start);
}

uint8_t HybridEPD37::getBufferPixel(uint8_t* buffer, uint16_t x, uint16_t y) {
    return buffer[uint32_t(y) * WIDTH + x];
};

uint8_t HybridEPD37::getBufferPixel(uint8_t* buffer, uint8_t bitsPerPixel, uint16_t x, uint16_t y) {
    return (buffer[(uint32_t(y) * WIDTH + uint32_t(x)) * bitsPerPixel / 8] >> (bitsPerPixel - (uint32_t(y) * WIDTH + x) * bitsPerPixel % 8)) & ((1 << bitsPerPixel) - 1);
}

void HybridEPD37::writeCommand(uint8_t c) {
    spi->beginTransaction(spiSettings);
    digitalWrite(dcPin, LOW);
    digitalWrite(csPin, LOW);
    spi->transfer(c);
    digitalWrite(csPin, HIGH);
    digitalWrite(dcPin, HIGH);
    spi->endTransaction();
}

void HybridEPD37::writeDataBegin() {
    if (isWritingData)
        Serial.printf("ERROR: already writing data, did you forgot to end?\n");
    spi->beginTransaction(spiSettings);
    digitalWrite(csPin, LOW);
    isWritingData = true;
}

void HybridEPD37::writeData(uint8_t d) {
    writeDataBegin();
    writeDataCont(d);
    writeDataEnd();
}

void HybridEPD37::writeDataCont(uint8_t d) {
    spi->transfer(d);
}

void HybridEPD37::writeDataEnd() {
    digitalWrite(csPin, HIGH);
    spi->endTransaction();
    isWritingData = false;
}

void HybridEPD37::waitUntilIdle() {
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

void HybridEPD37::setRamWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
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

void HybridEPD37::hwReset() {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(RESET_DURATION);
    digitalWrite(rstPin, LOW);
    delay(RESET_DURATION);
    digitalWrite(rstPin, HIGH);
    delay(RESET_DURATION);
    isHibernating = false;
}

void HybridEPD37::hibernate() {
    powerOff();
    writeCommand(0x10); // deep sleep mode
    writeData(0x03); // enter deep sleep
    isHibernating = true;
}

void HybridEPD37::powerOn() {
    if (!isPoweredOn) {
        writeCommand(0x22);
        writeData(0xc0); // enable clock signal -> enable analog

        writeCommand(0x20);
        waitUntilIdle();
    }
    isPoweredOn = true;
}

void HybridEPD37::powerOff() {
    if (isPoweredOn) {
        writeCommand(0x22);
        writeData(0x03); // disable analog -> disable clock signal

        writeCommand(0x20);
        waitUntilIdle();
    }
    isPoweredOn = false;
}

void HybridEPD37::debugPrintBuffer(uint8_t* buffer, uint8_t bitsPerPixel) {
    for (uint16_t y = 0; y < HEIGHT; y++) {
        for (uint16_t x = 0; x < WIDTH; x++) {
            float value = getBufferPixel(buffer, bitsPerPixel, x, y);
//            Serial.printf("pixel %d %d is %d\n", x, y, getBufferPixel(buffer, bitsPerPixel, x, y));
            float maxValue = float(int(1 << bitsPerPixel));
            char set[] = "'-.,^=!/*IE%X#$&@W";
            Serial.printf("%c", set[int(value / maxValue * sizeof(set))]);
        }
        Serial.printf("\n");
    }
}

