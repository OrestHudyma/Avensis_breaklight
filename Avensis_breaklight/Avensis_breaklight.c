/*
 * Avensis_breaklight.c
 *
 * Created: 09.12.2016 12:36:00
 *  Author: OHUD
 */ 

#include <avr/io.h>


#define default_port_config		0b00010111
#define F_CPU					9600000UL // CPU frequency 9.6 MHz
#include <util/delay.h>

#define channel_0	1<<PINB0
#define channel_1	1<<PINB4

#define pwm_period				254
#define low_temp				131
#define high_temp				140
#define uint8_max				255
#define number_of_channels		2

unsigned char pwm_count;
unsigned char temp_ratio;
unsigned char temp;
unsigned char channel_0_start, channel_0_stop;
unsigned char channel_1_start, channel_1_stop;

int relax_delay;
unsigned char duty_cycle = pwm_period;
unsigned char duty_cycle_new;

void turn_on(char channel);
void turn_off(char channel);
unsigned char get_temp();

int main(void)
{
	DDRB = default_port_config;
	PORTB |= default_port_config; // Fast turn on for all channels

	ADCSRA |= (0 << ADPS2) | (0 << ADPS1) | (0 << ADPS0); // Set ADC prescallar to 1
	ADMUX |= (0 << REFS0); // Set ADC reference to AVCC
	ADMUX |= (1 << ADLAR); // Switch to left adjusted result for 8-bit usage
	ADCSRA |= (1 << ADEN);  // Enable ADC
	
	temp_ratio = pwm_period / (high_temp - low_temp);

    while(1)
    {       	   
	   temp = get_temp();
	   
	   if (temp > low_temp)
	   {		   
		   duty_cycle_new = ((high_temp - low_temp) - (temp - low_temp)) * temp_ratio;
		   duty_cycle += (duty_cycle < duty_cycle_new) - (duty_cycle > duty_cycle_new);		   
		   if(temp > high_temp) {duty_cycle = 0;}			
		   relax_delay = (pwm_period - number_of_channels * duty_cycle) / number_of_channels;

		   channel_0_start = 0;
		   channel_0_stop = channel_0_start + duty_cycle;

		   channel_1_start = channel_0_stop + relax_delay;
		   channel_1_stop = channel_1_start + duty_cycle;
		   
		   for(pwm_count = 0; pwm_count <= pwm_period; pwm_count++)
		   {
			   if(pwm_count == channel_0_start) {turn_on(channel_0);}
			   if(pwm_count == channel_0_stop) {turn_off(channel_0);}
			   if(pwm_count == channel_1_start) {turn_on(channel_1);}
			   if(pwm_count == channel_1_stop) {turn_off(channel_1);}
		   }
	   }
	   else
	   {
		   PORTB |= default_port_config; // Fast turn on for all channels
	   }
	   
    }
}

void turn_on(char channel)
{
	PORTB |= channel;
}

void turn_off(char channel)
{
	PORTB &= ~channel;
}

unsigned char get_temp()
{
	// Connect AMUX to ADC3 (PB3)
	ADMUX |= (1 << MUX0);
	ADMUX |= (1 << MUX1);
	ADCSRA |= (1 << ADSC);  // Start conversion
	while(ADCSRA & (1 << ADSC)) {} // Wait for conversion complete
	
	return uint8_max - ADCH;
}
