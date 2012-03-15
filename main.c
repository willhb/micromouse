#include <stdio.h>
#include <lpc2148/lpc214x.h>
#include <lpc2148/openlpc.h>

#define SENSOR_L        ((AD1DR2 >> 6) & 0x3ff)         // index 2
#define SENSOR_F        ((AD1DR1 >> 6) & 0x3ff)         // index 1
#define SENSOR_R        ((AD1DR0 >> 6) & 0x3ff)         // index 0

void uart0_init(void);

void uart0_init(void){
	
	PINSEL0=0x5; //Enable UART0 on P0.0(TX) and P0.1(RX)
	U0FCR = 0x07;
	U0LCR = 0x83;
	U0DLL = 32; 
	U0DLM = 0;
	U0LCR = 0x03; 
}


void sright(int speed){
	U0THR = 0xC2;
    U0THR = speed;
    U0THR = 0xC5;
    U0THR = speed;
	delay_ms(590);
	stop();
}


int	main (void)
{	
	// initialize the LED pin as an output
	uart0_init();
	FIO1DIR |= 1 << 26 | 1 << 20;
	FIO1DIR |= (1<<30) | (1<<29) | (1<<28);
	FIO1CLR = (1<<30) | (1<<29) | (1<<28);
	FIO1SET = (1<<30) | (1<<29) | (1<<28);
	// RIGHT 	MIDDLE 		LEFT
	PINSEL0 |= (3 << 12) | (3 << 16) | (3 << 20);
	AD1CR = (1 << 0) | (1 << 1) | (1 << 2) | (13 << 8) | (1 << 16) | (1 << 21);
	
	int o = 0;	
	int mhold = 0;
	FIO1SET = (1<<30) | (1<<29) | (1<<28);
	
	int front,left,right;
	
	int adiff,diff;
	
	int interval = millis();
	

}

