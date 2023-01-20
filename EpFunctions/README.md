# epepd built-in functions

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

| | A2 | DU2 | GC2_FULL | GC2_PARTIAL |
| --: | :-- | :-- | :-- | :-- |
| Shades of grey |  2 | 2 | 2 | 2 |
| Speed | fastest | fast | normal | slower |
| Supports partial display | yes | yes | no | yes |
| Masks | partial, force | partial, force | | partial, force |
| Flashes | 0 | 0 | 1 | 1 |
| Update cycles | 1 | 1 | 1 | 2 |

### Transition in and out of A2

When displaying static image, use a function that supports `greyscale` for maximum resolution (text antialiasing for example).

When animation starts, use `DU2` to wipe the screen white, and then repeatly use `A2` during the animation.

When animation ends, use `A2` to wipe the screen, and then show the final image with the `greyscale` display function.