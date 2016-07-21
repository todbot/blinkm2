blinkm_mk2
==========

## aka. BlinkM as WS2812 / NeoPixel controller

### Key Features
* WS2812 / NeoPixel controller (of at least 16 pixels)
* On-board LEDs (at least 2, probalby 3, maybe 4)
* Small! (< 1 inch square)
* Stand-alone operation, as original BlinkM
* On-board FET switch to power-down 1mA/pixel drain
** Need pullup on FET gate to ensure not floating on bootloader
* Inputs for controlling behavior

### What about?
* Turn into tiny Arduino! (attiny85, attiny84)
* Default firmware is just Arduino sketch
* USB-based (Micronuclues bootloader, like Digispark), NOT I2C? (but could use A0
* Connector for LEDs? (not just pads)
* Should on-board LEDs be in-line or parallel to off-board?
* Can we switch them? (Need more pins then, no ATtiny85)

### Ideas:
- Digispark based?
- events trigger patterns:
- CapSense using "ADCTouch" library  (can this work on ATtiny84/85 system?)
- put inbound resistor on inputs for protection?

### Notes:
- TinyTouchLib works on Digispark (ATtiny85 at 16.5MHZ), with light editing. See TinyTouchTest1 sketch

### How is this different from blink(1)
* blink(1) is retail product, not programmable, tied to a computer, with more robust USB stack
* BlinkM Neo is a tiny Arduino, meant to work stand-alonet0dsib0m



## Algorithmic Pattern ideas

* shift (shift_amount, led_start, led_cnt, time? ) (time==fader)
* color change (fade,ledn)
* hue change( hue, ledn)
* brighness change (brightness, ledn)
* set led ( ledn )

## advanced functions
* "alter color based on neighbors"
* "alter color based on position"
* "if neighbors are brighter than B, my brightness decrease by 10"
* "if neighbors hue is close to #FF, my hue increases by 5"


## INPUTS
* "input changes pattern"
* "input alters pattern"



## Examples

### example x: police flasher
- 0ms - set leds 1010101 to #ff0000
- 1ms set leds 01010101 to #0000ff
- 500ms set leds 010101 to #ff0000
- 501ms set leds 101010 to #0000ff


### example x: one-directional clyon
- 0ms - set leds 11111... to #000000
- 1ms - set leds 100000... to #ff000
- 100ms - shift left w/rotate
- go back 1

###  example x: bi-directional cylon
- 0ms - set leds 11111... to #000000
- 1ms - set leds 100000...
- 2ms - set ledcounter to 10
- 3ms - shift left
- 4ms - decrement ledcounter
- 100ms - go back 1 if ledcounter > 0
-
-
