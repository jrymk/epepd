#ifndef EPBITMAP_H
#define EPBITMAP_H

#include <Arduino.h>
#include <vector>
#include <Adafruit_GFX.h>

// color codes for Adafruit_GFX
#define GFX_BLACK 0x0000
#define GFX_DARKGREY 0x7BEF
#define GFX_LIGHTGREY 0xC618
#define GFX_WHITE 0xFFFF

#define EPEPD_USE_PERCEIVED_LUMINANCE 1

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
    uint8_t color;

    EpShape(Shape shape, Operation op, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) :
            shape(shape), operation(op), x(x), y(y), w(w), h(h), color(color) {}

    bool intersect(int16_t x, int16_t y);
};

class EpBitmap : public Adafruit_GFX {
public:
    EpBitmap(int16_t w, int16_t h, uint8_t bitsPerPixel);

    ~EpBitmap();

    // Adafruit_GFX override function to write buffer
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    // from 16 bit R5G6B5 color Adafruit_GFX uses to 8 bit luminance value for ePaper display
    static uint8_t getLuminance(uint16_t color);

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
    bool allocate(uint32_t blockSize);

    bool isAllocated() const;

    void deallocate();


    // do we need pixels over 8 bit wide?
    // get the color of a particular pixel, aligned left
    uint8_t getPixel(int16_t x, int16_t y);

    void setPixel(int16_t x, int16_t y, uint8_t color);

    uint8_t getBitmapPixel(int16_t x, int16_t y);

    void setBitmapPixel(int16_t x, int16_t y, uint8_t color);

    /// SHAPES MODE ///
    // TODO: override fillCircle and fillRect functions and maybe roundedRect for a "more familiar" feel
    std::vector<EpShape> shapes;
    // the layers do matter (unless you use ADD for everything)
    // for simplicity’s sake, you just add shapes according to the operation order

    // not efficient, don't add too many shapes please
    uint16_t getShapePixel(int16_t x, int16_t y);

    void clearShapes();

    void setRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color, EpShape::Operation operation = EpShape::ADD);

    void setCircle(int16_t x, int16_t y, int16_t diameter, uint8_t color, EpShape::Operation operation = EpShape::ADD);

    void setTransparencyColor(uint8_t color);

    void setTransparencyPattern();

    uint8_t getTransparencyColor();

    /// for pros (for redRam and bwRam) ///
    void _linkBitmap(EpBitmap* nextBitmap);

    EpBitmap* _nextBitmap = nullptr; // the remaining bits will be shifted left and sent to the next bitmap

    void _streamBytesInBegin();

    void _streamBytesOutBegin();

    void _streamInBytesNext(uint8_t byte);

    // keep count of bytes read yourself, it will certainly crash if you go over!
    uint8_t _streamOutBytesNext();

private:
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
    /// TODO: vector blendMode

    uint16_t streamBytesInCurrentBlock;
    uint32_t streamBytesInCurrentByte;
    uint16_t streamBytesOutCurrentBlock;
    uint32_t streamBytesOutCurrentByte;
};


#endif //EPBITMAP_H
