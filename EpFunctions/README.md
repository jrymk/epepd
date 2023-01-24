# epepd built-in drawing functions

| | A2 | DU2 | GC2_FULL | GC2_PARTIAL | GC4 | GC16_FAST | GC16 |
| --: | :-- | :-- | :-- | :-- | :-- | :-- | :-- |
| Shades of grey | 2 | 2 | 2 | 2 | 4 | 16 | 16 |
| Speed | 0.54s | 0.96s | 0.79s | 1.40s | 1.34s | 1.45s | 2.58s |
| Supports partial display | yes | yes | no | yes | no | no | no |
| Masks | partial, force | partial, force | | partial, force | | | |
| Flashes | 0 | 0 | 1 | 1 | 3 | 3 | 3 |
| Update cycles | 1 | 1 | 1 | 2 | 3 | 2 | 3 |

## EpPartialDisplay

This function contains the most common and basic display modes for non-greyscale displaying.

### A2

Animation, 2 shades of grey

Only supports 1 bit of greyscale, it is the fastest update mode, with no flashes. The quality is the worst too, with quite some ghosting.\
Should only be used during animations.

Supports masked partial display (select a mask, only pixels that are bright in the mask will be updated)

### DU2

Direct update, 2 shades of grey

Support from any display state to black or white. Useful to quickly wipe the screen.

Supports masked partial display (select a mask, only pixels that are bright in the mask will be updated)

### GC2_FULL

Greyscale clear, 2 shades of grey, full screen update only

Displays high quality, ghost free (provided that the old frame buffer is present, for example the first refresh from reset will leave ghosting)\
It will flash once, and since this particular mode will update the whole display, it isn't really "partial display". Because of the limited frame memory on the
ePaper controller, hacky and less performant workarounds has to be used.

### GC2_PARTIAL

Greyscale clear, 2 shades of grey, supports partial update

Requires two screen update cycles to achieve, this is a display mode with actual "logic" which makes `epepd` special.\
At the cost of time, it allows you to use greyscale clear `GC2` only on certain pixels.

Supports masked partial display (select a mask, only pixels that are bright in the mask will be updated)\
Also supports force update mask (select a mask, pixels that are bright will be refreshed regardless of the previous image)\
In fact, all partial-update-capable display modes above supports force update mask, but it is less useful there. (already black pixels may become even darker,
making both the old and new image visible, new image being darker)

### Transition in and out of A2

When displaying static image, use a function that supports `greyscale` for maximum resolution (text antialiasing for example).

When animation starts, use `DU2` to wipe the screen white, and then repeatly use `A2` during the animation.

When animation ends, use `A2` to wipe the screen, and then show the final image with the `greyscale` display function.

## EpGreyscaleDisplay

This function contains the most common and basic display modes for greyscale displaying.

### GC4

Greyscale clear, 4 shades of grey (full screen update only)

Displays high quality 2 bit greyscale image in one display cycle.

The resulting bwRam follows the general color of the display, so it will somewhat work with other update functions, although clearing over the greyscale image
is strongly recommended.

> bwRam generally reflects the actual display contents on the screen. It is used by `EpPartialDisplay` to determine pixels to be updated. It may be used by
> other functions too.

| color | bwRam after display |
| --: | :-- |
| white | 1 |
| light grey | 1 |
| dark grey | 0 |
| black | 0 |

### GC16_FAST

Greyscale clear, 16 shades of grey (full screen update only)

Displays high quality 4 bit greyscale image in **two** display cycles.

This mode is actually over two times faster than `GC16`, because it does not require the display to reach full black before updating. This mode also "feels"
faster, because the display reaches a color close to the final image early in the first cycle. If the best "16 shades" isn't required, for example when showing
antialiased text or most bitmaps, then this is a good mode for quickly showing greyscale content. It's almost as fast as `GC4`, although `GC4` does produce a
cleaner image.\
Also the black is not as dark as other greyscale display modes (depends on the temperature I guess)

### GC16

Greyscale clear, 16 shades of grey (full screen update only)

This mode doesn't have a suffix while `GC16_FAST` does, because the quality of the greyscale image is more in-line with actual 16-shades-of-grey-capable ePaper
displays with (at least) 4 sets of RAM and (at least) 16 transition waveform tables. With 2 cycles, we have 8 transition waveform tables (equivalent), which
*can* make 16 shades of grey, but we have not much control.\
This is why mode `GC16` displays high quality (and better colors) 4 bit greyscale image in **three** display cycles, which can longer to update. Also the
display will reach full black before displaying content, and that takes quite some time too.

The image produced by this mode looks brighter.

# epepd built-in EpBitmap functions

These functions override `EpBitmap` `getPixel` to output pixel data programmatically, instead of simply reading from memory.\
They act just like `EpBitmap`s, and can be used as inputs for other functions.

## EpIntegerScaling

Scales the image up or down by an integer factor. No smoothing is applied to scale ups, meaning the output will be blocky if scaled up.

Negative scaling factor means scale down.

##### Example:

```cpp
EpIntegerScaling intScale(&gfxBuffer, 2); // input pixel x=3, y=5 is now equivalent to output rect x=6, y=10, w=2, h=2
greyscaleDisplay.display(&ssaa, placement, EpGreyscaleDisplay::GC16_FAST);
delay(1000);
ssaa.setScale(3); // input pixel x=3, y=5 is now equivalent to output rect x=9, y=15, w=3, h=3
greyscaleDisplay.display(&ssaa, placement, EpGreyscaleDisplay::GC16_FAST);
delay(1000);
ssaa.setScale(-2); // output pixel x=3, y=5 is now the average of input pixels (6, 10), (7, 10), (6, 11), (7, 11)
greyscaleDisplay.display(&ssaa, placement, EpGreyscaleDisplay::GC16_FAST);
delay(1000);
ssaa.setScale(-3); // output pixel x=3, y=5 is now the average of input pixels (9, 15), (10, 15), (11, 15), (9, 16), (10, 16), (11, 16), (9, 17), (10, 17), (11, 17)
greyscaleDisplay.display(&ssaa, placement, EpGreyscaleDisplay::GC16_FAST);
delay(1000);
```

## EpSSAA4x

Supersampling Anti-Aliasing, internally allocates a bitmap 4 times bigger (2 times on each edge). Each `Adafruit_GFX` `drawPixel` call draws 4 pixels in a
specified pattern. When a pixel color is requested, 4 pixels are read and combined to create the originally saved color.

Certain functions can write to the internal bitmap to allow "sub-pixel divisions". When a pixel is read back, 4 pixels are combined. This produces an image that
is smoother at the edges.

Text, circles and lines have intermediate colors at the edges to make the pixels less harsh and overall looks better.\
**These functions are not yet implemented.**

| output bpp | allocated bpp | memory increase |
| --: | --: | --: |
| 2 | 1 | 2 times |
| 3 | 2 | 2.67 times |
| 4 | 2 | 2 times |

<details>
<code>
Output Pixel to Internal Pixels Conversion<br>
[output color in dec] output color in bin <-conversion direction-> top left pixel, top right pixel, bottom left pixel, bottom right pixel [sum of 4 pixels]<br>
2 bits per output pixel<br>
[0] 00 <-> 0 0 0 0 [0]<br>
[1] 01 <-> 0 0 0 1 [1]<br>
[2] 10 <-> 0 0 1 1 [2]<br>
[3] 11 <-- 0 1 1 1 [3]<br>
[3] 11 <-> 1 1 1 1 [4]<br>
3 bits per output pixel<br>
[0] 000 <-> 00 00 00 00 [0]<br>
[1] 001 <-- 00 00 00 01 [1]<br>
[1] 001 <-> 00 00 01 01 [2]<br>
[2] 010 <-- 00 01 01 01 [3]<br>
[2] 010 <-> 01 01 01 01 [4]<br>
[3] 011 <-- 01 01 01 10 [5]<br>
[3] 011 <-> 01 01 10 10 [6]<br>
[4] 100 <-> 01 10 10 10 [7]<br>
[5] 101 <-> 10 10 10 10 [8]<br>
[6] 110 <-- 10 10 10 11 [9]<br>
[6] 110 <-> 10 10 11 11 [10]<br>
[7] 111 <-- 10 11 11 11 [11]<br>
[7] 111 <-> 11 11 11 11 [12]<br>
4 bits per output pixel<br>
(!): ordered pattern needed to differentiate<br>
[0] 0000 <-> 00 00 00 00 [0]<br>
[1] 0001 <-> 00 00 00 01 [1]<br>
[2] 0010 <-> 00 00 01 01 [2]<br>
[3] 0011 <-> 00 01 01 01 [3] (!)<br>
[4] 0100 --> 01 00 01 01 [3] (!)<br>
[5] 0101 <-> 01 01 01 01 [4]<br>
[6] 0110 <-> 01 01 01 10 [5]<br>
[7] 0111 <-> 01 01 10 10 [6]<br>
[8] 1000 <-> 01 10 10 10 [7] (!)<br>
[9] 1001 --> 10 01 10 10 [7] (!)<br>
[10] 1010 <-> 10 10 10 10 [8]<br>
[11] 1011 <-> 10 10 10 11 [9]<br>
[12] 1100 <-> 10 10 11 11 [10]<br>
[13] 1101 <-> 10 11 11 11 [11] (!)<br>
[14] 1110 --> 11 10 11 11 [11] (!)<br>
[15] 1111 <-> 11 11 11 11 [12]
</code>
</details>

##### Example:

(No anti-aliasing is utilized in the example below. Cicles, lines and text Adafruit_GFX function override is not implemented yet. There is currently no way of
writing to the internal bitmap yet)

```cpp
EpSSAA4x gfxBuffer(480, 280, 4);
EpPlacement placement(279, 0, 3);
EpGreyscaleDisplay greyscaleDisplay(epd);
gfxBuffer.allocate(4096); // allocated a 960*560, 2BPP bitmap

gfxBuffer.drawRGBBitmap(0, 0, miorinesan, 480, 280);
greyscaleDisplay.display(&gfxBuffer, placement, EpGreyscaleDisplay::GC16_FAST);
```