/*
 * Driver_USART.h
 *
 * Created: 07-04-2018 11:05:20
 *  Author: vivek
 */ 


#ifndef DRIVER_USART_H_
#define DRIVER_USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>


#ifndef F_CPU
#  error "Driver_USART.h requires F_CPU to be defined"
#endif

#define BAUD_TOLERANCE 2

#define USART_Mode_Asynchronus 0<<6
#define USART_Mode_Synchronus 1<<6

#define USART_Parity_Disable 0<<4
#define USART_Parity_Even 2<<4
#define USART_Parity_Odd 3<<4

#define USART_Stop_Bit_1 0<<3
#define USART_Stop_Bit_2 1<<3

#define USART_Character_Size_5 0<<UCSZ0
#define USART_Character_Size_6 1<<UCSZ0
#define USART_Character_Size_7 2<<UCSZ0
#define USART_Character_Size_8 3<<UCSZ0
#define USART_Character_Size_9 1<<(UCSZ2+8)|3<<UCSZ0

//#define USART_Clock_Polarity 1

#define USART_Tx_Enable 1<<11
#define USART_Rx_Enable 1<<12

extern void USART_Initialise(uint16_t Control);
extern void USART_Transmit(void * data, uint8_t len);
extern void USART_Receive(void * data, uint8_t len);

#define USART_Tx_Busy 1
#define USART_Rx_Busy 2
volatile uint8_t USART_Status_Flag;

#endif /* DRIVER_USART_H_ */