#include "EpBitmapMono.h"

EpBitmapMono::EpBitmapMono(int16_t w, int16_t h) : EpBitmap(w, h, 1) {}

EpBitmapMono::~EpBitmapMono() {
    if (allocated)
        deallocate();
}

bool EpBitmapMono::allocate() {
    if (blendMode == SHAPES_ONLY)
        Serial.printf("Warning: Allocating bitmap buffer when shapes blendMode is selected.\n");
#ifdef SHOW_HEAP_INFO
    Serial.printf("Before allocation:\n");
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif
    uint32_t requiredBytes = uint32_t(WIDTH) * uint32_t(HEIGHT) / 8;

    buffer = (uint8_t*) (heap_caps_malloc(requiredBytes, MALLOC_CAP_8BIT));
    if (buffer == nullptr) {
        Serial.printf("Failed to allocate memory!\n");
        return false;
    }
    Serial.printf("buffer at %p\n", buffer);

#ifdef SHOW_HEAP_INFO
    Serial.printf("After allocation:\n");
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif
    Serial.printf("Allocated in total %d bytes of continuous memory\n", requiredBytes);
    allocated = true;
    return true;
}

void EpBitmapMono::deallocate() {
    heap_caps_free(buffer);
    allocated = false;
}

uint8_t EpBitmapMono::getBitmapPixel(uint32_t x, uint32_t y) {
    return ((buffer[(y * EpBitmap::WIDTH + x) >> 3] << (x & 0b111)) & 0b10000000) ? 0xFF : 0x00;
}

void EpBitmapMono::setBitmapPixel(uint32_t x, uint32_t y, uint8_t color) {
    if (color & 0b10000000)
        buffer[(y * EpBitmap::WIDTH + x) >> 3] |= 0b10000000 >> (x & 0b111);
    else
        buffer[(y * EpBitmap::WIDTH + x) >> 3] &= ~(0b10000000 >> (x & 0b111));
}

uint8_t* EpBitmapMono::_getBuffer() {
    return buffer;
}

uint8_t EpBitmapMono::_get8MonoPixels(int16_t x, int16_t y) {
    return buffer[(y * EpBitmap::WIDTH + x) >> 3];
}

void EpBitmapMono::_set8MonoPixels(int16_t x, int16_t y, uint8_t pixels) {
    buffer[(y * EpBitmap::WIDTH + x) >> 3] = pixels;
}

