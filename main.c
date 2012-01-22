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

void right(int speed);
void left(int speed);
void forward(int speed);
void back(int speed);
void stop(void);

void sright(int speed){
	U0THR = 0xC2;
    U0THR = speed;
    U0THR = 0xC5;
    U0THR = speed;
	delay_ms(590);
	stop();
}
void sleft(int speed){
	U0THR = 0xC1;
    U0THR = speed;
    U0THR = 0xC6;
    U0THR = speed;
	delay_ms(580);
	stop();
}
void forward(int speed){
	U0THR = 0xC1;
	U0THR = speed;
	U0THR = 0xC5;
	U0THR = speed;
}
void back(int speed){
	U0THR = 0xC2;
    U0THR = speed;
    U0THR = 0xC6;
    U0THR = speed;
}

void stop(){
	U0THR = 0xC1;
	U0THR = 0;
	U0THR = 0xC5;
	U0THR = 0;
}


static int tkeep = 0;

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
	
	while(1){
	interval = millis();
	//printf("%d \n\r",interval);
	FIO1SET = (1<<30) | (1<<29) | (1<<28);
	left = SENSOR_L;
	front = SENSOR_F;
	right = SENSOR_R;
	diff = left - right;
	if(diff < 0){
		adiff = -diff;
	} else {
		adiff = diff;
	}
	
	if(front < 300){
		sright(20);
		sright(20);
	}
	
	if(adiff < 350){
		mhold = interval;
		while((interval - mhold) < 1400){
			interval = millis();
			left = SENSOR_L;
			front = SENSOR_F;
			right = SENSOR_R;
			diff = left - right;
			
			if(front < 350){
				sright(19);
				sright(19);
			}
			
			printf("%d %d %d %d %d\n\r",left, right, front,adiff,diff);

			if(diff < 0){
				adiff = -diff;
			} else {
				adiff = diff;
			}
			
			if(adiff < 150){
				delay_ms(1);
				U0THR = 0xC1;
				delay_ms(1);
				U0THR = 20;
				delay_ms(1);
				U0THR = 0xC5;
				delay_ms(1);
				U0THR = 20;
			} else if(diff > 200){
				delay_ms(1);
				U0THR = 0xC1;
				delay_ms(1);
				U0THR = 20;
				delay_ms(1);
				U0THR = 0xC5;
				delay_ms(1);
				U0THR = 30;
			} else if(diff < -200){
				delay_ms(1);
				U0THR = 0xC1;
				delay_ms(1);
				U0THR = 30;
				delay_ms(1);
				U0THR = 0xC5;
				delay_ms(1);
				U0THR = 20;
			} else {
				delay_ms(1);
				U0THR = 0xC1;
				delay_ms(1);
				U0THR = 20;
				delay_ms(1);
				U0THR = 0xC5;
				delay_ms(1);
				U0THR = 20;
			}
			
		}
	}  else {
	
	if(left > 750){
		sright(20);
	}
	
	if(right > 750){
		sleft(20);
	}
	
	delay_ms(1);
	U0THR = 0xC1;
	delay_ms(1);
	U0THR = 20;
	delay_ms(1);
	U0THR = 0xC5;
	delay_ms(1);
	U0THR = 20;
	delay_ms(1);
	printf("%d %d %d %d %d\n\r",left, right, front,adiff,diff);
	}
	
	
	
	delay_ms(1);
	U0THR = 0xC1;
	delay_ms(1);
	U0THR = 0;
	delay_ms(1);
	U0THR = 0xC5;
	delay_ms(1);
	U0THR = 0;
	delay_ms(1);
	delay_ms(1000);
	}
	
	int cont = 1;
	
	

	
	while(1){
		FIO1SET = (1<<30) | (1<<29) | (1<<28);
		left = SENSOR_L;
		front = SENSOR_F;
		right = SENSOR_R;
		diff = left - right;
		//FIO1CLR = (1<<30) | (1<<29) | (1<<28);
		if(diff < 0){
			adiff = -diff;
		} else {
			adiff = diff;
		}
		
		if(1){
		
		tkeep = 0;
		cont = 1;


		printf(" test %d \n\r", tkeep);
		
		while(1){
			stop();
		}
		
		while(cont == 1){

			left = SENSOR_L;
			front = SENSOR_F;
			right = SENSOR_R;
			diff = left - right;

			if(diff < 0){
				adiff = -diff;
			} else {
				adiff = diff;
			}
		
			tkeep = tkeep;
		printf("%d  %d   %d \n\r", front, diff,adiff);	
			
		if(tkeep > 1500){
			cont = 0;
		} else {
		
		
		if(adiff < 150){
			forward(25);
		} else if(diff > 150){
			U0THR = 0xC1;
			U0THR = 15;
			U0THR = 0xC5;
			U0THR = 25;
		} else if(diff < -150){
			U0THR = 0xC1;
			U0THR = 25;
			U0THR = 0xC5;
			U0THR = 15;
		} else {
			forward(25);
		}
		}
		
		}
		

		
		FIO1CLR = (1<<20);
				printf("done");
		delay_ms(1000);
		FIO1SET = (1<<20);
		
	}
}

		


	/*	stop();
		delay_ms(1000);

		
		forward(25);
		delay_ms(1050);

		stop();
		delay_ms(500);
	*/	
	
	
	while(1){
		FIO1SET = (1<<30) | (1<<29) | (1<<28);
		delay_ms(1);
		left = SENSOR_L;
		front = SENSOR_F;
		right = SENSOR_R;
		diff = left - right;
		FIO1CLR = (1<<30) | (1<<29) | (1<<28);
		if(diff < 0){
			adiff = -diff;
		} else {
			adiff = diff;
		}
		
		printf("Left: %d  Front: %d  Right: %d  Diff: %d                           \r",left, front, right, diff );
		
		if(front > 250){
			if(adiff < 150){
				forward(25);
			} else if(diff < -150){
				U0THR = 0xC1;
				U0THR = 25;
				U0THR = 0xC5;
				U0THR = 15;
			} else if(diff > 150){
				U0THR = 0xC1;
				U0THR = 15;
				U0THR = 0xC5;
				U0THR = 30;
			}
		} else {
			if(diff > 100){
				back(20);
				delay_ms(100);
				sright(20);
				delay_ms(400);
				stop();
			} else if(diff < -100){
				back(20);
				delay_ms(100);
				sleft(20);
				delay_ms(400);
				stop();
			} else {
				stop();
			}
		}

	}


	stop();

	o = getc(stdin);
	

	
	int open = 0;
	int centered = 0;
	
	printf("Type c when sensor is open\n\r");
	
	o = getc(stdin);
	
	open = SENSOR_R;
	
	printf("%d\n\r", open);
	
	printf("Type c when sensor is zerod on wall\n\r");
	
	o = getc(stdin);
	
	centered = SENSOR_R;
	
	int distance = 0;
	
	printf("%d \n\r", centered);
	while(1){
		printf("Distance: %d  Right: %d \n\r", distance, SENSOR_R );
		delay_ms(1);
		distance++;
		
		U0THR = 0xC1;
	    U0THR = 20;
	    U0THR = 0xC5;
	    U0THR = 20;
		
		delay_ms(1200);
		
		U0THR = 0xC1;
	    U0THR = 0;
	    U0THR = 0xC5;
	    U0THR = 0;
		printf("Distance: %d  Right: %d \n\r", distance, SENSOR_R );
		if(SENSOR_R < centered - 40){
			U0THR = 0xC2;
		    U0THR = 20;
		    U0THR = 0xC5;
		    U0THR = 20;
		} else if (SENSOR_R > centered + 40 & SENSOR_R < centered + 150) {
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 20;
		} else if(SENSOR_R > centered + 150){
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 20;
			delay_ms(500);
		} else {
			U0THR = 0xC1;
		    U0THR = 0;
		    U0THR = 0xC5;
		    U0THR = 0;
		}
		
		delay_ms(10);
		
		U0THR = 0xC1;
	    U0THR = 0;
	    U0THR = 0xC5;
	    U0THR = 0;
	
		delay_ms(5000);
		
		/*if(SENSOR_R < centered - 40){
			U0THR = 0xC2;
		    U0THR = 20;
		    U0THR = 0xC5;
		    U0THR = 20;
		} else if (SENSOR_R > centered + 40 & SENSOR_R < centered + 100) {
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 20;
		}*/
		
	}
	
	
	for(int i = 0; ; i++) {
		
		//printf("Left: %d, Center: %d, Right: %d \n\r", SENSOR_L,SENSOR_F ,SENSOR_R );
		
		o = getc(stdin);
		if(o == 's'){
			U0THR = 0xC1;
			U0THR = 0;
			U0THR = 0xC5;
			U0THR = 0;
		} else if (o == 'w'){
			U0THR = 0xC1;
			U0THR = 30;
			U0THR = 0xC5;
			U0THR = 30;
		} else if (o == 'q'){
			U0THR = 0xC2;
			U0THR = 30;
			U0THR = 0xC6;
			U0THR = 30;
		} else if (o == 'd'){
			U0THR = 0xC1;
			U0THR = 20;
			U0THR = 0xC6;
			U0THR = 20;
			delay_ms(121);
			U0THR = 0xC1;
			U0THR = 0;
			U0THR = 0xC5;
			U0THR = 0;
		} else if (o == 'a'){
		    U0THR = 0xC2;
		    U0THR = 20;
		    U0THR = 0xC5;
		    U0THR = 20;
			delay_ms(121);
			U0THR = 0xC1;
			U0THR = 0;
			U0THR = 0xC5;
			U0THR = 0;
		} else {
			U0THR = 0x02;
		}

	}
}

