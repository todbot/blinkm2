
static inline void adc_channel(uint8_t channel){
    ADMUX &= ~(0b11111);
    ADMUX |=   0b11111 & channel;
}

static inline uint16_t adc_get(void){
    ADCSRA |= (1<<ADSC); //start conversion
    while(!(ADCSRA & (1<<ADIF))); //wait for conversion to finish
    ADCSRA |= (1<<ADIF); //reset the flag
    return ADC; //return value
}

uint16_t touch_measure(touch_channel_t *channel){
    uint8_t i;
    uint16_t retval;

    retval = 0;

    //Do four measurements and average, just to smooth things out
    for (i=0 ; i<4 ; i++){
        *(channel->port) |= channel->portmask;    // set pullup on
        _delay_ms(1);                             // wait (could probably be shorter)
        *(channel->port) &= ~(channel->portmask); // set pullup off

        adc_channel(0b11111); //set ADC mux to ground;
        adc_get();            //do a measurement (to discharge the sampling cap)

        adc_channel(channel->mux); //set mux to right channel
        retval +=  adc_get(); //do a conversion
    }
    return retval /4;
