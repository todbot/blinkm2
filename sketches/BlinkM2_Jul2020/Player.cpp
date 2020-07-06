/**
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
     
#include "blinkm_config.h"

#include "Player.h"
#include "LedScripts.h"

#include "Arduino.h"

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


void Player::doFadersOld()
{
    // // global fader
    // if( led_dest_global != leds[19] ) { // FIXME: bad hack
    //     rgb_t l = leds[19];
    //     rgb_t ld = led_dest_global;
    //     rgb_t lnew = rgb_t( colorSlide(l.r, ld.r, fadespeed),
    //                         colorSlide(l.g, ld.g, fadespeed),
    //                         colorSlide(l.b, ld.b, fadespeed) );
    //     for( int i=0; i<nLEDs; i++ ) {
    //         leds[i] = lnew;
    //     }
    // }
    
    // // individual faders
    // for( int i=0; i<faderMax; i++) { 
    //     rgb_t l = leds[i];   // current value
    //     rgb_t ld = led_dests[i]; // destination value
    //     leds[ i ] = rgb_t( colorSlide( l.r, ld.r, fadespeed),
    //                        colorSlide( l.g, ld.g, fadespeed),
    //                        colorSlide( l.b, ld.b, fadespeed) );
    // }
}

// run faders, only first 0..faderMax LEDs have faders
void Player::doFaders()
{
    for( int i=0; i< nLEDs; i++) { 
        rgb_t l = leds[i];   // current value
        rgb_t ld = led_dests[i]; // destination value
        leds[ i ] = rgb_t( colorSlide( l.r, ld.r, fadespeed),
                           colorSlide( l.g, ld.g, fadespeed),
                           colorSlide( l.b, ld.b, fadespeed) );
    }
}   

// Call this periodically
void Player::update(void)
{
    if( !playing ) { return;  }

    uint32_t now = millis();
    if( (long)(now - tickUpdateNext) < 0 ) { // not time yet
        return;
    }
    tickUpdateNext = now + 10;
    tick++;

    doFaders();

    if( tick == 3 ) { // roughly 1/30th second FIXME
        tick=0;
        scriptTick++;
    }

    if( scriptTick < script_curr.dur ) { // ready to go to next line?
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

//
void Player::playScript(uint8_t scriptid, uint8_t reps, uint8_t pos)
{
    dbg("playScript:"); dbg(scriptid); dbg(','); dbg(reps); dbg(','); dbgln(pos);
    scriptId = scriptid;
    playPos = pos;
    scriptTick = 0;
    scriptReps = reps;
    playing = true;
    scriptLen = (scriptId>0) ? script_lengths[scriptId-1] : 16; // FIXME: needs EEPROM scriptlen 
    //fadespeed = 3;
    playNextScriptLine();
}

//
void Player::playNextScriptLine()
{
    if( scriptId == 0 ) {   // eeprom
        // eeprom_read_block( &script_curr, &ee_script_lines[playPos],
        //                     sizeof(script_line_t));
    }
    else {                  // flash
        //script_line_t* sl;
        // get pointer to scriptline set
        //memcpy_P(&sl, &scripts[scriptId-1], sizeof(script_line_t*));
        //const script_line_t* sl =  scripts_flash[scriptId-1];
        // then, get next script line at current script position
        //memcpy_P(&script_curr, &sl, sizeof(script_line_t));
        

        const script_line_t* sl = scripts[scriptId-1] + playPos;
        
        //const script_line_t* sl = script_rgb + playPos;
        memcpy_P(&script_curr, sl, sizeof(script_line_t));
        
        // this works
        // memcpy_P(&script_curr, script_rgb + playPos, sizeof(script_line_t));

        dbg("scriptId:"); dbg(scriptId); dbg(" scriptLen:"); dbg(scriptLen); dbg(" playPos:"); dbg(playPos);
        //dbg(" flash line:"); dbg(sizeof(script_line_t*)); dbg(":"); dbgln(sizeof(script_line_t));
        dbg("   cmd:"); dbg(script_curr.cmd);
        dbg(" args:"); dbg(script_curr.args[0]);
        dbg(","); dbg(script_curr.args[1]); dbg(","); dbgln(script_curr.args[2]);
    }
    
    cmd = script_curr.cmd;
    args[0] = script_curr.args[0];
    args[1] = script_curr.args[1];
    args[2] = script_curr.args[2];

    handleCmd();
}

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
    dbg("handleCmd:"); dbg(scriptId); dbg(':'); dbg(playPos); dbg('/'); dbg(scriptLen);
    dbg("  cmd:"); dbg(cmd); dbg(" args:"); dbg(args[0]); dbg(","); dbg(args[1]); dbg(","); dbgln(args[2]);

    if( cmd == 'l' ) { // set which LED to operate on
        dbgln("@@@ ledn!");
        ledn = args[0]; 
        if( ledn > faderMax ) {
            ledn = 0;
        }
    }
    else if( cmd == 'n' ) { // set rgb color now
        rgb_t newc = rgb_t(args[0], args[1], args[2]);
        if( ledn == 0 ) {
            for( int i=0 ; i< nLEDs; i++) { 
                leds[i] = newc;
                led_dests[i] = newc;
            }
        }
        else {
            leds[ ledn-1 ] = newc;
            led_dests[ ledn-1 ] = newc;
        }
    }
    else if( cmd == 'c' ) { // fade to rgb color
        rgb_t newc = rgb_t(args[0], args[1], args[2]);
        if( ledn == 0 ) { // change all LEDs?
            for( int i=0 ; i< nLEDs; i++) { 
                led_dests[ i ] = newc;
            }
        }
        else {           // or change one LED
            led_dests[ ledn-1 ] = newc;
        }
    }
    else if( cmd == 'C' ) {  // fade to random rgb color

    }
    else if( cmd == 'h' ) {  // fade to HSV
        uint8_t r,g,b;
        hsvToRgb( args[0], args[1], args[2], &r, &g, &b);
        led_dests[ledn] = rgb_t(r,g,b); // fixme: make better
    }
    else if( cmd == 'H' ) {  // fade to random HSV
        
    }
    else if( cmd == 'p' ) {  // play script
        playScript( args[0], args[1], args[2] );
    }
    else if( cmd == 'f' ) {  // set fadespeed
        fadespeed = args[0];
    }
    else if( cmd == 't' ) {  // time adjust
        timeadj = args[0];
    }
    else if( cmd == 'F' ) { // change fade speed on input

    }
    else if( cmd == 'j' )  { // jump back relative
        playPos += args[0] - 1;
    }
    else if( cmd == 'i' ) { // read inputs and act
        if( inputs[ args[0] ] > args[1] ) {
            playPos += args[2] - 1;
        }
    }
}

// liner interpolation (lerp) of curr to dest by step amount
// TODO: look into 'lerp8by8()' in FastLED/lib8tion.h
uint8_t Player::colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
{
    int diff = curr - dest;
    if(diff < 0)  diff = -diff;

    if( diff <= step ) return dest;
    if( curr == dest ) return dest;
    else if( curr < dest ) return curr + step;
    else                   return curr - step;
}

// from https://stackoverflow.com/a/22120275/221735
void Player::hsvToRgb(uint8_t oh, uint8_t os, uint8_t ov,
                      uint8_t* r, uint8_t* g, uint8_t* b)
{
    unsigned char region, p, q, t;
    unsigned int h, s, v, remainder;

    if (os == 0) {
        *r = ov;  *g = ov;  *b = ov;
        return;
    }

    // converting to 16 bit to prevent overflow
    h = oh;  s = os;   v = ov;

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            *r = v;  *g = t;  *b = p;
            break;
        case 1:
            *r = q;  *g = v;  *b = p;
            break;
        case 2:
            *r = p;  *g = v;  *b = t;
            break;
        case 3:
            *r = p;  *g = q;  *b = v;
            break;
        case 4:
            *r = t;  *g = p;  *b = v;
            break;
        default:
            *r = v;  *g = p;  *b = q;
            break;
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

