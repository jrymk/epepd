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
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    EpBitmap* fillBitmap = nullptr;
    EpPlacement fillBitmapPlacement;
    uint8_t fillColor = 0;

    EpShape(Shape shape, Operation op, int16_t x, int16_t y, int16_t w, int16_t h, EpBitmap* fill, EpPlacement &placement) :
            shape(shape), operation(op), x(x), y(y), w(w), h(h), fillBitmap(fill), fillBitmapPlacement(placement) {}

    EpShape(Shape shape, Operation op, int16_t x, int16_t y, int16_t w, int16_t h, EpBitmap* fill) :
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
    void include(EpRegion &region, EpPlacement &placement);

    EpShape getEpShape() { return EpShape(EpShape::RECTANGLE, EpShape::ADD, x, y, w, h, 0xFF); }

    // If userRegion is null -> full. X coordinates are byte-aligned (width is assumed a multiple of 8)
    static EpRegion _testAndAlign(EpRegion* userRegion, EpPlacement &placement, int16_t w, int16_t h);
};

class EpBitmap : public Adafruit_GFX {
public:
    EpBitmap(int16_t w, int16_t h, uint8_t bitsPerPixel);

    ~EpBitmap();

    enum BitmapShapeBlendMode {
        BITMAP_ONLY, SHAPES_ONLY,             //   if shape    | else
        BITMAP_ADD_SHAPES,                    //   shape       | bitmap
        BITMAP_SUBTRACT_SHAPES,               //   transparent | bitmap
        BITMAP_INTERSECT_SHAPES,              //   bitmap      | transparent
        //                      if bitmap != transparent color | else
        SHAPES_ADD_BITMAP,                    //   bitmap      | shape
        SHAPES_SUBTRACT_BITMAP,               //   transparent | shape
        SHAPES_INTERSECT_BITMAP,              //   shape       | transparent
    }; // did I miss anything?

    void setBitmapShapeBlendMode(BitmapShapeBlendMode mode);

    enum AdafruitGfxTargetMode {
        ON_BITMAP, ON_SHAPES,
        ON_VISIBLE
    };

    void setAdafruitGFXTargetMode(AdafruitGfxTargetMode mode);

    /// BITMAP MODE ///
    // by default EpBitmap uses bitmap blendMode. it takes up more memory space but allows for much better detail

    // allocate the bitmap memory, split into blocks, blockSize in bytes (you are not going to allocate multiple blocks over 64K on the esp32, so that's the biggest you can do)
    // blockSize must be a power of 2 (4200 will become 4096). I have to do this to get rid of the divides and mods
    virtual bool allocate(uint32_t blockSize);

    bool isAllocated() const;

    virtual void deallocate();


    // Adafruit_GFX override function to write buffer
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    // before drawing, call reset, this will include the pixels updated during drawing, useful when setting unknown updated region
    EpRegion gfxUpdatedRegion;

    // from 16 bit R5G6B5 color Adafruit_GFX uses to 8 bit luminance value for ePaper display
    static uint8_t getLuminance(uint16_t color);

    // do we need pixels over 8 bit wide?
    // get the color of a particular pixel, aligned left
    virtual uint8_t getPixel(int16_t x, int16_t y);

    virtual uint8_t getPixel(std::pair<int16_t, int16_t> coord) { return getPixel(coord.first, coord.second); }

    /// TODO: repeat bits so colors look good
    virtual uint8_t getBitmapPixel(uint32_t x, uint32_t y);

    virtual void setBitmapPixel(uint32_t x, uint32_t y, uint8_t color);

    virtual void setPixel(int16_t x, int16_t y, uint8_t color);

    /// SHAPES MODE ///
    // TODO: override fillCircle and fillRect functions and maybe roundedRect for a "more familiar" feel
    std::vector<EpShape> shapes;
    // the layers do matter (unless you use ADD for everything)
    // for simplicity’s sake, you just add shapes according to the operation order

    // not efficient, don't add too many shapes please
    virtual uint16_t getShapePixel(int16_t x, int16_t y);

    virtual void clearShapes();

    virtual void setRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color, EpShape::Operation operation);

    virtual void setRectangle(int16_t x, int16_t y, int16_t w, int16_t h, EpBitmap* bitmap, EpShape::Operation operation);

    virtual void setRectangle(int16_t x, int16_t y, int16_t w, int16_t h, EpBitmap* bitmap, EpPlacement &placement, EpShape::Operation operation);

    virtual void setCircle(int16_t x, int16_t y, int16_t diameter, uint8_t color, EpShape::Operation operation);

    virtual void setCircle(int16_t x, int16_t y, int16_t diameter, EpBitmap* bitmap, EpShape::Operation operation);

    virtual void setCircle(int16_t x, int16_t y, int16_t diameter, EpBitmap* bitmap, EpPlacement &placement, EpShape::Operation operation);

    virtual void pushShape(EpShape shape) { shapes.push_back(shape); }

    virtual void setTransparencyColor(uint8_t color);

    virtual void setTransparencyPattern();

    virtual uint8_t getTransparencyColor();

    /// for pros ///
    uint8_t** _getBlocks();

    virtual uint8_t _get8MonoPixels(int16_t x, int16_t y);

    virtual void _set8MonoPixels(int16_t x, int16_t y, uint8_t pixels);

protected:
    int16_t WIDTH, HEIGHT;
    uint8_t BPP;

    // transparent color for bitmap, and replacement colors for undefined pixels (it has to be exactly the same! even bits over the bpp)
    uint16_t transparencyColor = 0b01000000; // lo 8 bits: color = dark grey, for masks it will be interpreted as off (dark)

    BitmapShapeBlendMode blendMode = BITMAP_ONLY; // not going to waste time on getShapePixel by default
    AdafruitGfxTargetMode adafruitGfxMode = ON_BITMAP; // setPixel on shapes? are you sure?
    bool allocated = false;
    uint8_t** blocks;
    uint16_t blockSize; // in bytes
    uint16_t blockSizeExp; // 1 << blockSizeExp = blockSize
};


#endif //EPBITMAP_H
