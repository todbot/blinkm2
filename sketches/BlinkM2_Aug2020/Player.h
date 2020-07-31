#ifndef Player_h
#define Player_h

#include "blinkm_config.h"

#include "BlinkMTypes.h"

const int faderMax = NUM_LEDS;

class Player
{
    uint8_t nLEDs; // number of LEDs in strip

    rgb_t* leds; // pointer to real LED array in global memory

    bool playing;
    //callback_t stripUpdater;

    uint8_t tick; // 1/100th ticks
    //uint32_t tickUpdateNext;

    uint8_t scriptTick;
    uint8_t scriptId;
    uint8_t scriptLen;
    uint8_t scriptReps;
    uint8_t playPos;

    uint8_t cmd;     // cmd currently being worked on (from i2c or script)
    uint8_t args[3]; // args for cmd
    uint8_t dur;     // duration of this command
   
    uint8_t ledn; // current led to operate on, 0= all, 1st led = 1, 2nd = 2, ...
    uint8_t brightness;
    
    // fader logic consists of array of 'ledn' (ids of which LED to modify)
    // and matching array of destination colors
    //uint8_t ledns[faderMax];  // multi fader support, 255= not in use, 0=all, 1= 1st led, etc.

    // for now: every led in leds[] gets an led_dests[] fader target
    rgb_t led_dests[faderMax]; // multi fader support

    uint8_t* inputs;  // analog inputs, pointer to global held in main sketch

    uint8_t fadespeed;
    int8_t timeadj;

    uint8_t hue, sat, bri;
    
 public:
    Player(rgb_t* pleds, uint8_t nnLEDs )
    {
        leds = pleds;
        nLEDs = nnLEDs;
        ledn = 0;
        playing = false;
        fadespeed = 10;
        brightness = 255;
        off();
    }
    
    void update(); // called by Timer object every 10ms

    void doFaders();

    void setFadespeed( uint8_t fs ) { fadespeed = fs; }
    void setInputs( uint8_t* ins) { inputs = ins; }
    void setBrightness(uint8_t b) { brightness = b; }
    
    void setCmd(uint8_t c) { cmd = c; }
    void setArgs(uint8_t a, uint8_t b, uint8_t c) {
        args[0] = a;  args[1] = b;  args[2] = c;
    }

    void handleCmd();  // uses only cmd and args
    //void handleScriptCmd(uint8_t cmd, uint8_t* args);  // uses given cmd and args
    void handleCmd(uint8_t cmd, uint8_t arg0, uint8_t arg1, uint8_t arg2);  // uses given cmd and args
    
    void playNextScriptLine(); // should be private

    void playScript(uint8_t scriptid, uint8_t reps, uint8_t pos);
    void playScript(uint8_t scriptid) { playScript(scriptid, 0, 0);  }
    void stop() { playing = false; }
    void off();

};

#endif
