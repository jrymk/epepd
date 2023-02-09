#include "EpBitmap.h"

#define EPEPD_DEBUG Serial.printf

EpBitmap::EpBitmap(uint16_t w, uint16_t h, uint8_t bpp, uint32_t allocateSize) :
        Adafruit_GFX(w, h),
        width(w), height(h), bpp(bpp), allocateSize(allocateSize) {
    if (allocateSize == 0) return; // no alloc
    if (bpp == 0) bpp = 1;
    uint32_t bytes = (uint32_t(w) * h * bpp + 7) / 8;
    if (allocateSize == -1) {
        EPEPD_DEBUG("[V][epepd] Allocating %u bytes of continuous memory\n", bytes);
        bitmap = heap_caps_malloc(bytes, MALLOC_CAP_8BIT);
        if (!bitmap)
            EPEPD_DEBUG("[E][epepd] Failed to allocate memory. Bitmap will show cross pattern\n");
        return;
    }
    if ((this->allocateSize % 4) || (this->allocateSize % bpp)) {
        this->allocateSize -= (this->allocateSize %
                               (4 * bpp)); // technically we should mod by lcm(4, bpp), doesn't matter...
        EPEPD_DEBUG("[W][epepd] Allocate size must be a multiple of 4 and bpp!\nUsing %ulld\n", this->allocateSize);
    }
    uint16_t fullBlocksCnt = bytes / this->allocateSize;
    uint16_t remainderBlockSize = bytes - (uint32_t(fullBlocksCnt) * this->allocateSize);
    EPEPD_DEBUG("[V][epepd] Allocating %u bytes of memory in %u byte chunks\n", bytes, this->allocateSize);
    bitmap = heap_caps_malloc((fullBlocksCnt + (remainderBlockSize ? 1 : 0)) * sizeof(uint8_t *),
                              MALLOC_CAP_32BIT); // allocate memory to store pointers to blocks
    if (!bitmap) {
        EPEPD_DEBUG("[E][epepd] Failed to allocate memory. Bitmap will show cross pattern\n", bytes);
        return;
    }
    for (uint16_t b = 0; b < fullBlocksCnt; b++) {
        ((uint32_t **) bitmap)[b] = (uint32_t *) (heap_caps_malloc(this->allocateSize, MALLOC_CAP_32BIT));
        if (!((uint32_t **) bitmap)[b]) {
            EPEPD_DEBUG("[E][epepd] Failed to allocate memory. Bitmap will show cross pattern\n", bytes);
            for (uint16_t fb = 0; fb < b; fb++)
                heap_caps_free(((uint8_t **) bitmap)[fb]);
            heap_caps_free(bitmap);
            bitmap = nullptr;
            return;
        }
    }
    // allocate remainder block
    if (remainderBlockSize) {
        ((uint32_t **) bitmap)[fullBlocksCnt] = (uint32_t *) (heap_caps_malloc(remainderBlockSize, MALLOC_CAP_32BIT));
        if (!((uint32_t **) bitmap)[fullBlocksCnt]) {
            EPEPD_DEBUG("[E][epepd] Failed to allocate memory. Bitmap will show cross pattern\n", bytes);
            for (uint16_t fb = 0; fb < fullBlocksCnt; fb++)
                heap_caps_free(((uint32_t **) bitmap)[fb]);
            heap_caps_free(bitmap);
            bitmap = nullptr;
        }
    }
}


EpBitmap::~EpBitmap() {
    if (!bitmap) return;
    if (allocateSize != 0 && allocateSize != -1) { // allocated and not continuous
        uint32_t blocks = ((uint32_t(width) * height * bpp + 7) / 8 + allocateSize - 1) / allocateSize;
        for (uint16_t b = 0; b < blocks; b++)
            heap_caps_free(((uint8_t **) bitmap)[b]);
    }
    heap_caps_free(bitmap); // both split and continuous
    EPEPD_DEBUG("[V][epepd] Freed memory\n");
}

__attribute__((always_inline)) uint8_t EpBitmap::getPixel(int16_t x, int16_t y) {
    if (uint16_t(x) >= width || uint16_t(y) >= height) // param already takes unsigned
        return 0; // transparent
    if (!bitmap)
        return ((((x + y) & 0b110) == 0b110) || (((x - y) & 0b110) == 0b110)) ? 0x00 : 0xFF;

    uint32_t bitIdx = (y * width + x) * bpp; // of the left most bit of the pixel
    return (((allocateSize == 0
              ? ((uint32_t *) bitmap)[bitIdx >> 5]
              : (((uint32_t **) bitmap)[(bitIdx >> 3) / allocateSize])[((bitIdx >> 3) % allocateSize) >> 2])
            << (bitIdx & 0b11111)
            ) & pixelMask(bpp)) >> 24;
}

__attribute__((always_inline)) void EpBitmap::setPixel(int16_t x, int16_t y, uint8_t color) { // I don't care about the color format, I just take the first bpp bits
    if (x >= width || y >= height) // param already takes unsigned
        return;
    if (!bitmap) return;
    uint32_t bitIdx = (y * width + x) * bpp; // of the left most bit of the pixel
    uint32_t *dest = (allocateSize == 0
                      ? &(((uint32_t *) bitmap)[bitIdx >> 5])
                      : &((((uint32_t **) bitmap)[(bitIdx >> 3) / allocateSize])[((bitIdx >> 3) % allocateSize)
                    >> 2]));
    *dest &= ~(pixelMask(bpp) >> (bitIdx & 0b11111)); // rshift 1s + bpp 0s + 1s
    *dest |= ((uint32_t(color) << 24) & pixelMask(bpp)) >> (bitIdx & 0b11111);
}

__attribute__((always_inline)) bool EpShape::intersect(int16_t a, int16_t b) {
    if (shape == RECTANGLE)
        return ((a >= x) && (a <= x + w - 1) && (b >= y) && (b <= y + h - 1));
    else if (shape == CIRCLE)
        return ((float(a) - x) * (float(a) - x) + (float(b) - y) * (float(b) - y)) <= (float(w) / 2.f) * (float(w) / 2.f);
}

uint8_t EpBitmap::getLuminance(uint16_t color) {
#ifdef EPEPD_USE_PERCEIVED_LUMINANCE
    float r = float(((color & 0b1111100000000000) >> 8) + ((color & 0b1111100000000000) >> 13)) / 256.f;
    float g = float(((color & 0b0000011111100000) >> 3) + ((color & 0b0000011111100000) >> 9)) / 256.f;
    float b = float(((color & 0b0000000000011111) << 3) + ((color & 0b0000000000011111) >> 2)) / 256.f;
    return std::min(uint8_t((0.2126f * r + 0.7152f * g + 0.0722f * b) * 256.f), uint8_t(0xFF));
//    return std::min(uint8_t((0.299f * r * r + 0.587f * g * g + 0.114f * b * b) * 256.f), uint8_t(0xFF));
#else
    return (color & 0b0000011111100000) >> 3;
#endif
}

void EpBitmap::drawPixel(int16_t x, int16_t y, uint16_t color) {
    setPixel(x, y, getLuminance(color));
    gfxUpdatedRegion.include(x, y);
}

__attribute__((always_inline)) std::pair<int16_t, int16_t> EpPlacement::getTargetPos(int16_t x, int16_t y) const {
    if (rotation == 0)
        return {originX + x, originY + y};
    if (rotation == 1)
        return {originX + y, originY - x};
    if (rotation == 2)
        return {originX - x, originY - y};
    if (rotation == 3)
        return {originX - y, originY + x};
}

__attribute__((always_inline)) std::pair<int16_t, int16_t> EpPlacement::getSourcePos(int16_t x, int16_t y) const {
    if (rotation == 0)
        return {x - originX, y - originY};
    if (rotation == 1)
        return {originY - y, x - originX};
    if (rotation == 2)
        return {originX - x, originY - y};
    if (rotation == 3)
        return {y - originY, originX - x};
}

void EpRegion::reset() {
    x = 0;
    y = 0;
    w = -1;
    h = -1;
}

__attribute__((always_inline)) void EpRegion::include(int16_t ix, int16_t iy) {
    if (w < 0 || h < 0) {
        x = ix;
        y = iy;
        w = 1;
        h = 1;
        return;
    }
    if (ix < x)
        x = ix;
    if (iy < y)
        y = iy;
    if (ix >= x + w)
        w = ix - x + 1;
    if (iy >= y + h)
        h = iy - y + 1;
}

__attribute__((always_inline)) void EpRegion::include(std::pair<int16_t, int16_t> coord) {
    include(coord.first, coord.second);
}

__attribute__((always_inline)) void EpRegion::include(EpRegion &region, EpPlacement placement) {
    include(placement.getTargetPos(region.x, region.y));
    include(placement.getTargetPos(region.x + region.w - 1, region.y + region.h - 1));
}

__attribute__((always_inline)) bool EpRegion::includes(int16_t x0, int16_t y0) const {
    return !(x0 < x || y0 < y || x0 >= x + w || y0 >= y + h);
}

EpRegion EpRegion::_testAndAlign(EpRegion *userRegion, EpPlacement placement, int16_t w, int16_t h) {
    if (userRegion == nullptr)
        return EpRegion(0, 0, w, h);
    if (userRegion->w < 0 || userRegion->h < 0)
        return EpRegion(0, 0, 0, 0);
    int16_t l = placement.getTargetPos(userRegion->x, userRegion->y).first;
    int16_t r = placement.getTargetPos(userRegion->x + userRegion->w - 1, userRegion->y + userRegion->h - 1).first; // after the right edge
    int16_t u = placement.getTargetPos(userRegion->x, userRegion->y).second;
    int16_t d = placement.getTargetPos(userRegion->x + userRegion->w - 1, userRegion->y + userRegion->h - 1).second;
    if (l > r) std::swap(l, r);
    if (u > d) std::swap(u, d);
    r++;
    d++;
    l = (l < 0 ? 0 : (l >= w ? w - 1 : l)) & ~0b111;
    r = ((r < 0 ? 0 : (r > w ? w : r)) + 7) & ~0b111;
    u = u < 0 ? 0 : (u >= h ? h - 1 : u);
    d = d < 0 ? 0 : (d > h ? h : d);
    return (userRegion == nullptr)
           ? EpRegion(0, 0, w, h)
           : EpRegion(l, u, r - l, d - u); /// TODO: confirm this is correct
}

EpEpdRam::EpEpdRam(uint16_t w, uint16_t h) :
        EpBitmap(w, h, 1, -1) {
}

EpRectMask::EpRectMask() :
        EpBitmap(-1, -1, 1, 0) {
}
void EpRectMask::setRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    rects.emplace_back(x, y, w, h);
    Serial.printf("%d %d %d %d\n", x, y, w, h);
}
uint8_t EpRectMask::getPixel(int16_t x, int16_t y) {
    for (auto &rect: rects) {
        if (rect.includes(x, y))
            return invert ? 0x00 : 0xFF;
    }
    return invert ? 0xFF : 0x00;
}
