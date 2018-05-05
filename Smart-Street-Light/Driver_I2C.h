#ifndef _Driver_I2C_H
#define _Driver_I2C_H

#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#  error "Driver_I2C.h requires F_CPU to be defined"
#endif

#define I2C_Master_Write	0<<1
#define I2C_Master_Read		1<<1
#define I2C_Slave_Write		2<<1
#define I2C_Slave_Read		3<<1
#define I2C_Mode_Mask		3<<1

#define I2C_Busy	1

#define I2C_General_Call_En	1

#define I2C_Master_Transmit_Arb_Lost	1<<3
#define I2C_Master_Receive_Arb_Lost		2<<3
#define I2C_Master_Write_Slave_NACK		3<<3
#define I2C_Master_Read_Slave_NACK		4<<3
#define I2C_Master_Arb_Lost_SLA_W_Rec	5<<3
#define I2C_Master_Arb_Lost_Gen_Rec		6<<3
#define I2C_Master_Arb_Lost_SLA_R_Rec	7<<3
#define I2C_Error_Mask					7<<3

extern void I2C_Master_Transmit(uint8_t SLA_ADD, uint8_t *dat, uint8_t num);
extern void I2C_Master_Receive(uint8_t SLA_ADD, uint8_t *dat, uint8_t num);
extern void I2C_Slave_Transmit(uint8_t slave_ad, uint8_t *dat, uint8_t num);
extern void I2C_Slave_Receive(uint8_t slave_ad, uint8_t gen_call_en, uint8_t *dat, uint8_t num);
extern void I2C_Set_Speed(uint32_t Speed);

extern volatile uint8_t I2C_flag;

#endif