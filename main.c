#include <stdio.h>
#include <lpc2148/lpc214x.h>
#include <lpc2148/openlpc.h>

#define R_OFFSET  100

#define SENSOR_B		((AD1DR4 >> 6) & 0x3ff)					// index 4
#define SENSOR_L        ((AD1DR3 >> 6) & 0x3ff)         		// index 3
#define SENSOR_F        ((AD1DR2 >> 6) & 0x3ff)         		// index 2
#define SENSOR_R        ((AD1DR0 >> 6) & 0x3ff) + R_OFFSET     // index 0

#define NORTH 1
#define EAST 2
#define SOUTH 3
#define WEST 4

int direction = NORTH;

int x = 1;
int y = 1;

static int maze[18][18] = { 
	{17,19,20,21,22,23,24,25,26,25,24,23,22,21,20,19,18,17},
	{18,16,15,14,13,12,11,10,9 ,8 ,10,11,12,13,14,15,16,19},
	{19,15,14,13,12,11,10,9 ,8 ,7 ,9 ,10,11,12,13,14,15,20},
	{20,14,13,12,11,10,9 ,8 ,7 ,6 ,8 ,9 ,10,11,12,13,14,21},
	{21,13,12,11,10,9 ,8 ,7 ,6 ,5 ,7 ,8 ,9 ,10,11,12,13,22},
	{22,12,11,10,9 ,8 ,7 ,6 ,5 ,4 ,6 ,7 ,8 ,9 ,10,11,12,23},
	{23,11,10,9 ,8 ,7 ,6 ,5 ,4 ,3 ,5 ,6 ,7 ,8 ,9 ,10,11,24},
	{24,10,9 ,8 ,7 ,6 ,5 ,4 ,3 ,2 ,4 ,5 ,6 ,7 ,8 ,9 ,10,25},
	{25,9 ,8 ,7 ,6 ,5 ,4 ,3 ,1 ,1 ,3 ,4 ,5 ,6 ,7 ,8 ,9 ,26},
	{26,8 ,7 ,6 ,5 ,4 ,3 ,2 ,1 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,25},
	{26,10,9 ,8 ,7 ,6 ,5 ,4 ,3 ,2 ,4 ,5 ,6 ,7 ,8 ,9 ,10,24},
	{25,11,10,9 ,8 ,7 ,6 ,5 ,4 ,3 ,5 ,6 ,7 ,8 ,9 ,10,11,23},
	{24,12,11,10,9 ,8 ,7 ,6 ,5 ,4 ,6 ,7 ,8 ,9 ,10,11,12,22},
	{23,13,12,11,10,9 ,8 ,7 ,6 ,5 ,7 ,8 ,9 ,10,11,12,13,21},
	{22,14,13,12,11,10,9 ,8 ,7 ,6 ,8 ,9 ,10,11,12,13,14,20},
	{21,15,14,13,12,11,10,9 ,8 ,7 ,9 ,10,11,12,13,14,15,19},
	{20,16,15,14,13,12,11,10,9 ,8 ,10,11,12,13,14,15,16,18},
	{19,18,19,20,21,22,23,24,25,25,24,23,22,21,20,19,18,17},
	

};

static visited[16][16] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

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

volatile int lcount = 0;
volatile int rcount = 0;
volatile int move = 0;
volatile int count = 0;	
volatile int finished = 0;
		int floodfill = 0;


void left_enc_ISR(){
	VICIntEnClr |= (1 << 16); //Turn off EINT2 interrupt
	EXTPOLAR ^= (1 << 2); //Change polarity of interrupt
	VICIntEnable |= (1 << 16); //Turn interrupts back on.
	FIO1PIN  ^=	1 << 20;
//	printf("L: %d\n\r", lcount);
	if(lcount <= 0){
		U0THR = 0xC1;
		U0THR = 0;
		finished--;
	} else {
		lcount--;	
	}
	
	EXTINT = (1<<2); //Clear interrupt flag on EINT2
}


void right_enc_ISR(){
	VICIntEnClr |= (1 << 14); //Turn off EINT0 interrupt
	EXTPOLAR ^= (1 << 0); //Change polarity of interrupt
	VICIntEnable |= (1 << 14); //Turn interrupts back on.
	FIO1PIN  ^=	1 << 20;
//	printf("R: %d\n\r", rcount);
	if(rcount <= 0){
		U0THR = 0xC5;
		U0THR = 0;
		finished--;
	} else {
		rcount--;
	}
	
	EXTINT = (1<<0); //Clear interrupt flag on EINT0
}



int difference = 0;
int pid = 0;
int prev_diff = 0;
int abs_diff = 0;
int integral = 0;


float kp = 5;
float kd = 1.5; //.7
float ki = 0.01;



void wheel_PID(){
	
	int l_sensor = SENSOR_L;
	int r_sensor = SENSOR_R;
	int f_sensor = SENSOR_F;
	
	int difference = lcount - rcount;
	

	int sensor_diff = l_sensor-r_sensor;
	
	if(sensor_diff < 0){
		sensor_diff = -sensor_diff;
	}

//	printf("difference: %d\n\r", sensor_diff);


		if(l_sensor < 500){
			difference += 2000/l_sensor;
		}
		if(r_sensor < 500){
			difference -= 2000/r_sensor;
		}

	if(difference < 3){
		integral = 0;
	} else {
		integral += difference;
	}
	
	pid = difference*kp + (difference - prev_diff)*kd + integral*ki;
	
	prev_diff = difference;
	
	U0THR = 0xC1;
	U0THR = 20 + pid;
	U0THR = 0xC5;
	U0THR = 20 - pid;
}

void forward(){
	lcount = 64;
	rcount = 64;
	difference = 0;
	pid = 0;
	prev_diff = 0;
	abs_diff = 0;
	integral = 0;
	int interval = set_interval(wheel_PID, 5);
	while((lcount > 0) && (rcount > 0) && SENSOR_F>550){
		
	}
	clear_interval(interval);
	
	if(SENSOR_F>550 || (lcount < 30) || (rcount < 30)){
	
	if(direction == NORTH){
		y++;
	}
	else if(direction == SOUTH){
		y--;
	}
	else if(direction == EAST){
		x++;
	}
	else if(direction == WEST){
		x--;
	}
	
	}
	U0THR = 0xC2;
	U0THR = 0;
	U0THR = 0xC5;
	U0THR = 0;
	
}

void calibrate(){
	
	lcount = 7;
	rcount = 7;
	
	U0THR = 0xC2;
	U0THR = 20;
	U0THR = 0xC5;
	U0THR = 20;
	
	while((lcount > 0) && (rcount > 0))
	{

	}
	
	int front_start = SENSOR_F;
	int loop = 1;
	int front_change = 1000;

	VICIntEnClr |= (1 << 14)|(1<<16);
	
	delay_ms(1);
	
	U0THR = 0xC1;
	U0THR = 20;
	U0THR = 0xC6;
	U0THR = 20;
	
	while(loop){
		
//		printf("FC: %d, loop: %d \n\r", front_change, loop);
		
		delay_ms(10);
		
		if(SENSOR_F < front_change){
			front_change = SENSOR_F;
		}
		
		if(SENSOR_F > front_change + 10){
			loop = 0;
		}
		
		
	}
	
	VICIntEnable |= (1 << 14)|(1 << 16);
	
	lcount = 3;
	rcount = 3;
	
	U0THR = 0xC2;
	U0THR = 20;
	U0THR = 0xC5;
	U0THR = 20;
	
	while((lcount > 0) && (rcount > 0))
	{
	}

	delay_ms(500);
		/*
	
	VICIntEnClr |= (1 << 14)|(1<<16);
	
	
	while(SENSOR_F < 400){
		U0THR = 0xC2;
		U0THR = 20;
		U0THR = 0xC6;
		U0THR = 20;	
	}
	
	
	
	VICIntEnable |= (1 << 14)|(1 << 16);*/

	
}

void rotate_180(){
	
	lcount = 46;
	rcount = 46;
//	printf("hello");
	
	if(direction == NORTH) direction = SOUTH;
	else if(direction == SOUTH) direction = NORTH;
	else if(direction == EAST) direction = WEST;
	else if(direction == WEST) direction = EAST;
	
	while((lcount > 0) && (rcount > 0))
	{
//	printf("rotating");
	U0THR = 0xC2;
	U0THR = 20;
	U0THR = 0xC5;
	U0THR = 20;
	}
	
	U0THR = 0xC2;
	U0THR = 0;
	U0THR = 0xC5;
	U0THR = 0;
}

void rotate_left(){
	
	lcount = 23;
	rcount = 22;
	
	direction--;
	if(direction <= 0){
		direction = 4;
	}
	
	while((lcount > 0) && (rcount > 0))
	{
	//	printf("rotating");
	U0THR = 0xC2;
	U0THR = 20;
	U0THR = 0xC5;
	U0THR = 20;
	}
	
	U0THR = 0xC2;
	U0THR = 0;
	U0THR = 0xC5;
	U0THR = 0;
	
}

void rotate_right(){
	
	lcount = 23;
	rcount = 22;
	
	direction++;
	if(direction >= 4){
		direction = 1;
	}
	
	printf("hello");
	
	while((lcount > 0) && (rcount > 0))
	{
	//printf("rotating");
	U0THR = 0xC1;
	U0THR = 20;
	U0THR = 0xC6;
	U0THR = 20;
	}
	
	U0THR = 0xC2;
	U0THR = 0;
	U0THR = 0xC5;
	U0THR = 0;
	
}

int get_options(int f, int r, int l){
	
	int feedback = 0;
	
	visited[x][y] = 1;
	
	int smallest = 1000;
	
	printf("Maze: Old %d", maze[x][y]);
	
	if(direction == NORTH){
		if(f){
		feedback |= maze[x][y+1];
		if(maze[x][y+1] <= smallest){
			smallest = maze[x][y+1];
		}
		//feedback += (visited[x][y+1]);
		}
		if(r){
		feedback |= maze[x+1][y] << 8;
		if(maze[x+1][y] <= smallest){
			smallest = maze[x+1][y];
		}
		//feedback += (visited[x+1][y] << 8);	
		}
		if(l){
		feedback |= maze[x-1][y] << 16;
		if(maze[x-1][y] <= smallest){
			smallest = maze[x-1][y];
		}
		//feedback += (visited[x-1][y] << 16);	
		}
		if(maze[x][y-1] <= smallest){
			smallest = maze[x][y-1];
		}
		if(maze[x][y] < smallest + 1){
			maze[x][y] = smallest + 1;
		} else {
			maze[x][y]++;
		}
	}
	else if(direction == SOUTH){
		if(f){
		feedback |= maze[x][y-1];
		if(maze[x][y-1] <= smallest){
			smallest = maze[x][y-1];
		}
		//feedback += (visited[x][y-1] );
		}
		if(r){
		feedback |= maze[x-1][y] << 8;
		if(maze[x-1][y] <= smallest){
			smallest = maze[x-1][y];
		}
		//feedback += (visited[x-1][y] << 8);	
		}
		if(l){
		feedback |= maze[x+1][y] << 16;
		if(maze[x+1][y] <= smallest){
			smallest = maze[x+1][y];
		}
		//feedback += (visited[x+1][y] << 16);	
		}
		if(maze[x+1][y] <= smallest){
			smallest = maze[x+1][y];
		}
		if(maze[x][y] < smallest + 1){
			maze[x][y] = smallest + 1;
		}	 else {
				maze[x][y]++;
			}
	}
	else if(direction == WEST){
		if(f){
		feedback |= maze[x-1][y];
		if(maze[x-1][y] <= smallest){
			smallest = maze[x-1][y];
		}
		//feedback += (visited[x-1][y]);
		}
		if(r){
		feedback |= maze[x][y+1] << 8;
		if(maze[x][y+1] <= smallest){
			smallest = maze[x][y+1];
		}
		//feedback += (visited[x][y+1] << 8);	
		}
		if(l){
		feedback |= maze[x][y-1] << 16;
		if(maze[x][y-1] <= smallest){
			smallest = maze[x][y-1];
		}
		//feedback += (visited[x][y-1] << 16);	
		}
		if(maze[x+1][y] <= smallest){
			smallest = maze[x+1][y];
		}
		if(maze[x][y] < smallest + 1){
			maze[x][y] = smallest + 1;
		}	 else {
				maze[x][y]++;
			}
	}
	else if(direction == EAST){
		if(f){
		feedback |= maze[x+1][y];
		if(maze[x+1][y] <= smallest){
			smallest = maze[x+1][y];
		}
		//feedback += (visited[x+1][y]);
		}
		if(r){
			
		feedback |= maze[x][y-1] << 8;
		if(maze[x][y-1] <= smallest){
			smallest = maze[x][y-1];
		}
		//feedback += (visited[x][y-1] << 8);	
		}
		if(l){
		feedback |= maze[x][y+1] << 16;
		if(maze[x][y+1] <= smallest){
			smallest = maze[x][y+1];
		}
		//feedback += (visited[x][y+1] << 16);	
		}
		if(maze[x-1][y] <= smallest){
			smallest = maze[x-1][y];
		}
		if(maze[x][y] < smallest + 1){
			maze[x][y] = smallest + 1;
		}	 else {
				maze[x][y]++;
			}
	}
	
	printf(" new %d \n\r", maze[x][y]);
	
	return feedback;
}

void led_off()
{
    FIO1SET = 1 << 20;
}

void pulse_led()
{
    FIO1CLR = 1 << 20;				// turn on LED1.26
    set_timeout(led_off, 100);		// LED1.26 will be turned off in 100ms
}



void button_released()
{
    static int count = 0;
    count++;
    if(EXTINT & (1 << 1)) {
        pulse_led();
        printf("%d The button was released!\n\r", count);
		if(floodfill == 0){
			floodfill = 1;
		} else if(floodfill == 1){
			floodfill = 2;
		} else {
			floodfill = 0;
		}
    	EXTINT = 0x2;	// clear interrupt flag
    }
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
	
	EXTMODE |= (1 << 1);             // set EXT1 as edge sensitive
	EXTPOLAR |= (1 << 1);           // listen for rising edge (button released)
	PINSEL0 |= (PINSEL0 & ~(0x3 << 28)) | (0x2 << 28);
	
	EXTINT = 0xf;					// clear external interrupt flags
	VIC_registerIRQ(button_released, EINT1_IRQn, 13);
	VIC_enableIRQ(EINT1_IRQn);
	
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
	U0THR = 10;
	U0THR = 0xC5;
	U0THR = 10;
	
	int cmd = 0;

	FIO1SET = (1<<30) | (1<<29) | (1<<28); //Turn on IR LEDs
	
	int i = 0;
	
	while(i < 10){
		int l = SENSOR_L;
		int r = SENSOR_R;
		int f = SENSOR_F;
		i++;
	}
	
	
	while(1){
			printf("SENSOR_F: %d, SENSOR_R: %d, SENSOR_L %d \n\r", SENSOR_F, SENSOR_R,SENSOR_L);
	}
	rcount = 0;
	lcount = 0;
	
	int r_a = 0;
	int l_a = 0;
	int f_a = 0;
	
	int dest = 0;
	
	
	delay_ms(2500);
	
	forward();
	
	delay_ms(500);
	
	int options = 14;

	while(1){
		
	
		
	//	printf("%d %d %d \n\r %d %d %d \n\r %d %d %d \n\r", maze[x-1][y+1],maze[x][y+1],maze[x+1][y+1],maze[x-1][y],maze[x][y],maze[x+1][y+1],maze[x-1][y-1],maze[x][y-1],maze[x+1][y-1]);
		
		
		r_a = 0;
		l_a = 0;
		f_a = 0;
		
		if(SENSOR_F<500){
			//calibrate();
		}
		
		if(SENSOR_R > 720){
			r_a = 1;
		}
		
		if(SENSOR_L > 720){
			l_a = 1;
		}
		
		if(SENSOR_F > 650){
			f_a = 1;
		}
		
		options = get_options(f_a,r_a,l_a);
		
		
		int front_option = (options)&0xFF;
		int right_option = (options>>8)&0xFF;
		int left_option = (options>>16)&0xFF;
		

		
		delay_ms(100);
		
		//printf("Front: %d, Right: %d, Left %d , Maze %d, Direction %d, x: %d, y: %d\n\r", front_option, right_option, left_option, maze[x][y], direction, x,y);
		
		dest = r_a + (l_a << 1) + (f_a << 2);
		
		if(floodfill == 0){
		
		switch(dest){
			case 1:
				calibrate();
				rotate_right();
				break;
			case 2:
				calibrate();
				rotate_left();
				break;
			case 3:
				calibrate();
				if(right_option < left_option){
					rotate_right();
				} else if (left_option < right_option) {
					rotate_left();
				} else {
					if(millis()%2){
					rotate_left();
					} else {
						rotate_right();
					}
				}
				break;
			case 0:
				calibrate();
				maze[x][y]++;
				rotate_180();
				break;
			case 4:
				forward();
				break;
			case 5:
				if(front_option < right_option){
					forward();
				} else {
					rotate_right();
				}
				break;
			case 6:
			    if(front_option < left_option){
			    	forward();
			    } else {
			    	rotate_left();
			    }
				break;
			case 7:
				if(front_option <= right_option){
					if(front_option < left_option){
						forward();
					}
				} else 	if(left_option <= right_option){
							rotate_left();
						} else {
							rotate_right();
						}
				break;
			default:
			break;
		}
		
		} else if(floodfill == 1) {
			
			switch(dest){
				case 1:
					calibrate();
					rotate_right();
					break;
				case 2:
					calibrate();
					rotate_left();
					break;
				case 3:
					calibrate();
					if(right_option+ visited[x][y] < left_option+ visited[x][y]){
						rotate_right();
					} else if (left_option + visited[x][y] < right_option +visited[x][y]) {
						rotate_left();
					} else {
						if((millis()>>10)%2){
						rotate_right();	
						} else {
						rotate_left();
						}
					}
					break;
				case 0:
					calibrate();
					visited[x][y] += 1;
					rotate_180();
					break;
				case 4:
				if(front_option < maze[x][y]){
					forward();
				} else {
					rotate_180();
				}
					break;
				case 5:
					if(front_option + visited[x][y] < right_option + visited[x][y]){
						forward();
					} else {
						rotate_right();
					}
					break;
				case 6:
				    if(front_option + visited[x][y] < left_option + visited[x][y]){
				    	forward();
				    } else {
				    	rotate_left();
				    }
					break;
				case 7:
					if(front_option + visited[x][y] <= right_option + visited[x][y]){
						if(front_option + visited[x][y] <= left_option + visited[x][y]){
							forward();
						}
					} else 	if(left_option +visited[x][y] <= right_option + visited[x][y]){
								rotate_left();
							} else {
								rotate_right();
							}
					break;
				default:
				break;
			}
			
		} else if(floodfill == 2){
			
			switch(dest){
				case 1:
					calibrate();
					rotate_right();
					break;
				case 2:
					calibrate();
					rotate_left();
					break;
				case 3:
					calibrate();
					if(right_option < left_option){
						rotate_right();
					} else if (left_option  < right_option) {
						rotate_left();
					} else {
						if((millis()>>10)%2){
						rotate_right();	
						} else {
						rotate_left();
						}
					}
					break;
				case 0:
					calibrate();
					visited[x][y] += 1;
					rotate_180();
					break;
				case 4:
				if(front_option < maze[x][y] +1){
					forward();
				} else {
					rotate_180();
				}
					break;
				case 5:
					if(front_option  <= right_option+1){
						forward();
					} else {
						rotate_right();
					}
					break;
				case 6:
				    if(front_option <= left_option +1){
				    	forward();
				    } else {
				    	rotate_left();
				    }
					break;
				case 7:
					if(front_option <= right_option ){
						if(front_option <= left_option ){
							forward();
						}
					} else 	if(left_option < right_option ){
								rotate_left();
							} else {
								rotate_right();
							}
					break;
				default:
				break;
			}
			
			
		}
		
		
		
		//printf("Available: F: %d, R: %d, L: %d \n\r", f_a, r_a, l_a);
	//	printf("Direction: %d \n\r", direction);

		
	}
		
	
	
	
	while(1){
	printf("%ld %ld %ld \n\r", SENSOR_F,SENSOR_L,SENSOR_R);
	}
	
	while(1){
	/*		rotate_right();
			delay_ms(2000);
			rotate_left();
			delay_ms(2000);
			rotate_180();
			delay_ms(2000);*/
			if(SENSOR_F>500){
			forward();
			} else {
			if(SENSOR_R > 750){
				rotate_right();
			} else if(SENSOR_L > 750){
				rotate_left();
			} else {
			rotate_180();
			}	
			}
			delay_ms(1000);
	}
	
	int interval = set_interval(wheel_PID, 5);
	
	lcount = 60;
	rcount = 60;
	

		
	while(1){
		
		if(SENSOR_F < 580){
			clear_interval(interval);
			printf("cleared");
			if((SENSOR_R > 650) & (SENSOR_L > 650)){
				if(millis()%2){
					rotate_right();
				} else {
					rotate_left();
				}
			} else if(SENSOR_R > 650) {
				rotate_right();
			} else if(SENSOR_L > 650) {
				rotate_left();
			}	else {
			rotate_180();
			}
			difference = 0;
			pid = 0;
			prev_diff = 0;
			abs_diff = 0;
			integral = 0;
			interval = set_interval(wheel_PID, 1);
			printf("set");
		}
	
	}
}

