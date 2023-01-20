#include "EpBitmap.h"

#define SHOW_HEAP_INFO

EpBitmap::EpBitmap(int16_t w, int16_t h, uint8_t bitsPerPixel) :
        Adafruit_GFX(w, h) {
    if (bitsPerPixel > 8)
        Serial.printf("You can not create a bitmap with bpp over 8 yet\n");
    WIDTH = w;
    HEIGHT = h;
    BPP = bitsPerPixel;
}

EpBitmap::~EpBitmap() {
    if (allocated)
        deallocate();
}

bool EpBitmap::allocate(uint32_t bs) {
    if (blendMode == SHAPES_ONLY)
        Serial.printf("Warning: Allocating bitmap buffer when shapes blendMode is selected.\n");
#ifdef SHOW_HEAP_INFO
    Serial.printf("Before allocation:\n");
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif
    blockSizeExp = 31 - __builtin_clzl(bs | 1);
    blockSize = 1 << blockSizeExp;
    uint32_t requiredBytes = uint32_t(WIDTH) * uint32_t(HEIGHT) * BPP / 8;
    uint16_t fullBlocksCnt = requiredBytes / blockSize;
    uint16_t remainderBlockSize = requiredBytes - uint32_t(fullBlocksCnt) * blockSize;
    blocks = (uint8_t**) (heap_caps_malloc((fullBlocksCnt + (remainderBlockSize ? 1 : 0)) * sizeof(uint8_t*), MALLOC_CAP_32BIT));
    if (blocks == nullptr) {
        Serial.printf("Failed to allocate memory!\n");
        return false;
    }
    Serial.printf("head at %p\n", blocks);

    // allocate full blocks
    for (uint16_t b = 0; b < fullBlocksCnt; b++) {
        blocks[b] = (uint8_t*) (heap_caps_malloc(blockSize, MALLOC_CAP_8BIT));
        if (blocks[b] == nullptr) {
            Serial.printf("Failed to allocate memory!\n");
            for (uint16_t fb = 0; fb < b; fb++)
                heap_caps_free(blocks[fb]);
            heap_caps_free(blocks);
            return false;
        }
        Serial.printf("block %d at %p\n", b, blocks[b]);
    }
    // allocate remainder block
    if (remainderBlockSize) {
        blocks[fullBlocksCnt] = (uint8_t*) (heap_caps_malloc(remainderBlockSize, MALLOC_CAP_8BIT));
        if (blocks[fullBlocksCnt] == nullptr) {
            Serial.printf("Failed to allocate memory!\n");
            for (uint16_t fb = 0; fb < fullBlocksCnt; fb++)
                heap_caps_free(blocks[fb]);
            heap_caps_free(blocks);
            return false;
        }
        Serial.printf("block %d at %p\n", fullBlocksCnt, blocks[fullBlocksCnt]);
    }

#ifdef SHOW_HEAP_INFO
    Serial.printf("After allocation:\n");
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif
    Serial.printf("Allocated in total %d bytes of memory in %d %d byte chunks\n",
                  (fullBlocksCnt + (remainderBlockSize ? 1 : 0)) * sizeof(uint8_t*) + fullBlocksCnt * blockSize + remainderBlockSize,
                  (fullBlocksCnt + (remainderBlockSize ? 1 : 0)),
                  blockSize);
    Serial.printf("Remainder block: block %d size %d bytes\n", fullBlocksCnt, remainderBlockSize);
    allocated = true;
    return true;
}

void EpBitmap::deallocate() {
    if (!allocated) {
        Serial.printf("Error: Deallocating bitmap buffer when it is not allocated.\n");
        return;
    }
    uint32_t blockCnt = (uint32_t(WIDTH) * uint32_t(HEIGHT) * BPP / 8 + blockSize - 1) / blockSize;
    for (uint16_t b = 0; b < (blockCnt); b++) {
        heap_caps_free(blocks[b]);
    }
    heap_caps_free(blocks);
    allocated = false;
    return;
}

uint8_t EpBitmap::getPixel(int16_t x, int16_t y) { // I hope I were better at optimization... this will get called a lot
//    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? ((((x >> 3) & 1) != ((y >> 3) & 1)) ? 0b10000000 : 0b01000000) : this->transparencyColor;
    // rotated 45 degs and offset by some amount for better clarity
//    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? ((((((x + 7) - y) >> 2) + (((x + 7) - y) >> 4) & 1) != (((x + (y + 3)) >> 2) + ((x + (y + 3)) >> 4) & 1))
//                                                                         ? 0b10000000 : 0b01000000)
//                                                                      : this->transparencyColor;
    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? (
            (
                    ((x & 0b11) == 0) && ((y & 0b11) == 1) ||
                    ((x & 0b11) == 1) && ((y & 0b11) == 0) ||
                    ((x & 0b11) == 2) && ((y & 0b11) == 2) ||
                    ((x & 0b11) == 3) && ((y & 0b11) == 3)
            )
            ? 0b01000000 : 0b10000000) : this->transparencyColor;

    uint16_t shapesColor;
    uint8_t bitmapColor;
    if (blendMode != BITMAP_ONLY)
        shapesColor = getShapePixel(x, y);
    if (blendMode != SHAPES_ONLY)
        bitmapColor = getBitmapPixel(x, y);
    uint8_t color = transparencyColor;
    switch (blendMode) {
        case BITMAP_ONLY:
            return bitmapColor;
        case SHAPES_ONLY:
            return (shapesColor == 0xFFFF) ? transparencyColor : shapesColor;
        case BITMAP_ADD_SHAPES:
            return (shapesColor == 0xFFFF) ? bitmapColor : shapesColor;
        case BITMAP_SUBTRACT_SHAPES:
            return (shapesColor == 0xFFFF) ? bitmapColor : transparencyColor;
        case BITMAP_INTERSECT_SHAPES:
            return (shapesColor == 0xFFFF) ? transparencyColor : bitmapColor;
        case SHAPES_ADD_BITMAP:
            return (bitmapColor == transparencyColor) ? ((shapesColor == 0xFFFF) ? transparencyColor : shapesColor) : bitmapColor;
        case SHAPES_SUBTRACT_BITMAP:
            return (bitmapColor == transparencyColor) ? ((shapesColor == 0xFFFF) ? transparencyColor : shapesColor) : transparencyColor;
        case SHAPES_INTERSECT_BITMAP:
            return (bitmapColor == transparencyColor) ? transparencyColor : ((shapesColor == 0xFFFF) ? transparencyColor : shapesColor);
    }
}

void EpBitmap::setPixel(int16_t x, int16_t y, uint8_t color) {
    switch (adafruitGfxMode) {
        case ON_BITMAP:
            return setBitmapPixel(x, y, color);
        case ON_SHAPES:
            if (shapes.size() == 99) Serial.printf("Oh my god there's 100 shapes, your display will take forever to update\n");
            return setRectangle(x, y, 1, 1, color, EpShape::ADD);
        case ON_VISIBLE:
            switch (blendMode) {
                case SHAPES_ONLY:
                case BITMAP_ADD_SHAPES:
                case SHAPES_SUBTRACT_BITMAP: // these as well, but nah let's not add "getPixel" such slow function to the mix
                case SHAPES_INTERSECT_BITMAP:
                    if (shapes.size() == 99) Serial.printf("Oh my god there's 100 shapes, your display will take forever to update\n");
                    return setRectangle(x, y, 1, 1, color, EpShape::ADD);
                case BITMAP_ONLY:
                case BITMAP_SUBTRACT_SHAPES:
                case BITMAP_INTERSECT_SHAPES:
                case SHAPES_ADD_BITMAP: // technically if bitmap already transparent, we should set shape, but that is going to be confusing. you are only supposed to get BITMAP_ONLY, I'm already over-delivering
                default:
                    return setBitmapPixel(x, y, color);
            }
    }
}

uint8_t EpBitmap::getBitmapPixel(int16_t x, int16_t y) {
    if (!allocated) {
        Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
        return transparencyColor;
    }
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return transparencyColor;
    int32_t bitIdx = (int32_t(y) * WIDTH + x + 1) * BPP - 1; // of the last bit of the pixel
    uint16_t joined = (blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)];
    if ((bitIdx >> 3) >= BPP) // not the first byte
        joined |= ((blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)]) << 8;
    joined >>= 7 - (bitIdx & 0b111);
    joined &= (1 << BPP) - 1; // mask
    return (joined << (8 - BPP)) | (_nextBitmap ? (_nextBitmap->getBitmapPixel(x, y) >> BPP) : 0);
}

void EpBitmap::setBitmapPixel(int16_t x, int16_t y, uint8_t color) {
    if (!allocated) {
        Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
        return;
    }
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    color >>= (8 - BPP); // align right (and trim excess)
    uint32_t bitIdx = (uint32_t(y) * WIDTH + x + 1) * BPP - 1; // of the last bit of the pixel
    uint16_t mask = ((uint16_t(1) << BPP) - 1) << (7 - (bitIdx & 0b111));
    (blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)] &= ~mask; // reset
    (blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)] |= color << (7 - (bitIdx & 0b111));
    if ((bitIdx & 0b111) + 1 < BPP) { // cross byte boundary
        (blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)] &= ~(mask >> 8); // reset
        (blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)] |= color >> ((bitIdx & 0b111) + 1);
    }
    if (_nextBitmap)
        _nextBitmap->setBitmapPixel(x, y, color << BPP);
}

bool EpBitmap::isAllocated() const {
    return allocated;
}

void EpBitmap::clearShapes() {
    shapes.clear();
}

void EpBitmap::setRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color, EpShape::Operation operation) {
    shapes.push_back(EpShape(EpShape::RECTANGLE, operation, x, y, w, h, color));
}

void EpBitmap::setCircle(int16_t x, int16_t y, int16_t diameter, uint8_t color, EpShape::Operation operation) {
    shapes.push_back(EpShape(EpShape::CIRCLE, operation, x, y, diameter - 1, diameter - 1, color));
}

bool EpShape::intersect(int16_t a, int16_t b) {
    switch (shape) {
        case RECTANGLE:
            return (a >= x && a <= x + w - 1 && b >= y && b <= x + w - 1);
        case CIRCLE: // adafruit seems to do things differently... will that work?
            return ((float(a) - x) * (float(a) - x) + (float(b) - y) * (float(b) - y)) <= (float(w) / 2.f) * (float(w) / 2.f);
    }
}

uint16_t EpBitmap::getShapePixel(int16_t x, int16_t y) {
    bool isTransparent = true;
    uint8_t currColor = 0xFFFF;
    for (auto &shape: shapes) {
        if (shape.intersect(x, y)) {
            switch (shape.operation) {
                case EpShape::ADD:
                    isTransparent = false;
                    currColor = shape.color;
                    break;
                case EpShape::SUBTRACT:
                    isTransparent = true;
                    break;
                case EpShape::INTERSECT:
                    if (!isTransparent)
                        currColor = shape.color;
                    break;
                case EpShape::INTERSECT_USE_BEHIND:
                    break;
                case EpShape::EXCLUDE:
                    if (!isTransparent) // overlapping area
                        isTransparent = true;
                    else { // new shape only
                        isTransparent = false;
                        currColor = shape.color;
                    }
            }
        }
        else {
            switch (shape.operation) {
                case EpShape::ADD:
                case EpShape::SUBTRACT:
                case EpShape::EXCLUDE:
                    break;
                case EpShape::INTERSECT:
                case EpShape::INTERSECT_USE_BEHIND:
                    isTransparent = true;
                    break;
            }
        }
    }
    return isTransparent ? 0xFFFF : currColor;
}

void EpBitmap::setBitmapShapeBlendMode(EpBitmap::BitmapShapeBlendMode mode) {
    blendMode = mode;
}

void EpBitmap::setAdafruitGFXTargetMode(AdafruitGfxTargetMode mode) {
    adafruitGfxMode = mode;
}

void EpBitmap::setTransparencyColor(uint8_t color) {
    transparencyColor &= ~(1 << 15);
    transparencyColor = color;
}

uint8_t EpBitmap::getTransparencyColor() {
    return transparencyColor;
}

void EpBitmap::setTransparencyPattern() {
    transparencyColor |= (1 << 15);
}

void EpBitmap::_streamBytesInBegin() {
    streamBytesInCurrentBlock = 0;
    streamBytesInCurrentByte = 0;
}

void EpBitmap::_streamBytesOutBegin() {
    streamBytesOutCurrentBlock = 0;
    streamBytesOutCurrentByte = 0;
}

void EpBitmap::_streamInBytesNext(uint8_t byte) {
    (blocks[streamBytesInCurrentBlock])[streamBytesInCurrentByte] = byte;
    streamBytesInCurrentByte++;
    if (streamBytesInCurrentByte == blockSize) {
        streamBytesInCurrentBlock++;
        streamBytesInCurrentByte = 0;
    }
}

uint8_t EpBitmap::_streamOutBytesNext() {
    uint8_t data = (blocks[streamBytesOutCurrentBlock])[streamBytesOutCurrentByte];
    streamBytesOutCurrentByte++;
    if (streamBytesOutCurrentByte == blockSize) {
        streamBytesOutCurrentBlock++;
        streamBytesOutCurrentByte = 0;
    }
    return data;
}

void EpBitmap::_linkBitmap(EpBitmap* nextBitmap) {
    _nextBitmap = nextBitmap;
}


uint8_t EpBitmap::getLuminance(uint16_t color) {
#ifdef EPEPD_USE_PERCEIVED_LUMINANCE
    float r = float(color & 0b1111100000000000);
    float g = float(color & 0b0000011111100000 << 5);
    float b = float(color & 0b0000000000011111 << 11);
    return std::min(uint16_t((0.299f * r * r + 0.587f * g * g + 0.114f * b * b) * 65536.f), uint16_t(0xFFFF));
#else
    /// TODO: wtf how would this work
    return (color & 0b1111100000000000) + (color & 0b0000011111100000 << 5) + (color & 0b0000000000011111 << 11);
#endif
}

void EpBitmap::drawPixel(int16_t x, int16_t y, uint16_t color) {
    setPixel(x, y, getLuminance(color));
}
