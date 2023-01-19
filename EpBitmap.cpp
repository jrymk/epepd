#include "EpBitmap.h"

EpBitmap::EpBitmap(int16_t w, int16_t h, uint8_t bitsPerPixel) {
    if (bitsPerPixel > 8) {
        Serial.printf("You can not create a bitmap with bpp over 8 yet\n");
    }
    WIDTH = w;
    HEIGHT = h;
    BPP = bitsPerPixel;
}

bool EpBitmap::allocate(uint16_t blockSize) {
#ifdef SHOW_HEAP_INFO
    Serial.printf("Before allocation:\n");
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif
    uint32_t requiredBytes = uint32_t(WIDTH) * uint32_t(HEIGHT) * BPP / 8;
    this->blockSize = blockSize;
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

uint8_t EpBitmap::getPixel(int16_t x, int16_t y) { // I hope I were better at optimization... this will get called a lot
    if (!allocated) {
        Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
        return outOfBoundsColor;
    }
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return outOfBoundsColor;
    int32_t bitIdx = (int32_t(y) * WIDTH + x + 1) * BPP - 1; // of the last bit of the pixel
    uint16_t joined = (blocks[(bitIdx >> 3) / blockSize])[(bitIdx >> 3) % blockSize];
    if ((bitIdx >> 3) >= blockSize) // not the first byte
        joined |= ((blocks[((bitIdx >> 3) - 1) / blockSize])[((bitIdx >> 3) - 1) % blockSize]) << 8;
    joined >>= 7 - (bitIdx & 0b111);
    joined &= (1 << BPP) - 1; // mask
    return (joined << (8 - BPP));
}

void EpBitmap::setPixel(int16_t x, int16_t y, uint8_t color) {
    if (!allocated) {
        Serial.printf("Fatal error: Accessing unallocated EpBitmap!\n");
        return;
    }
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    color >>= (8 - BPP); // align right (and trim excess)
    uint32_t bitIdx = (uint32_t(y) * WIDTH + x + 1) * BPP - 1; // of the last bit of the pixel
    uint16_t mask = ((uint16_t(1) << BPP) - 1) << (7 - (bitIdx & 0b111));
    (blocks[(bitIdx >> 3) / blockSize])[(bitIdx >> 3) % blockSize] &= ~mask; // reset
    (blocks[(bitIdx >> 3) / blockSize])[(bitIdx >> 3) % blockSize] |= color << (7 - (bitIdx & 0b111));
    if ((bitIdx & 0b111) + 1 < BPP) { // cross byte boundary
        (blocks[((bitIdx >> 3) - 1) / blockSize])[((bitIdx >> 3) - 1) % blockSize] &= ~(mask >> 8); // reset
        (blocks[((bitIdx >> 3) - 1) / blockSize])[((bitIdx >> 3) - 1) % blockSize] |= color >> ((bitIdx & 0b111) + 1);
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

void EpBitmap::streamBytesReset() {
    streamBytesCurrentBlock = 0;
    streamBytesCurrentByte = 0;
}

uint8_t EpBitmap::streamBytesNext() {
    uint8_t data = (blocks[streamBytesCurrentBlock])[streamBytesCurrentByte];
    streamBytesCurrentByte++;
    if (streamBytesCurrentByte == blockSize) {
        streamBytesCurrentBlock++;
        streamBytesCurrentByte = 0;
    }
    return data;
}
