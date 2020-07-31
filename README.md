blinkm_mk2
==========

## aka. BlinkM2, aka BlinkM as WS2812 / NeoPixel controller

### Key Features
* WS2812 / NeoPixel controller (of at least 16 pixels)
* On-board LEDs (maybe two or more)
* Small! (< 1 inch square)
* Stand-alone operation, as original BlinkM
* Inputs for controlling behavior
* Turn into tiny Arduino! (attiny85, attiny84)
* Default firmware is just Arduino sketch (uses awesome ATtinyCore)

### How is this different from blink(1) USB RGB LED?
* blink(1) is retail product, not programmable, tied to a computer, with more robust USB stack
* BlinkM2 / BlinkM Neo is a tiny Arduino, meant to work stand-alone


### Some future directions
* Connector for additional Neopixel LEDs (not just pads)
* USB-based? NO (Micronuclues bootloader, like Digispark), NOT I2C? (but could use A0)
* Serial as well as I2C?  Maybe with SoftSerial
* Should on-board LEDs be in-line or parallel to off-board?
* Can we switch them? (Need more pins then, no ATtiny85)
- Digispark based?
- events trigger patterns:
- CapSense using "ADCTouch" library  (can this work on ATtiny84/85 system?)
- put inbound resistor on inputs for protection?

### Notes:
- TinyTouchLib works on Digispark (ATtiny85 at 16.5MHZ), with light editing. See TinyTouchTest1 sketch

### Further Reading:
- See also PatternIdeas.md


-
-
