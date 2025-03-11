# Lanskern Bytecode Graphics API

The API allows you to draw lines, rectangles, etc.

The color is specified as a 8-bit color value.
Registers r1-r4 are used by the API. r0 is used to return status codes.

## Instructions

### 0xC0 GLINIT
Prepare the graphics API. This instruction must be the first instruction in the program.
Arguments:
- r1: Width of the window
- r2: Height of the window
Returns:
- r0: Status code
- Zero flag set if failed

### 0xC1 GLCLEAR
Clear the screen with current color
Arguments:
- none
Returns:
- none

### 0xC2 GLSETCOLOR
Set the current color
Arguments:
- r1: Color
Returns:
- none

### 0xC3 GLPLOT
Draw a pixel at x, y with current color
Arguments:
- r1: X coordinate
- r2: Y coordinate
Returns:
- none

### 0xC4 GLRECT
Draw a rectangle with current color
Arguments:
- r1: X coordinate
- r2: Y coordinate
- r3: Width
- r4: Height
Returns:
- none

### 0xC5 GLLINE
Draw a line with current color
Arguments:
- r1: Start X coordinate
- r2: Start Y coordinate
- r3: End X coordinate
- r4: End Y coordinate
Returns:
- none