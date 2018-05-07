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

#include <stdio.h>

#include "DS1307.h"
#include "Driver_USART.h"

uint8_t EEMEM pwm_time[] = {20,30};
uint8_t EEMEM intensity = 12;

uint8_t time_compare(TIME,TIME);
void normal_routine(void);
void command_routine(void);

TIME int_time;
volatile uint8_t pwm_set=0;

int main(void)
{
	PORTD = 0x80;
	if ((PIND & 0x80) == 0)
		command_routine();
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
    pwm_mode_time.hour = eeprom_read_byte(&pwm_time[0]);
 	eeprom_busy_wait();
	pwm_mode_time.min = eeprom_read_byte(&pwm_time[1]);
    pwm_mode_time.sec = 0;
    pwm_set=0;
   	eeprom_busy_wait();
	intensity_percent = eeprom_read_byte(&intensity);
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
	char cmd[10], exit_loop = 0;
	uint8_t temp[3];
	
	USART_Initialise(USART_Character_Size_8|USART_Tx_Enable|USART_Rx_Enable);
	
	while (1)
	{
		USART_printf("1.SetTime\r");
		USART_printf("2.ShowTime\r");
		USART_printf("3.SetPWMTime\r");
		USART_printf("4.ShowPWMTime\r");
		USART_printf("5.SetIntensity\r");
		USART_printf("6.ShowIntensity\r");
		USART_printf("7.Exit\r");
		USART_Receive(cmd,1);
		while(USART_Status_Flag & USART_Rx_Busy){}
			
		switch(cmd[0]){
			case '1':
				USART_printf("\rSetTime as HH MM SS\r");
				USART_Receive(cmd,8);
				while(USART_Status_Flag & USART_Rx_Busy){}
				cmd[8]=0;
				sscanf(cmd, "%d%d%d", temp, temp+1, temp+2);
				int_time.hour = temp[0];
				int_time.min = temp[1];
				int_time.sec = temp[2];
				DS1307_Set_Time(&int_time);
				USART_printf("Time is set\r",int_time.hour,int_time.min,int_time.sec);
				break;
			case '2':
				DS1307_Read_Time(&int_time);
				USART_printf("%d:%d:%d\r",int_time.hour,int_time.min,int_time.sec);
				break;
			case '3':
				USART_printf("\rSetPWMTime as HH MM\r");
				USART_Receive(cmd,5);
				while(USART_Status_Flag & USART_Rx_Busy){}
				cmd[5]=0;
				sscanf(cmd, "%d%d", (int *)&temp[0],(int *)&temp[1]);
				eeprom_busy_wait();
				eeprom_write_block(temp,pwm_time,2);
				USART_printf("Time is set\r");
				break;
			case '4':
				eeprom_busy_wait();
				eeprom_read_block(temp,pwm_time,2);
				USART_printf("%d:%d\r",temp[0],temp[1]);
				break;
			case '5':
				USART_Transmit("\rSetIntensity as xx\r",20);
				USART_Receive(cmd,2);
				while(USART_Status_Flag & USART_Rx_Busy){}
				cmd[2]=0;
				sscanf(cmd, "%d", (int *)&temp[0]);
				eeprom_busy_wait();
				eeprom_write_byte(&intensity,temp[0]);
				USART_printf("Intensity Set\r");
				break;
			case '6':
				eeprom_busy_wait();
				temp[0]=eeprom_read_byte(&intensity);
				USART_printf("%d\r",temp[0]);
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
