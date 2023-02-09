#ifndef EPBITMAP_H
#define EPBITMAP_H

#define SHOW_HEAP_INFO

#include <Arduino.h>
#include <vector>
#include <Adafruit_GFX.h>

// color codes for Adafruit_GFX
#define GFX_BLACK     0b0000000000000000
#define GFX_DARKGREY  0b0101001010101010
#define GFX_LIGHTGREY 0b1010110101010101
#define GFX_WHITE     0b1111111111111111

#define GFX_GREY_0    0b0000000000000000
#define GFX_GREY_1    0b0001000010000010
#define GFX_GREY_2    0b0010000100000100
#define GFX_GREY_3    0b0011000110000110
#define GFX_GREY_4    0b0100001000101000
#define GFX_GREY_5    0b0101001010101010
#define GFX_GREY_6    0b0110001100101100
#define GFX_GREY_7    0b0111001110101110
#define GFX_GREY_8    0b1000110001010001
#define GFX_GREY_9    0b1001110011010011
#define GFX_GREY_A    0b1010110101010101
#define GFX_GREY_B    0b1011110111010111
#define GFX_GREY_C    0b1100111001111001
#define GFX_GREY_D    0b1101111011111011
#define GFX_GREY_E    0b1110111101111101
#define GFX_GREY_F    0b1111111111111111

#define EPEPD_USE_PERCEIVED_LUMINANCE 1 // (doesn't make it exponent/log whatever, just balance out the perception of rgb colors)

// Determines the relative position of bitmaps, for feeding into functions and setting EpShape fills
struct EpPlacement {
    int16_t originX = 0;
    int16_t originY = 0;
    int8_t rotation = 0; // *90 degrees counter-clockwise (obviously)

    EpPlacement() = default;

    // how the source bitmap is placed onto the target bitmap,
    // by "where the (0,0) on the source bitmap goes, in terms of target bitmap coordinates
    // and the rotation, in 90 degrees counter-clockwise turns of the source image
    EpPlacement(int16_t x, int16_t y, int8_t rotation = 0) : originX(x), originY(y), rotation(rotation & 0b11) {}

    std::pair<int16_t, int16_t> getTargetPos(int16_t x, int16_t y) const;

    std::pair<int16_t, int16_t> getTargetPos(std::pair<int16_t, int16_t> coord) const { return getTargetPos(coord.first, coord.second); }

    std::pair<int16_t, int16_t> getSourcePos(int16_t x, int16_t y) const;

    std::pair<int16_t, int16_t> getSourcePos(std::pair<int16_t, int16_t> coord) const { return getSourcePos(coord.first, coord.second); }
};

class EpBitmap;

struct EpShape {
    enum Shape {
        RECTANGLE,
        CIRCLE
    } shape;
    enum Operation {
        ADD,
        SUBTRACT,
        INTERSECT,
        INTERSECT_USE_BEHIND,
        EXCLUDE // excludes overlapping areas
    } operation;
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
    EpBitmap *fillBitmap = nullptr;
    EpPlacement fillBitmapPlacement;
    uint8_t fillColor = 0;

    EpShape() = default;

    EpShape(Shape shape, Operation op, int16_t x, int16_t y, int16_t w, int16_t h, EpBitmap *fill, EpPlacement placement) :
            shape(shape), operation(op), x(x), y(y), w(w), h(h), fillBitmap(fill), fillBitmapPlacement(placement) {}

    EpShape(Shape shape, Operation op, int16_t x, int16_t y, int16_t w, int16_t h, EpBitmap *fill) :
            shape(shape), operation(op), x(x), y(y), w(w), h(h), fillBitmap(fill), fillBitmapPlacement(0, 0, 0) {}

    EpShape(Shape shape, Operation op, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) :
            shape(shape), operation(op), x(x), y(y), w(w), h(h), fillBitmapPlacement(0, 0, 0), fillColor(color) {}

    bool intersect(int16_t x, int16_t y);
};

// Stores a rectangle to limit pixel-by-pixel scan area
struct EpRegion {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;

    EpRegion() = default;

    EpRegion(int16_t x, int16_t y, int16_t w, int16_t h) : x(x), y(y), w(w), h(h) {}

    void reset();

    void include(int16_t x, int16_t y);

    void include(std::pair<int16_t, int16_t> coord);

    // include a "source" bitmap updated region (doing the other way around doesn't make sense)
    void include(EpRegion &region, EpPlacement placement);
    bool includes(int16_t x, int16_t y) const;
    EpShape getEpShape() { return EpShape(EpShape::RECTANGLE, EpShape::ADD, x, y, w, h, 0xFF); }

    // If userRegion is null -> full. X coordinates are byte-aligned (width is assumed a multiple of 8)
    static EpRegion _testAndAlign(EpRegion *userRegion, EpPlacement placement, int16_t w, int16_t h);
};

class EpBitmap : public Adafruit_GFX {
public:
    EpBitmap(uint16_t w, uint16_t h, uint8_t bpp, uint32_t allocateSize = 4096);

    ~EpBitmap();

    // before drawing, call reset, this will include the pixels updated during drawing, useful when setting unknown updated region
    EpRegion gfxUpdatedRegion;

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    // from 16 bit R5G6B5 color Adafruit_GFX uses to 8 bit luminance value for ePaper display
    static uint8_t getLuminance(uint16_t color);

    // do we need pixels over 8 bit wide?
    // get the color of a particular pixel, aligned left
    virtual uint8_t getPixel(int16_t x, int16_t y);

    virtual uint8_t getPixel(std::pair<int16_t, int16_t> coord) { return getPixel(coord.first, coord.second); }

    virtual void setPixel(int16_t x, int16_t y, uint8_t color);

    const uint16_t width; // don't change this plz
    const uint16_t height;
    const uint8_t bpp;
    void *bitmap = nullptr;

private:
    uint32_t allocateSize;
    static constexpr uint32_t pixelMask(uint8_t bits) { return ~((1 << (32 - bits)) - 1); }
};

class EpEpdRam : public EpBitmap {
public:
    EpEpdRam(uint16_t w, uint16_t h);
};

class EpRectMask : public EpBitmap {
public:
    EpRectMask();
    bool invert = false;
    void setRect(int16_t x, int16_t y, int16_t w, int16_t h);
    void setRect(EpRegion &region) {
        setRect(region.x, region.y, region.w, region.h);
    }
    uint8_t getPixel(int16_t x, int16_t y) override;
    void setPixel(int16_t x, int16_t y, uint8_t color) override {
        setRect(x, y, 1, 1);
    }
    void reset() { rects.clear(); }
    EpRegion getRegion() {
        EpRegion region;
        for (auto &rect: rects)
            region.include(rect, EpPlacement(0, 0, 0));
        return region;
    }

private:
    std::vector<EpRegion> rects;
};


#endif //EPBITMAP_H
