# Epepd

An **eP**aper **d**isplay library with **e**ndless **p**ossibilities\
_currently only supports Waveshare 3.7" ePaper HAT_

## What can it do?

#### Anti-aliasing

Rough edges? With `getPixelLUT` custom transition function, you can send pixel data that are multisampled from neighbor pixels.

<img src="doc/antialiasing.jpg" width="400">

Yes, I know it looks terrible, especially that `c`... Although there's a white line that went through it.\
But all you need to do is...

```cpp
EpBitmap gfxBuffer(480, 280, 1); // set the Adafruit_GFX buffer
Epepd epd(gfxBuffer, EPAPER_CS, EPAPER_DC, EPAPER_RST, EPAPER_BUSY);

void setup() {
    epd.init();
    gfxBuffer.allocate(4096); // allocate the buffer in 4KB chunks so that it will fit in memory
    
    // start drawing
    epd.setTextColor(BLACK);
    epd.setCursor(10, 100);
    epd.print("Connecting to\nWiFi...");
    epd.setFont(&HarmonyOS_Sans_Medium8pt7b);
    epd.setCursor(10, 180);
    epd.print("Connecting to WiFi...");
    epd.drawLine(0, 150, 260, 9, BLACK);
    epd.drawLine(0, 10, 150, 425, WHITE);

    // display
    epd.display();
}
```

and for the display function override... _(the actual "plugin" style classes has not been implemented yet)_

```cpp
void Epepd::display() {
    initDisplay();
    
    writeToDisplay([](Epepd &epepd, int16_t x, int16_t y) {
        int blackPixels = 0;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                if (epepd.gfxBuffer->getPixel(x + dx, y + dy)) blackPixels++;
        const uint8_t defBlack[] = { LUT0, LUT0, LUT1, LUT1, LUT2, LUT2, LUT2, LUT2, LUT3, LUT3 };
        const uint8_t defWhite[] = { LUT0, LUT0, LUT0, LUT0, LUT0, LUT1, LUT1, LUT1, LUT2, LUT3 };
        // what LUT0-3 does depend on the LUT, which will be customizable too
        // in this case it is just a normal 4-shades-of-grey LUT
        return (epepd.gfxBuffer->getPixel(x, y) ? defBlack[blackPixels] : defWhite[blackPixels]);
    });
    
    updateDisplay();
}
```

Now you have smooth lines and kinda smooth text, without special fonts or drawing functions, and the same memory usage!

#### Supersampling

If you have plenty of RAM to spare, you can increase the `Adafruit_GFX` frame buffer resolution, and then use the `getPixelLUT` function to output an
appropriate level of grey for smooth fonts too (as GFXFonts don't do anti-aliased fonts, what about just supersample it?)

<img src="doc/supersampling.jpg" width="400">

And all you need to do is...

```cpp
EpBitmap gfxBuffer(960, 560, 1); // set the Adafruit_GFX buffer oversized
Epepd epd(gfxBuffer, EPAPER_CS, EPAPER_DC, EPAPER_RST, EPAPER_BUSY);

void setup() {
    epd.init();
    gfxBuffer.allocate(4096); // allocate the buffer in 4KB chunks so that it will fit in memory
    
    // start drawing, with everything doubled
    epd.setFont(&Aero_Matics_Bold44pt7b);
    epd.fillScreen(WHITE);
    epd.setTextColor(BLACK);
    epd.setCursor(20, 200);
    epd.print("Connecting to\nWiFi...");
    ...
    
    // display
    epd.display();
}
```

and for the display function override...

```cpp
void Epepd::display() {
    initDisplay();
    
    writeToDisplay([](Epepd &epepd, int16_t x, int16_t y) {
        int blackPixels = 0;
        for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++)
                if (epepd.gfxBuffer->getPixel(x * 2 + dx, y * 2 + dy)) blackPixels++;
        const uint8_t def[] = {LUT0, LUT1, LUT1, LUT2, LUT3};
        return def[blackPixels];
    });
    
    updateDisplay();
}
```

and that's it. That's pretty intuitive if I say so myself.

#### Mixed display modes

What is stopping "4 shades of grey" and "partial update" from existing the same time? It is that there is only two sets of memory in the display controller. By
storing the states ourselves, we can make partial updaate work without wiping the drawn greyscale image using the powerful `getPixelLUT` function.

#### Maybe 8 shades of grey? How about 16?

By switching LUTs within a display update routine, maybe, just maybe, we can display even more levels of grey. Although the displays that have way higher
resolutions and 16 levels of grey costs just 3 to 4 times more than this 3.7 inch display that is apparently not very popular.\
(The 4.3 inch one seems more
popular, especially the B/W/Red ones, but geez the red ones are barely usable with the painfully slow update speed and lack of *official* partial update
support)

#### Blur? Dithering? Effects!

You get the point.

---

Basically, the library _will_ allow you to do just about anything technically possible. But I probably will only implement some that I need, so that's why it is
called the **Endless Possibility ePaper Display Library**, I'm just creating the framework, not implementing all kinds of dithering algorithms.

## Display support

To keep the development process fast, I didn't future-proof it with any display module selection code. It can be added later though. But it is certain that I
can't support a lot of displays, because each display will require its own LUT and tuning. I may make my classes compatible with Zingg's GxEPD2 library, but it
will still require you to have the knowledge to tinker and write your own LUT for support of other modules.

Also, this library is clearly not done. None of the features mentioned is implemented yet. This library can currently display 4 shades of grey.

## Microcontroller support

The whole library will be designed for the ESP32. Maybe Teensy 4.1.\
The balance of features and compromises is decided based on the ESP32, mainly DRAM related problems.

## References

[Display driver datasheet](https://www.waveshare.com/w/upload/2/2a/SSD1677_1.0.pdf)\
[Waveshare wiki page](https://www.waveshare.com/wiki/3.7inch_e-Paper_HAT_Manual#Introduction)\
[ZinggJM/GxEPD2](https://github.com/ZinggJM/GxEPD2)\
[ZinggJM/GxEPD2_4G](https://github.com/ZinggJM/GxEPD2_4G)\
[Definitions of display modes on high end epaper displays that I can't afford](https://www.waveshare.net/w/upload/c/c4/E-paper-mode-declaration.pdf)\
[Allocating Frame Buffer Memory 4KB At A Time](https://newscrewdriver.com/2021/05/21/allocating-frame-buffer-memory-4kb-at-a-time/)\
[ESP API that will make allocating tons of memory possible](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/mem_alloc.html#_CPPv432heap_caps_get_largest_free_block8uint32_t)