#include "Driver_I2C.h"
#include "DS1307.h"

#define DS1307_ADD 0x68

static uint8_t char_bcd(uint8_t dat){
	return ((dat/10)<<4 | dat%10 );
}
static uint8_t bcd_char(uint8_t dat){
	return (dat & 0xF)+((dat>>4) *10);
}

void DS1307_Set_Time(void* time){
	uint8_t i;
	uint8_t* dat=(uint8_t*) time;
	dat[0]=0;
	for(i=1;i<4;i++)
		dat[i]=char_bcd(dat[i]);

	while (I2C_flag & I2C_Busy);
	I2C_Set_Speed(100000);
	I2C_Master_Transmit(DS1307_ADD,dat,4);
}

void DS1307_Read_Time(void *time){
	uint8_t* dat=(uint8_t*) time;
	uint8_t i=0;
	
	while (I2C_flag & I2C_Busy);
	I2C_Set_Speed(100000);
	I2C_Master_Transmit(DS1307_ADD,&i,1);

	while (I2C_flag & I2C_Busy);
	I2C_Master_Receive(DS1307_ADD,dat+1,3);
	while (I2C_flag & I2C_Busy);
	dat[0]=0;
	for(i=0;i<4;i++)
		dat[i]=bcd_char(dat[i]);
}