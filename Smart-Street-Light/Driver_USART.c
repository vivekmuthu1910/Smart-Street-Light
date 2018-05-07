/*
 * Driver_USART.c
 *
 * Created: 03-05-2018 13:14:44
 *  Author: vivek
 */ 


#include <avr/io.h>
#include "Driver_USART.h"
#include <util/setbaud.h>

uint8_t *transmit_data, *receive_data, transmit_data_len=0, receive_data_len=0;

ISR(USART_UDRE_vect){
	static uint8_t transmited_data_len=0;
	transmited_data_len++;
	if(transmited_data_len < transmit_data_len){
		UDR = transmit_data[transmited_data_len];
	}
	else {
		UCSRB &= ~(1<<UDRIE);
		USART_Status_Flag &= ~USART_Tx_Busy;
		transmited_data_len = 0;
	}
}

ISR(USART_RXC_vect){
	static uint8_t received_data_len=0;
	receive_data[received_data_len] = UDR;
	received_data_len++;
	if(received_data_len >= receive_data_len){
		UCSRB &= ~(1<<RXCIE);
		USART_Status_Flag &= ~USART_Rx_Busy;
		received_data_len=0;
	}
}

void USART_Initialise(uint16_t Control){
	cli();
	UCSRC = 1<<7 | (Control & 0xFF);
	UCSRB = (Control>>8) & 0xFF;
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
#if USE_2X
	UCSRA |= (1 << U2X);
#else
	UCSRA &= ~(1 << U2X);
#endif
	sei();
	USART_Status_Flag = 0;
}

void USART_Transmit(void * data, uint8_t len){
	transmit_data = (uint8_t *)data;
	transmit_data_len = len;
	UDR = transmit_data[0];
	UCSRB |= 1<<UDRIE;
	USART_Status_Flag |= USART_Tx_Busy;
}

void USART_Receive(void * data, uint8_t len){
	receive_data = (uint8_t *)data;
	receive_data_len = len;
	UCSRB |= 1<<RXCIE;
	USART_Status_Flag |= USART_Rx_Busy;
}

void USART_printf(const char *fmt, ...){
	char out[30], len;
	va_list ap;
	va_start(ap,fmt);
	len = vsprintf(out,fmt,ap);
	va_end(ap);
	USART_Transmit(out,len);
	while(USART_Status_Flag & USART_Tx_Busy){}
}