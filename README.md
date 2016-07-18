blinkm_mk2
==========


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
