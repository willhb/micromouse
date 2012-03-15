#include <stdio.h>
#include <lpc2148/lpc214x.h>
#include <lpc2148/openlpc.h>

#define SENSOR_B		((AD1DR4 >> 6) & 0x3ff)			// index 4
#define SENSOR_L        ((AD1DR3 >> 6) & 0x3ff)         // index 3
#define SENSOR_F        ((AD1DR2 >> 6) & 0x3ff)         // index 2
#define SENSOR_R        ((AD1DR0 >> 6) & 0x3ff)         // index 0

void uart0_init(void);
void uart1_init(void);

void uart0_init(void){	
	PINSEL0 |= (1<<0) | (1<<2); //Enable UART0 on P0.0(TX) and P0.1(RX)
	U0FCR = 0x07;
	U0LCR = 0x83;
	U0DLL = 32; //115200 Baud
	U0DLM = 0;
	U0LCR = 0x03; 
}

void uart1_init(void){	
	PINSEL0 |= (1<<16) | (1<<18); //Enable UART1 on P0.8(TX) and P0.9(RX)
	U1FCR = 0x07;
	U1LCR = 0x83;
	U1DLL = 0x87; //9600 Baud
	U1DLM = 0x01;
	U1LCR = 0x03; 
}

int lcount = 0;
int rcount = 0;

void left_enc_ISR(){
	VICIntEnClr |= (1 << 16); //Turn off EINT2 interrupt
	EXTPOLAR ^= (1 << 2); //Change polarity of interrupt
	VICIntEnable |= (1 << 16); //Turn interrupts back on.
	FIO1PIN  ^=	1 << 20;
	lcount++;
	printf("Left tick # %d\n\r", lcount);
	if(lcount == 24){
		U0THR = 0xC1;
		U0THR = 0;
	}
	EXTINT = (1<<2); //Clear interrupt flag on EINT2
}

void right_enc_ISR(){
	VICIntEnClr |= (1 << 14); //Turn off EINT0 interrupt
	EXTPOLAR ^= (1 << 0); //Change polarity of interrupt
	VICIntEnable |= (1 << 14); //Turn interrupts back on.
	FIO1PIN  ^=	1 << 20;
	rcount++;
	printf("Right tick  # %d\n\r", rcount);
	if(rcount == 24){
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
	
	printf("Pins Setup...\n\r");	//Let anyone debugging through USB know we are around.
	U1THR = 'O';
	U1THR = 'K';
	FIO1CLR  =	1 << 20;			//Turn on status LED. 
	
	VIC_registerIRQ(right_enc_ISR, EINT0_IRQn, 2);	//EINT0 is the right wheel encoder.
	VIC_registerIRQ(left_enc_ISR, EINT2_IRQn, 1);	//EINT2 is the left wheel encoder.
	
	EXTINT = (1<<0)|(1<<2);	//Clear any interrupt bits before starting.
	
	VIC_enableIRQ(EINT0_IRQn);	//Enable interrupts on EINT0
	VIC_enableIRQ(EINT2_IRQn);	//Enable interrupts on EINT2
	
	printf("Interrupts Enabled...\n\r");	//Let anyone debugging through USB know we are around.
	U1THR = 'O';	//Let anyone debugging over wireless know we are around.
	U1THR = 'K';
	
	AD1CR |= (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4); //AD pins 0,2,3,4 are inputs
	AD1CR |= (15 << 8) | (1 << 16) | (1 << 21); //(60/15) = 4MHz ADC clock, burst mode, ADC power on.
	
	U0THR = 0xC1;
	U0THR = 0;
	U0THR = 0xC6;
	U0THR = 0;
	
	int cmd = 0;
	
	
	while(1){
		
		printf("Left: %ld\n\rRight: %ld\n\rCenter: %ld\n\rBack: %ld\n\r", SENSOR_L, SENSOR_R, SENSOR_F, SENSOR_B);
		
		cmd = getc(stdin);
		if(cmd == 'd'){
			lcount = 0;
			rcount = 0;
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 21;
		}
		if(cmd == 'a'){
			lcount = 0;
			rcount = 0;
			U0THR = 0xC2;
			U0THR = 20;
			U0THR = 0xC5;
			U0THR = 21;
		}
		if(cmd == 'w'){
			lcount = 0;
			rcount = 0;
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC5;
			U0THR = 21;
		}
		if(cmd == 's'){
			lcount = 0;
			rcount = 0;
			U0THR = 0xC2;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 21;
		}
		if(cmd == '1'){
			FIO1SET = (1<<30) | (1<<29) | (1<<28); // RIGHT 	MIDDLE 		LEFT
			
		}
		if(cmd == '0'){
			FIO1CLR = (1<<30) | (1<<29) | (1<<28); // RIGHT 	MIDDLE 		LEFT
			
		}
		
	}


	

}

