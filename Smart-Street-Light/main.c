/*
 * Smart-Street-Light.c
 *
 * Created: 05-05-2018 17:05:32
 * Author : vivek
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "DS1307.h"
#include "Driver_USART.h"

uint8_t time_compare(TIME,TIME);
void normal_routine(void);
void command_routine(void);

TIME int_time;
volatile uint8_t pwm_set=0;

int main(void)
{
	if ((PIND7 & 0x80) == 0)
	{
		command_routine();
	}
	normal_routine();
}

ISR(INT0_vect){
	OCR2 = 255;
	DS1307_Read_Time(&int_time);
	pwm_set=0;
}

void normal_routine(void){
	TIME time, pwm_mode_time;
    uint8_t intensity_percent, person_passed;

	eeprom_busy_wait();
    pwm_mode_time.hour = eeprom_read_byte((const uint8_t *)0);
 	eeprom_busy_wait();
	pwm_mode_time.min = eeprom_read_byte((const uint8_t *)1);
    pwm_mode_time.sec = 0;
    pwm_set=0;
   	eeprom_busy_wait();
	intensity_percent = eeprom_read_byte((const uint8_t *)5);
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

void command_routine(){
	char cmd[15], exit_loop = 0;
	uint8_t temp_time[3];
	char Msg[] = "\r1.SetTime\r2.ShowTime\r3.SetPWMTime\r4.ShowPWMTime\r5.SetIntensity\r6.ShowIntensity\r7.Exit\r";
	uint8_t msg_len = strlen(Msg);
	
	USART_Initialise(USART_Character_Size_8|USART_Tx_Enable|USART_Rx_Enable);

	while (1)
	{
		while(USART_Status_Flag & USART_Tx_Busy){}
		USART_Transmit(Msg,msg_len);
		USART_Receive(cmd,1);
		while(USART_Status_Flag & USART_Rx_Busy){}
		switch(cmd[0]){
			case '1':
				USART_Transmit("\rSetTime as HH MM SS\r",21);
				USART_Receive(cmd,8);
				while(USART_Status_Flag & USART_Rx_Busy){}
				sscanf(cmd, "%d%d%d", (int *)&temp_time[0], (int *)&temp_time[1], (int *)&temp_time[2]);
				int_time.hour=temp_time[0];
				int_time.min=temp_time[1];
				int_time.sec=temp_time[2];
				DS1307_Set_Time(&int_time);
				USART_Transmit("\rTime is set\r",13);
				break;
			case '2':
				DS1307_Read_Time(&int_time);
				sprintf(cmd,"\r%02d:%02d:%02d\r",int_time.hour,int_time.min,int_time.sec);
				USART_Transmit(cmd,10);
				break;
			case '3':
				USART_Transmit("\rSetPWMTime as HH MM\r",21);
				USART_Receive(cmd,5);
				while(USART_Status_Flag & USART_Rx_Busy){}
				sscanf(cmd, "%d%d", (int *)&temp_time[0],(int *)&temp_time[1]);
				eeprom_busy_wait();
				eeprom_update_block(temp_time,(void *)0,2);
				USART_Transmit("\rTime is set\r",13);
				break;
			case '4':
				eeprom_busy_wait();
				eeprom_read_block(temp_time,(const void *)0,2);
				sprintf(cmd,"\r%02d:%02d\r",temp_time[0],temp_time[1]);
				USART_Transmit(cmd,7);
				break;
			case '5':
				USART_Transmit("\rSetIntensity as xx\r",20);
				USART_Receive(cmd,2);
				while(USART_Status_Flag & USART_Rx_Busy){}
				sscanf(cmd, "%d", (int *)&temp_time[0]);
				eeprom_busy_wait();
				eeprom_write_byte((uint8_t *)5,temp_time[0]);
				USART_Transmit("\rIntensity Set\r",15);
				break;
			case '6':
				eeprom_busy_wait();
				temp_time[0]=eeprom_read_byte((const uint8_t*)5);
				sprintf(cmd,"\r%02d",temp_time[0]);
				USART_Transmit(cmd,3);
				break;
			case '7': exit_loop=1;
		}
		if (exit_loop)
		{
			break;
		}
	}
}

uint8_t time_compare(TIME pre_time,TIME ref_time){
	int present = pre_time.hour<<6 | pre_time.min;
	int ref = ref_time.hour<<6 | ref_time.min;
	return (present-ref)>0?1:0;
}
