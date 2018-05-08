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
void command_routine(void);

TIME int_time;
volatile uint8_t pwm_set=0;

int main(void)
{
	USART_Initialise(USART_Character_Size_8|USART_Tx_Enable|USART_Rx_Enable);
	PORTB |= 1<<4;
	if ((PINB & 0x10)==0)
	{
		command_routine();
	}
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
	//USART_printf("PWM Timing: %d:%d\n",pwm_mode_time.hour,pwm_mode_time.min);
	//USART_printf("Intensity: %d\n",intensity_percent);
	intensity_percent = 255 * intensity_percent / 100;

	DDRB |= 1<<3;
	PORTB |= 1<<3;
	GICR=3<<INT0;			// External Interrupt Enable
	MCUCR=0xF;	// Rising Edge
	TCCR2 = 0x68;
	sei();
	while (1)
	{
		DS1307_Read_Time(&time);
		person_passed = time.sec-int_time.sec;
		if(person_passed<0)	person_passed += 60;
		if((!pwm_set) && time_compare(time,pwm_mode_time) && person_passed>15){
			OCR2 = intensity_percent;
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

void command_routine(){
	char cmd[10], exit_loop = 0;
	uint8_t temp[3];
	while (1)
	{
		USART_printf("1.SetTime\n");
		USART_printf("2.ShowTime\n");
		USART_printf("3.SetPWMTime\n");
		USART_printf("4.ShowPWMTime\n");
		USART_printf("5.SetIntensity\n");
		USART_printf("6.ShowIntensity\n");
		USART_printf("7.Exit\n");
		USART_Receive(cmd,1);
		while(USART_Status_Flag & USART_Rx_Busy){}

		switch(cmd[0]){
			case '1':
				USART_printf("\nSetTime as HH MM SS\n");
				USART_Receive(cmd,8);
				while(USART_Status_Flag & USART_Rx_Busy){}
				cmd[8]=0;
				sscanf(cmd, "%d%d%d", temp, temp+1, temp+2);
				int_time.hour = temp[0];
				int_time.min = temp[1];
				int_time.sec = temp[2];
				DS1307_Set_Time(&int_time);
				USART_printf("Time is set\n",int_time.hour,int_time.min,int_time.sec);
				break;
			case '2':
				DS1307_Read_Time(&int_time);
				USART_printf("%d:%d:%d\n",int_time.hour,int_time.min,int_time.sec);
				break;
			case '3':
				USART_printf("\nSetPWMTime as HH MM\n");
				USART_Receive(cmd,5);
				while(USART_Status_Flag & USART_Rx_Busy){}
				cmd[5]=0;
				sscanf(cmd, "%d%d", (int *)&temp[0],(int *)&temp[1]);
				eeprom_busy_wait();
				eeprom_write_block(temp,pwm_time,2);
				USART_printf("Time is set\n");
				break;
			case '4':
				eeprom_busy_wait();
				eeprom_read_block(temp,pwm_time,2);
				USART_printf("%d:%d\n",temp[0],temp[1]);
				break;
			case '5':
				USART_Transmit("\nSetIntensity as xx\n",20);
				USART_Receive(cmd,2);
				while(USART_Status_Flag & USART_Rx_Busy){}
				cmd[2]=0;
				sscanf(cmd, "%d", (int *)&temp[0]);
				eeprom_busy_wait();
				eeprom_write_byte(&intensity,temp[0]);
				USART_printf("Intensity Set\n");
				break;
			case '6':
				eeprom_busy_wait();
				temp[0]=eeprom_read_byte(&intensity);
				USART_printf("%d\n",temp[0]);
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
