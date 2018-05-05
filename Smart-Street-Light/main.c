/*
 * Smart-Street-Light.c
 *
 * Created: 05-05-2018 17:05:32
 * Author : vivek
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include "DS1307.h"
#include "Driver_USART.h"
#include <util/delay.h>

uint8_t time_compare(TIME,TIME);

TIME int_time;
volatile uint8_t pwm_set=0;

int main(void)
{
    TIME time, pwm_mode_time;
	uint8_t intensity_percent = 5, person_passed;
	//char to_print[10];

	pwm_mode_time.hour = 19;
	pwm_mode_time.min = 23;
	pwm_mode_time.sec = 0;
	pwm_set=0;
	
	DDRB |= 1<<3;
	PORTB |= 1<<3;
	
	GICR=1<<INT0;			// External Interrupt Enable
	MCUCR=1<<ISC01|1<<ISC00;	// Rising Edge
	TCCR2 = 0x68;
	
	sei();
	while (1)
	{
		DS1307_Read_Time(&time);
		person_passed = time.sec-int_time.sec;
		if(person_passed<0)	person_passed += 60;
		if((!pwm_set) && time_compare(time,pwm_mode_time) && person_passed>10){
			OCR2 = 255 * intensity_percent / 100;
			TCCR2 = 0x6B;
			pwm_set = 1;
		}
		_delay_ms(3000);
	}
}

ISR(INT0_vect){
	OCR2 = 255;
	DS1307_Read_Time(&int_time);
	pwm_set=0;
}

uint8_t time_compare(TIME pre_time,TIME ref_time){
	int present = pre_time.hour<<6 | pre_time.min;
	int ref = ref_time.hour<<6 | ref_time.min;
	return (present-ref)>0?1:0;
}

	//uint8_t temp = 255 * intensity_percent / 100;
	//USART_Initialise(USART_Character_Size_8|USART_Tx_Enable|USART_Rx_Enable);
	//
	//sprintf(to_print,"%d",temp);
	//USART_Transmit(to_print,10);
