#include <stdio.h>
#include <lpc2148/lpc214x.h>
#include <lpc2148/openlpc.h>

#define SENSOR_L        ((AD1DR2 >> 6) & 0x3ff)         // index 2
#define SENSOR_F        ((AD1DR1 >> 6) & 0x3ff)         // index 1
#define SENSOR_R        ((AD1DR0 >> 6) & 0x3ff)         // index 0

void uart0_init(void);
void uart1_init(void);

void uart0_init(void){	
	PINSEL0 |= (1<<0) | (1<<2); //Enable UART0 on P0.0(TX) and P0.1(RX)
	U0FCR = 0x07;
	U0LCR = 0x83;
	U0DLL = 32; 
	U0DLM = 0;
	U0LCR = 0x03; 
}

void uart1_init(void){	
	PINSEL0 |= (1<<16) | (1<<18); //Enable UART1 on P0.8(TX) and P0.9(RX)
	U1FCR = 0x07;
	U1LCR = 0x83;
	U1DLL = 56; 
	U1DLM = 1;
	U1LCR = 0x03; 
}

int lcount = 0;
int rcount = 0;

void left_enc_ISR(){
	lcount++;
	printf("Left tick # %d\n\r", lcount);
	if(lcount == 12){
		U0THR = 0xC1;
		U0THR = 0;
	}
	EXTINT = (1<<2); //Clear interrupt flag on EINT2
}

void right_enc_ISR(){
	rcount++;
	printf("Right tick  # %d\n\r", rcount);
	if(rcount == 12){
		U0THR = 0xC6;
		U0THR = 0;
	}
	EXTINT = (1<<0); //Clear interrupt flag on EINT0
}

int	main (void)
{	
	FIO1DIR |=  1 << 20;	//Use OpenLPC Green LED
	FIO1SET  =	1 << 20;	//Turn off LED during initialization.
	
	uart0_init();	//Initialize UART0 for 3pi motor comm.
	uart1_init();	//Initialize UART1 for Xbee debugging.
	
	FIO1DIR |= (1<<30) | (1<<29) | (1<<28);	//Pins 1.28,1.29, and 1.30 control IR L,R, and F emitters. 
	FIO1CLR = (1<<30) | (1<<29) | (1<<28);	//Turn off all IR LEDs
	
	PINSEL0 |= (3 << 12) | (3 << 20) | (3 << 24) | (3 << 26); //Setup P1.0/2/3/4 as ADC inputs
	
	PINSEL0 |= 1 << 31; //Setup P1.15 as EINT2
	PINSEL1 |= 1 << 0;	//Setup P1.16 as EINT0
	
	EXTMODE = (1 << 0) | (1 << 2); //EINT0 and EINT2 are edge sensitive.
	EXTPOLAR = (1 << 0) | (1 << 2); //Rising edge sensitive. 
	
	printf("Starting up...\n\r");	//Let anyone debugging through USB know we are around.
	FIO1CLR  =	1 << 20;			//Turn on status LED. 
	
	VIC_registerIRQ(right_enc_ISR, EINT0_IRQn, 2);	//EINT0 is the right wheel encoder.
	VIC_registerIRQ(left_enc_ISR, EINT2_IRQn, 1);	//EINT2 is the left wheel encoder.
	
	EXTINT = (1<<0)|(1<<2);	//Clear any interrupt bits before starting.
	
	VIC_enableIRQ(EINT0_IRQn);	//Enable interrupts on EINT0
	VIC_enableIRQ(EINT2_IRQn);	//Enable interrupts on EINT2
	
	//AD1CR = (1 << 0) | (1 << 1) | (1 << 2) | (13 << 8) | (1 << 16) | (1 << 21);
	
	//FIO1SET = (1<<30) | (1<<29) | (1<<28);
	// RIGHT 	MIDDLE 		LEFT
	
	U0THR = 0xC1;
	U0THR = 20;
	U0THR = 0xC6;
	U0THR = 20;
	
	int cmd = 0;
	
	while(1){
		
		cmd = getc(stdin);
		if(cmd == 'r'){
			lcount = 0;
			rcount = 0;
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 20;
		}
		if(cmd == 'l'){
			lcount = 0;
			rcount = 0;
			U0THR = 0xC2;
			U0THR = 20;
			U0THR = 0xC5;
			U0THR = 20;
		}
		
	}


	

}

