//----------------------------------------------------------------------------

// C main line

//----------------------------------------------------------------------------



#include <m8c.h>        // part specific constants and macros

#include "PSoCAPI.h"    // PSoC API definitions for all User Modules

#include <stdbool.h>



void print_lcd(int, int, char[]);

void threshold_state();



//Initialize the variables

int ZERO = 0;

int SEVEN_BITS = 7;

int MAX_STATE = 5;

int ACC_SUB_STATE = 3;

int THRESHOLD_SAMPLES = 10;

int minimum_avg = 0;

int maximum_avg = 0;

bool result = true;

int TIMER_MAX = 10;

int time_recordings[TIMER_MAX];

int recordings[THRESHOLD_SAMPLES];

unsigned int toggle_state = 0; //mode_flag

unsigned int button_count; //check

unsigned int short_press = 0; //accuracyState

unsigned int acc_state = 0; //ch



void accuracy_state(){

	int start = 1;

	int end = 0;

	clear_LCD();

	LCD_Position(0,0);

	LCD_PrCString("START ACCMODE");

	while (result)

	{

		if(short_press == 1)

		{

			print_lcd(start, end, "ONE SEC");

		}

		else if(short_press == 2)

		{

			print_lcd(start, end, "HALF SEC");

		}

		else if(short_press == 3)

		{

			print_lcd(start, end, "ONE_TENTH SEC");

		}

		if(acc_state == 1)

		{

			short_press = short_press + 1;

			if(short_press > ACC_SUB_STATE)

			{

				short_press = 1;   

			}

			acc_state = 0;

		}

		else 

		{

			acc_state = ZERO;

			toggle_state = ACC_SUB_STATE - 1; //Goes to the main (Check for the next long press) 

			break;

		}

	}

}



//Function to collect the number of samples

void threshold_state()

{

	print_lcd(0,0, "THRESHOLD_STATE");

	int mode_count = 0;

	while(1)

	{

		if(PRT0DR&0x01)

		{

			mode_count = mode_count + 1;

		}

		if (mode_count == 1){

			print_lcd(0,0, "THRESHOLD_ONE");

		}

		if (mode_count == 2){

			print_lcd(0,0, "CALCULATE");

			for(int itr=0;itr<THRESHOLD_SAMPLES;itr++)

			{	

				if(DUALADC_fIsDataAvailable()!=0){

					recordings[itr] = DUALADC_iGetDatal();

				} 

			}	

			quickSort(recordings,0, THRESHOLD_SAMPLES);

			for(int j=0;j<5;j++){

				minimum_avg += recordings[j]; 

			}

			minimum_avg = minimum_avg / 5;

			for(int k=5;k<10;k++){

				maximum_avg += recordings[k]; 

			}

			maximum_avg = maximum_avg / 5;

		}

		if (mode_count == 3)

		{

			print_lcd(0,0,"THRESHOLD_DONE");

		}

		if (mode_count>3)

		{

			break;

		}

	}

}



int partition (int arr[], int mi, int mx) 

{ 

    int p = arr[mx];   

    int i = (mi - 1);   

  

    for (int j = mi; j <= mx- 1; j++) 

    {  

        if (arr[j] < p) 

        { 

            i++;  

			int t = arr[i]; 

    		arr[i] = arr[j]; 

    		arr[j] = t; 

        } 

    } 

   	int t = arr[i+1]; 

   	arr[i+1] = arr[high]; 

    arr[high] = t; 

    return (i + 1); 

} 



void quickSort(int arr[], int mi, int mx) 

{ 

    if (mi < mx) 

    { 

        quickSort(arr, mi, partition(arr, mi, mx) - 1); 

        quickSort(arr, partition(arr, mi, mx) + 1, mx); 

    } 

} 



void buttonpress_state(){

	print_lcd(0,0, "BUTTONPRESS");

	while(1){

		if (short_press > 1)

		{

			run_timer();

		}

		if (PRT0DR&0x01 && delay(5) && short_press > 2)

		{

			short_press = short_press + 1;	

			run_timer();

		}

	}

}



void microphone_state(){

	print_lcd(0,0, "MICROPHONE");

	while(1){

		if (DUALADC_fIsDataAvailable()!=1)

		{

			run_timer();

		}

		if (PRT0DR&0x01 && delay(5) && DUALADC_fIsDataAvailable()!=1)

		{

			short_press = short_press + 1;	

			run_timer();

		}

	}

}



void memory_mode()

{

	

	int maximum;

	int minimum;

	int average;

	int sum = 0;

	print_lcd(0,0, "MEMORY_MODE");

	while (1)

	{

		if (short_press > 1)

		{

			if(PRT0DR&0x01 && delay(1))

			{

				short_press = short_press + 1;

				Timer16_1_Start();

				for (int j=0;j<TIMER_MAX;j++)

				{

					time_recordings[j] = Timer16_1_wReadTimer();

				}

			}

		}

	}

	//Find the maximum

	quicksort(time_recordings, 0, TIMER_MAX);

	qsort(time_recordings, TIMER_MAX, sizeof(int), comparefunction);

	minimum = time_recordings[0];

	maximum = time_recordings[9];

	for (int q = 0; q < TIMER_MAX; q++)

	{

		sum += time_recordings[q];

	}

	average = sum  / TIMER_MAX;

}



void print_lcd(int x, int y, char display[]){

	LCD_Position(x,y);

	LCD_PrCString(display);

}



void run_timer()

{

	if(PRT0DR&0x01 && delay(1))

	{

			short_press = short_press + 1;

			Timer16_1_Start();

			for (int i=0;i<100;i++)

			{

				for (int j=0;j<60;j++)

				{

					for (int k=0;k<60;k++)

					{

							LCD_Position(0,0);

							LCD_PrHexByte(i);

							LCD_Position(0,2);

							LCD_PrHexByte(j);

							LCD_Position(0,4);

							LCD_PrHexByte(k);

					}

				}

			}

	}

}



void main(void)

{

	// Insert your main routine code here.

	LCD_1_Start();

	LCD_Position(0,0);

	LCD_PrCString("Start PSoC");

	

	// Enable interrupts

	M8C_EnableGInt;  

	M8C_EnableIntMask INT_MSK1; 

	M8C_EnableIntMask(INT_MSK1, INT_MSK1_DBB01); 	

	M8C_EnableIntMask INT_MSK0; 

	M8C_EnableIntMask(INT_MSK0, INT_MSK0_GPIO); 

	

	//Initialization part - ADC 

	PGA_1_Start(PGA1_ON);

	PGA_2_Start(PGA2_ON);

	

	//Filter Initialize

	LPF2_3_Start(FILTER_ON);

                   

	//Analog to Digital conversion part

	DUALADC_Start(DUALADC_ON); // Turn on Analog section

    DUALADC_SetResolution(SEVEN_BITS);  // Set resolution to 7 Bits

    DUALADC_GetSamples(ZERO);

	Timer16_1_Start();	

	

	//Validate the button for long press

	while(1)

	{

		button_count = 0;

		if(PRT0DR&0x01)

		{

			button_count = button_count + 1;

		}

	}

	

	//Validate the finite states

	if(button_count == ZERO){

		toggle_state = 1;

	}

	else{

		toggle_state = toggle_state + 1;

		if(toggle_state > MAX_STATE){

			toggle_state = 1;   

		}

	}

	

	//Accuracy state - ToggleState 1

	if (toggle_state == 1){

		accuracy_state();

	}

	

	//Threshold state 

	if (toggle_state == 2){

		threshold_state();

	}

	

	//Memory state 

	if (toggle_state == 3){

		memory_state();

	}

	

	//Threshold state 

	if (toggle_state == 4){

		microphone_state();

	}

	

	//Threshold state 

	if (toggle_state == 5){

		buttonpress_state();

	}

}