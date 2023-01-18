# HybridEPD

An ePaper display library for ultimate customizability\
_currently only supports Waveshare 3.7" ePaper HAT_

## What can it do?

#### Anti-aliasing

Rough edges? With `bufferToLUT` custom transition function, you can send pixel data that are multisampled from neighbor pixels.

#### Supersampling

If you have plenty of RAM to spare, you can increase the `Adafruit_GFX` frame buffer resolution, and then use the `bufferToLUT` function to output an
appropriate level of grey for smooth fonts too (as GFXFonts don't do anti-aliased fonts, what about just supersample it?)

#### Mixed display modes

What is stopping "4 shades of grey" and "partial update" from existing the same time? It is that there is only two sets of memory in the display controller. By
storing the states ourselves, we can make partial updaate work without wiping the drawn greyscale image using the powerful `bufferToLUT` function.

#### Maybe 8 shades of grey? How about 16?

By switching LUTs within a display update routine, maybe, just maybe, we can display even more levels of grey. Although the displays that have way higher
resolutions and 16 levels of grey costs just 3 to 4 times more than this 3.7 inch display that is apparently not very popular.\
(The 4.3 inch one seems more
popular, especially the B/W/Red ones, but geez the red ones are barely usable with the painfully slow update speed and lack of *official* partial update
support)

#### Blur? Dithering? Effects!

You get the point.

## Display support

To keep the development process fast, I didn't future-proof it with any display module selection code. It can be added later though. But it is certain that I
can't support a lot of displays, because each display will require its own LUT and tuning. I may make my classes compatible with Zingg's GxEPD2 library, but it
will still require you to have the knowledge to tinker and write your own LUT for support of other modules.

Also, this library is clearly not done. None of the features mentioned is implemented yet. This library can currently display 4 shades of grey.

## References

[Display driver datasheet](https://www.waveshare.com/w/upload/2/2a/SSD1677_1.0.pdf)\
[Waveshare wiki page](https://www.waveshare.com/wiki/3.7inch_e-Paper_HAT_Manual#Introduction)\
[ZinggJM/GxEPD2](https://github.com/ZinggJM/GxEPD2)\
[ZinggJM/GxEPD2_4G](https://github.com/ZinggJM/GxEPD2_4G)\
[Definitions of display modes on high end epaper displays that I can't afford](https://www.waveshare.net/w/upload/c/c4/E-paper-mode-declaration.pdf)
