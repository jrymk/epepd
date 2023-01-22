# epepd built-in functions

| | A2 | DU2 | GC2_FULL | GC2_PARTIAL | GC4 | GC16_FAST | GC16 |
| --: | :-- | :-- | :-- | :-- | :-- | :-- | :-- |
| Shades of grey |  2 | 2 | 2 | 2 | 4 | 16 | 16 |
| Speed | fastest | fast | normal | slower | normal | slower | slowest |
| Supports partial display | yes | yes | no | yes | no | no | no |
| Masks | partial, force | partial, force | | partial, force | | | |
| Flashes | 0 | 0 | 1 | 1 | 4 | 5 | 5 |
| Update cycles | 1 | 1 | 1 | 2 | 1 | 2 | 3 |

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

The resulting bwRam follows the general color of the display, so it will somewhat work with other update functions, although clearing over the greyscale image is strongly recommended.

> bwRam generally reflects the actual display contents on the screen. It is used by `EpPartialDisplay` to determine pixels to be updated. It may be used by other functions too.

| color | bwRam after display |
| --: | :-- |
| white | 1 |
| light grey | 1 |
| dark grey | 0 |
| black | 0 |

### GC16_FAST

Greyscale clear, 16 shades of grey (full screen update only)

Displays high quality (but poor colors) 4 bit greyscale image in **two** display cycles.

There are 3 options for different greyscale scales:

#### from black
This mode brightens the image from black. It has good contrast on the dark end (0000, 0001, 0010 are well distinguished), and terrible contrast at the bright end (half of the scale is almost white), best for dark image details. Greyscale strictly increasing.

#### from white
This mode darkens the image from white. It has good contrast on the bright end (1111, 1110, 1101 are well distinguished), and terrible contrast at the dark end (half of the scale is almost black), best for bright image details. Greyscale strictly increasing.

#### from grey
Better contrast across the board. The scale **overall** looks the best (most balanced and linear) but not strictly increasing (e.g. 1011 < 1100 and 0011 < 0100 not guaranteed).

If none of these options satisfy, try GC16, which uses 3 display cycles to display 64 possibilities. 16 of which is selected to create a better greyscale color set.

### GC16

Greyscale clear, 16 shades of grey (full screen update only)

This mode doesn't have a suffix while `GC16_FAST` does, because the quality of the greyscale image is more in-line with actual 16-shades-of-grey-capable ePaper displays with (at least) 4 sets of RAM and (at least) 16 transition waveform tables. With 2 cycles, we have 8 transition waveform tables (equivalent), which *can* make 16 shades of grey, but we have not much control.\
This is why mode `GC16` displays high quality (and better colors) 4 bit greyscale image in **three** display cycles, which can take about a full second.

