#ifndef DS1307_H_
#define DS1307_H_

typedef struct _time{
	uint8_t _RESERVED;
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
}TIME;

extern void DS1307_Set_Time(void* time);
extern void DS1307_Read_Time(void* time);

#endif