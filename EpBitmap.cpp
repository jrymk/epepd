#include "EpBitmap.h"

#define SHOW_HEAP_INFO

constexpr const uint8_t EPBITMAP_PIXEL_MASK[9] = {0b10000000, 0b10000000, 0b11000000, 0b11100000, 0b11110000, 0b11111000, 0b11111100, 0b11111110, 0b11111111};

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

__attribute__((always_inline)) uint8_t EpBitmap::getPixel(int16_t x, int16_t y) {
//    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? ((((x >> 3) & 1) != ((y >> 3) & 1)) ? 0b10000000 : 0b01000000) : this->transparencyColor;
    // rotated 45 degs and offset by some amount for better clarity
//    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? ((((((x + 7) - y) >> 2) + (((x + 7) - y) >> 4) & 1) != (((x + (y + 3)) >> 2) + ((x + (y + 3)) >> 4) & 1))
//                                                                         ? 0b10000000 : 0b01000000)
//                                                                      : this->transparencyColor;

//    uint8_t transparencyColor = (this->transparencyColor & (1 << 15)) ? (
//            (
//                    (((x & 0b11) == 0) && ((y & 0b11) == 1)) ||
//                    (((x & 0b11) == 1) && ((y & 0b11) == 0)) ||
//                    (((x & 0b11) == 2) && ((y & 0b11) == 2)) ||
//                    (((x & 0b11) == 3) && ((y & 0b11) == 3))
//            )
//            ? 0b01000000 : 0b10000000) : this->transparencyColor;

    // most common modes optimization
    if (blendMode == BITMAP_ONLY)
        return getBitmapPixel(x, y);
    if (blendMode == SHAPES_ONLY) {
        uint16_t shapesColor = getShapePixel(x, y); // just read the first: 152556us
        return (shapesColor == 0xFFFF) ? transparencyColor : shapesColor;
    }

    uint8_t bitmapColor;
    uint16_t shapesColor;
    ///// takes about 60ms per typical usage update
    shapesColor = getShapePixel(x, y);
    ///// takes about 60ms per typical usage update
    bitmapColor = getBitmapPixel(x, y);

    switch (blendMode) {
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

__attribute__((always_inline)) void EpBitmap::setPixel(int16_t x, int16_t y, uint8_t color) {
    if (adafruitGfxMode == ON_BITMAP)
        return setBitmapPixel(x, y, color);
    if (adafruitGfxMode == ON_SHAPES) {
        if (shapes.size() == 99) Serial.printf("Oh my god there's 100 shapes, your display will take forever to update\n");
        return setRectangle(x, y, 1, 1, color, EpShape::ADD);
    }
    if (adafruitGfxMode == ON_VISIBLE) {
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

__attribute__((always_inline)) uint8_t EpBitmap::getBitmapPixel(uint32_t x, uint32_t y) {
    if (!allocated) {
        Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
        return transparencyColor;
    }
    if (x >= WIDTH || y >= HEIGHT) // use unsigned int to save x < 0 and y < 0
        return transparencyColor;
    uint32_t bitIdx = (y * WIDTH + x) * BPP; // of the first bit of the pixel
    uint16_t joined = ((blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)]) << 8;

    if ((bitIdx & 0b111) + BPP > 8) // cross byte boundary to the next byte
        joined |= ((blocks[((bitIdx >> 3) + 1) >> blockSizeExp])[((bitIdx >> 3) + 1) & ((1 << blockSizeExp) - 1)]);

    joined >>= (8 - (bitIdx & 0b111)); // (bitIdx & 0b111 bits)(bpp bits of data)(...)(2nd byte)
    joined &= EPBITMAP_PIXEL_MASK[BPP]; // mask

    /// nextBitmap will be deprecated, I don't even care
    uint8_t next = 0;
    if (_nextBitmap) {
        if (!_nextBitmap->allocated) {
            Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
            return transparencyColor;
        }
        uint16_t nextJoined = (_nextBitmap->blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)];
        if ((bitIdx >> 3) >= BPP) // not the first byte
            nextJoined |= ((_nextBitmap->blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)]) << 8;
        nextJoined >>= 7 - (bitIdx & 0b111);
        nextJoined &= (1 << BPP) - 1; // mask
        next = nextJoined << (8 - BPP);
    }

    return joined | (next >> BPP);
}

__attribute__((always_inline)) void EpBitmap::setBitmapPixel(uint32_t x, uint32_t y, uint8_t color) {
    if (!allocated) {
        Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
        return;
    }
    if (x >= WIDTH || y >= HEIGHT)
        return;
    uint32_t bitIdx = (y * WIDTH + x) * BPP; // of the first bit of the pixel
    uint8_t* byte = &((blocks[bitIdx >> 3 >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)]);
    *byte &= ~(EPBITMAP_PIXEL_MASK[BPP] >> (bitIdx & 0b111));
    *byte |= ((color & EPBITMAP_PIXEL_MASK[BPP]) >> (bitIdx & 0b111));

    if ((bitIdx & 0b111) + BPP > 8) { // cross byte boundary to the next byte
        uint8_t* byte2 = &((blocks[((bitIdx >> 3) + 1) >> blockSizeExp])[((bitIdx >> 3) + 1) & ((1 << blockSizeExp) - 1)]);
        *byte2 &= ~(EPBITMAP_PIXEL_MASK[BPP] << (8 - (bitIdx & 0b111)));
        *byte2 |= ((color & EPBITMAP_PIXEL_MASK[BPP]) << (8 - (bitIdx & 0b111)));
    }

    if (_nextBitmap) {
        if (!_nextBitmap->allocated) {
            Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
            return;
        }
        color <<= BPP;
        (_nextBitmap->blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)] &= ~EPBITMAP_PIXEL_MASK[BPP]; // reset
        (_nextBitmap->blocks[(bitIdx >> 3) >> blockSizeExp])[(bitIdx >> 3) & ((1 << blockSizeExp) - 1)] |= color << (7 - (bitIdx & 0b111));
        if ((bitIdx & 0b111) + 1 < BPP) { // cross byte boundary
            (_nextBitmap->blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)] &= ~(EPBITMAP_PIXEL_MASK[BPP] >> 8); // reset
            (_nextBitmap->blocks[((bitIdx >> 3) - 1) >> blockSizeExp])[((bitIdx >> 3) - 1) & ((1 << blockSizeExp) - 1)] |= color >> ((bitIdx & 0b111) + 1);
        }
    }
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

__attribute__((always_inline)) bool EpShape::intersect(int16_t a, int16_t b) {
    if (shape == RECTANGLE)
        return ((a >= x) && (a <= x + w - 1) && (b >= y) && (b <= y + h - 1));
    else if (shape == CIRCLE)
        return ((float(a) - x) * (float(a) - x) + (float(b) - y) * (float(b) - y)) <= (float(w) / 2.f) * (float(w) / 2.f);
//    switch (shape) {
//        case RECTANGLE:
//        case CIRCLE: // adafruit seems to do things differently... will that work?
//    }
}

__attribute__((always_inline)) uint16_t EpBitmap::getShapePixel(int16_t x, int16_t y) {
    uint16_t currColor = 0xFFFF;
    for (auto &shape: shapes) {
        if (shape.intersect(x, y)) {
            if (shape.operation == EpShape::ADD)
                currColor = shape.color;
            else if (shape.operation == EpShape::SUBTRACT)
                currColor = 0xFFFF;
            else if (shape.operation == EpShape::INTERSECT) {
                if (currColor != 0xFFFF)
                    currColor = shape.color;
            }
            else if (shape.operation == EpShape::EXCLUDE) {
                if (currColor != 0xFFFF) // overlapping area
                    currColor = 0xFFFF;
                else // new shape only
                    currColor = shape.color;
            }
        }
        else if (shape.operation == EpShape::INTERSECT || shape.operation == EpShape::INTERSECT_USE_BEHIND)
            currColor = 0xFFFF;
    }
    return currColor;
}

void EpBitmap::setBitmapShapeBlendMode(EpBitmap::BitmapShapeBlendMode mode) {
    blendMode = mode;
}

void EpBitmap::setAdafruitGFXTargetMode(AdafruitGfxTargetMode mode) {
    adafruitGfxMode = mode;
}

void EpBitmap::setTransparencyColor(uint8_t color) {
    transparencyColor &= ~(1 << 15);
    transparencyColor |= color;
}

uint8_t EpBitmap::getTransparencyColor() {
    return transparencyColor;
}

void EpBitmap::setTransparencyPattern() {
    transparencyColor |= (1 << 15);
}

uint8_t** EpBitmap::_getBlocks() {
    return blocks;
}

void EpBitmap::_streamBytesInBegin() {
    streamBytesInCurrentBlockIdx = 0;
    streamBytesInRemainingBytesInBlock = 0;
}

void EpBitmap::_streamBytesInNext(uint8_t byte) {
    if (!streamBytesInRemainingBytesInBlock) {
        streamBytesInByte = &(blocks[streamBytesInCurrentBlockIdx])[0];
        streamBytesInCurrentBlockIdx++;
        streamBytesInRemainingBytesInBlock = blockSize;
    }
    streamBytesInRemainingBytesInBlock--;
    *streamBytesInByte++ = byte;
}

void EpBitmap::_streamBytesOutBegin() {
    streamBytesOutCurrentBlockIdx = 0;
    streamBytesOutRemainingBytesInBlock = 0;
}


uint8_t EpBitmap::_streamBytesOutNext() {
    if (!streamBytesOutRemainingBytesInBlock) {
        streamBytesOutByte = &(blocks[streamBytesOutCurrentBlockIdx])[0];
        streamBytesOutCurrentBlockIdx++;
        streamBytesOutRemainingBytesInBlock = blockSize;
    }
    streamBytesOutRemainingBytesInBlock--;
    return *streamBytesOutByte++;
}

void EpBitmap::_linkBitmap(EpBitmap* nextBitmap) {
    _nextBitmap = nextBitmap;
}

/// TODO: fix
uint8_t EpBitmap::getLuminance(uint16_t color) {
#ifdef EPEPD_USE_PERCEIVED_LUMINANCE
    float r = float((color & 0b1111100000000000) >> 8) / 256.f;
    float g = float((color & 0b0000011111100000) >> 3) / 256.f;
    float b = float((color & 0b0000000000011111) << 3) / 256.f;
    return std::min(uint8_t((0.299f * r * r + 0.587f * g * g + 0.114f * b * b) * 256.f), uint8_t(0xFF));
#else
    return (color & 0b0000011111100000) >> 3;
#endif
}

void EpBitmap::drawPixel(int16_t x, int16_t y, uint16_t color) {
    setPixel(x, y, getLuminance(color));
}

