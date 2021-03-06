/**
 * Player -- Part of BlinkM2
 * 2018-2020 @todbot / Tod E. Kurt 
 *
 * Theory of operation:
 * - There is a Player object that owns the ability to play light scripts
 * - The Player also handles the LED fading 
 * - All commands over I2C can also be light script commands
 * - There is an 'ledn' parameter that specifies an LED to operate on
 *   - If ledn==0, operate fade on all LEDs
 *   - If ledn==1..faderMax, operate fade on LEDs 0..faderMax-1
 *
 */
     
#include "Arduino.h"

#include "Player.h"
#include "LedScripts.h"

#include "utils.h"

// FIXME: don't like either of these
#include <avr/eeprom.h>

extern ee_mem EEMEM ee;

// run faders, only first 0..faderMax LEDs have faders
void Player::doFaders()
{
#if 1
    for( int i=0; i< nLEDs; i++) { 
        leds[i].slideTowards(led_dests[i], fadespeed);
    }
#else
    // old way
    for( int i=0; i< nLEDs; i++) { 
        rgb_t l = leds[i];   // current value
        rgb_t ld = led_dests[i]; // destination value
        leds[ i ] = rgb_t( colorSlide( l.r, ld.r, fadespeed),
                           colorSlide( l.g, ld.g, fadespeed),
                           colorSlide( l.b, ld.b, fadespeed) );
    }
#endif
}   

//#define tickfudge 4
// Call this periodically
void Player::update(void)
{
    // uint32_t now = millis();
    // if( (long)(now - tickUpdateNext) < 0 ) { // not time yet
    //     return;
    // }
    // tickUpdateNext = now + 33 - tickfudge;  // was 10, because thought we wanted 10 between ticks, but maybe 3?
    //tick++;
    
    doFaders();

    if( !playing ) { return; }

    tick++;
    if( tick == 3 ) { // roughly 1/30th second FIXME
        tick=0;
        scriptTick++;
    }

    // FIXME: only place script_curr is used outside of loading line
    if( scriptTick < line_cur.dur ) { // ready to go to next line?
        return; // no
    }
    scriptTick = 0; // reset our ticker

    playPos++;    // go to next line
    if( playPos == scriptLen ) { // at end of script?
        playPos = 0;
        scriptReps--;  // repeat
        if( scriptReps == 255 ) { // if wrapped, we're infinite reps
            scriptReps = 0;
            playNextScriptLine();
        }
        else if( scriptReps == 0 ) { // no more reps, done playing
            playing = false;
            return; // FIXME: correct behavior? no other cleanup?
        }
        else {
            playNextScriptLine();
        }
    }
    else { // not at end of scriptId
        playNextScriptLine();
    }
}

// Given a script id, return the length of that script
uint8_t getScriptLen(uint8_t scriptId)
{
    uint8_t len = 0;
    if( scriptId == 0 ) {
        len = eeprom_read_byte( &ee.config.script_len );
    }
    else {
        //len = script_lengths[scriptId-1]; // RAM
        len = pgm_read_byte( &script_lengths[scriptId-1] );
    }
    //if( len==0 || len > 16 ) len = 16;
    dbg("len:"); dbgln(len);
    return len;
}

//
void Player::playScript(uint8_t scriptid, uint8_t reps, uint8_t pos )
{
    dbg("playScript:"); dbg(scriptid); dbg(','); dbg(reps); dbg(','); dbgln(pos);
    scriptId = scriptid;
    playPos = pos;
    scriptTick = 0;
    scriptReps = reps;
    playing = true;
    scriptLen = getScriptLen(scriptId);
    playNextScriptLine();
}

//
void Player::playNextScriptLine()
{
    //script_line_t script_curr; 
    if( scriptId == 0 ) {   // eeprom
        eeprom_read_block( &line_cur, &ee.script_lines[playPos],
                           sizeof(script_line_t));
    }
    else {                  // flash
        // first get pointer to scriptline set
        script_line_t* sl = pgm_read_ptr( &scripts[scriptId-1] );
        sl += playPos;         // ofset pointer to current play position
        // read actual script line
        memcpy_P(&line_cur, sl, sizeof(script_line_t));
    }
    /*
    cmd     = script_curr.cmd;
    args[0] = script_curr.args[0];
    args[1] = script_curr.args[1];
    args[2] = script_curr.args[2];
    dur     = script_curr.dur;
    */
    handleCmd();
}

//
void Player::handleCmd(uint8_t c, uint8_t a0, uint8_t a1, uint8_t a2)
{
    setCmd(c);
    setArgs(a0,a1,a2);
    handleCmd();
}

// handle the current script line, or a command set up via setCmd() & setAargs()
//  lights leds, etc.
void Player::handleCmd()
{
    dbg("handleCmd:");
    dbg(scriptId); dbg(':'); dbg(line_cur.dur); dbg(':'); dbg(playPos); dbg('/'); dbg(scriptLen);
    dbg("  cmd:"); dbg((char)line_cur.cmd);
    dbg(" args:"); dbg(line_cur.args[0]); dbg(","); dbg(line_cur.args[1]); dbg(","); dbg(line_cur.args[2]);
    dbgln();
    //dbg("freeMem:"); dbgln(freeMemory());

    int i; rgb_t lastc, newc;

    switch(line_cur.cmd) {
    case '.':                 // set which LED to operate on  BLINKM2
        //dbg("@@@ ledn! "); dbgln(args[0]);
        ledn = line_cur.args[0]; 
        if( ledn >= nLEDs ) { ledn = 0; }
        break;

    case 'b':                 // set brightness               BLINKM2
        brightness = line_cur.args[0];
        /*
        for( int i=0; i<nLEDs; i++) {
            leds[i].dim(brightness);
            led_dests[i].dim(brightness);
        }
        */
        break;

    case '>':                 // move leds to the right      BLINKM2
        for( int i=i; i< nLEDs-1; i++ ) {
            leds[i] = leds[i+1];
        }
        leds[0] = leds[nLEDs-1];                
        break;

    case 'n':                 // set rgb color now
        newc = rgb_t(line_cur.args[0], line_cur.args[1], line_cur.args[2]);
        if( ledn == 0 ) {
            for( int i=0 ; i< nLEDs; i++) { 
                leds[i] = led_dests[i] = newc;
            }
        }
        else {
            leds[ ledn-1 ] = led_dests[ ledn-1 ] = newc;
        }
        break;

    case 'c':                 // fade to rgb color
        newc = rgb_t(line_cur.args[0], line_cur.args[1], line_cur.args[2]);
        newc.dim(brightness);
        if( ledn == 0 ) { // change all LEDs?
            for( int i=0 ; i< nLEDs; i++) { 
                led_dests[ i ] = newc;
            }
        }
        else {                // or change one LED
            led_dests[ ledn-1 ] = newc;
        }        
        break;

    case 'C':                 // fade to random rgb color
        // fixme:
        i = (ledn==0) ? 0 : ledn-1;
        lastc = led_dests[i];
        newc = rgb_t( get_rand_range(lastc.r, line_cur.args[0]),
                            get_rand_range(lastc.g, line_cur.args[1]),
                            get_rand_range(lastc.b, line_cur.args[2]) );
        led_dests[i] = newc;
        dbg("lastc:");dbg(lastc.r);dbg(',');dbg(lastc.g);dbg(',');dbg(lastc.b);
        dbg(" newc:");dbg(newc.r);dbg(',');dbg(newc.g);dbg(',');dbg(newc.b); dbgln();        
        break;

    case 'h':                 // fade to HSV
        uint8_t r,g,b;
        hsvToRgb( line_cur.args[0], line_cur.args[1], line_cur.args[2], &r, &g, &b);
        led_dests[ledn] = rgb_t(r,g,b); // fixme: make better
        break;

    case 'H':                 // fade to random HSV
        i = (ledn==0) ? 0 : ledn-1;
        dbg("last hsv:");dbg(hue);dbg(',');dbg(sat);dbg(',');dbg(bri);
        hue = get_rand_range( hue, line_cur.args[0] );
        sat = get_rand_range( sat, line_cur.args[1] );
        bri = get_rand_range( bri, line_cur.args[2] );
        
        hsvToRgb( hue,sat,bri, &newc.r, &newc.g, &newc.b );
        led_dests[i] = newc;
        dbg(" new hsv:");dbg(hue);dbg(',');dbg(sat);dbg(',');dbg(bri); dbgln();        
        break;

    case 'p':                 // play script
        playScript( line_cur.args[0], line_cur.args[1], line_cur.args[2] );
        break;
        
    case 'f':                 // set fadespeed
        fadespeed = line_cur.args[0];
        break;

    case 't':                 // time adjust
        timeadj = line_cur.args[0];
        break;

    case('T'):                // random time delay   // FIXME: test this
        line_cur.dur = get_rand_range( line_cur.dur, line_cur.args[0] );
        break;

    case 'F':                 // change fade speed on input // FIXME:
        
        break;

    case 'j':                 // jump back relative
        playPos += line_cur.args[0] - 1;        
        break;

    case 'i':                // read inputs and act
        if( inputs[ line_cur.args[0] ] > line_cur.args[1] ) {
            playPos += line_cur.args[2] - 1;
        }
        break;

    case 'o':                 // stop playback  FIXME: check if this is in spec
        stop();
        break;

    }
}

// 
void Player::off()
{
    stop();
    rgb_t o = rgb_t(0,0,0);
    for( int i=0; i<nLEDs; i++) {
        leds[i] = o;
    }
    for( int i=0; i<faderMax; i++) { // FIXME?
        led_dests[i] = o;
    }
}



// void Player::setStripUpdater( callback_t callback )
// {
//     stripUpdater = callback;
// }

// // fader logic: add current 'ledn' to fader pool
// void Player::addFader()
// {
//     // search to see if fader already in use
//     for( int i=0; i< faderMax; i++) {
//         if( ledns[i] == ledn ) { // already in use, do nothing
//             return;
//         }
//     }
//     // search for unused slot
//     for( int i=0; i< faderMax; i++ ) {
//         if( ledns[i] == 255 ) { // found unused, so use it
//             ledns[i] = ledn;
//             return;
//         }
//     }
//     // else we are boned. what to do now?
// }

// // run fader logic.  if at end of fade, fader is freed
// void Player::doFadersOrig()
// {
//     dbg("player.doFaders:"); dbgln(ledns[0]);
//     for( int i=0; i< faderMax; i++) {
//         rgb_t ld = led_dests[ i ];  // get dest color from fader pool
//         uint8_t ln = ledns[i];      // get an ledn from fader pool
//         if( ln == 255 ) { continue; }  // 255=unused, go to next ledn
//         rgb_t l = leds[ ln ];       // get LED to modify
//         // check if we're done fading, if so, release fader
//         if( l.r == ld.r && l.g == ld.g && l.b == ld.b ) {
//             ledns[ i ] = 255;  // we're done, release fader
//         }
//         else {  // otherwise, do fade
//             leds[ ln ] = rgb_t( colorSlide( l.r, ld.r, fadespeed),
//                                 colorSlide( l.g, ld.g, fadespeed),
//                                 colorSlide( l.b, ld.b, fadespeed) );
//         }
//     } // for
// }

// // run fader logic.  if at end of fade, fader is freed
// void Player::doFaders()
// {
//     dbg("player.doFaders:"); dbg(leds[0].r); dbg(','); dbg(leds[0].g);dbg(','); dbgln(leds[0].b);
//     for( int i=0; i< faderMax; i++) {
//         rgb_t ld = led_dests[ i ];  // get dest color from fader pool
//         uint8_t ln = ledns[i];      // get an ledn from fader pool
//         if( ln == 255 ) { continue; }  // 255=unused, go to next ledn
//         rgb_t l = leds[ ln ];       // get LED to modify

//     }
//     uint8_t ledn = ledns[0];
//     rgb_t l = leds[ledn];
//     rgb_t ld = led_dest;
    
//     // check if we're done fading
//     //if( l.r == ld.r && l.g == ld.g && l.b == ld.b ) {
//     //ledns[ i ] = 255;  // we're done, release fader
//     //}
//     //else {  // otherwise, do fade
//     leds[ ledn ] = rgb_t( colorSlide( l.r, ld.r, fadespeed),
//                           colorSlide( l.g, ld.g, fadespeed),
//                           colorSlide( l.b, ld.b, fadespeed) );
// }

