#include "EpBitmap.h"

EpBitmap::EpBitmap(int16_t w, int16_t h, uint8_t bitsPerPixel) {
    if (bitsPerPixel > 8) {
        Serial.printf("You can not create a bitmap with bpp over 8 yet\n");
    }
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
    printf("head at %p\n", blocks);
    if (blocks == nullptr) {
        Serial.printf("Failed to allocate memory!\n");
        return false;
    }

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
        printf("block %d at %p\n", b, blocks[b]);
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
        printf("block %d at %p\n", fullBlocksCnt, blocks[fullBlocksCnt]);
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
    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? ((((x >> 3) & 1) != ((y >> 3) & 1)) ? 0b10000000 : 0b01000000) : this->transparencyColor;
    uint16_t shapesColor;
    uint8_t bitmapColor;
    if (blendMode != BITMAP_ONLY)
        shapesColor = getShapePixel(x, y);
    if (blendMode != SHAPES_ONLY) { /// TODO: force block size power of 2 to get rid of div and mod
        if (!allocated) {
            Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
            return transparencyColor;
        }
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return transparencyColor;
        int32_t bitIdx = (int32_t(y) * WIDTH + x + 1) * BPP - 1; // of the last bit of the pixel
        uint16_t joined = (blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)];
        if ((bitIdx >> 3) >= blockSize) // not the first byte
            joined |= ((blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)]) << 8;
        joined >>= 7 - (bitIdx & 0b111);
        joined &= (1 << BPP) - 1; // partialUpdateMask
        bitmapColor = (joined << (8 - BPP));
    }
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
}

int16_t EpBitmap::width() const {
    return WIDTH;
}

int16_t EpBitmap::height() const {
    return HEIGHT;
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

void EpBitmap::_streamBytesBegin() {
    streamBytesCurrentBlock = 0;
    streamBytesCurrentByte = 0;
}

uint8_t EpBitmap::_streamOutBytesNext() {
    uint8_t data = (blocks[streamBytesCurrentBlock])[streamBytesCurrentByte];
    streamBytesCurrentByte++;
    if (streamBytesCurrentByte == blockSize) {
        streamBytesCurrentBlock++;
        streamBytesCurrentByte = 0;
    }
    return data;
}

void EpBitmap::_streamInBytesNext(uint8_t byte) {
    (blocks[streamBytesCurrentBlock])[streamBytesCurrentByte] = byte;
    streamBytesCurrentByte++;
    if (streamBytesCurrentByte == blockSize) {
        streamBytesCurrentBlock++;
        streamBytesCurrentByte = 0;
    }
}
