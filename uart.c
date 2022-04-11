#include "MKL25Z4.h"                    // Device header
#include <math.h>

#define SOUND_OUT (1) // Sound Output in PORTC
#define SW1 (12) // Buttons to choose music
#define SW2 (4) // Buttons to choose music
#define SW3 (2) // Buttons to choose music
#define SW4 (1) // Buttons to choose music


#define MAX_DAC_CODE (4095)
#define NUM_STEPS (512)
#define MASK(x) (1UL << (x))

//OCTAVE 3
# define  C_o3     131
# define  Csh_o3   139 
# define  D_o3     147  
# define  Dsh_o3   156
# define  E_o3     165
# define  F_o3     175       
# define  Fsh_o3   185
# define  G_o3     196       
# define  Gsh_o3   208    
# define  A_o3     220
# define  Ash_o3   233
# define  B_o3     247
//OCTAVE 4
# define  C_o4     262
# define  Csh_o4   277   //Db or do diesis//sharp or re bemol/flat 
# define  D_o4     294 //2554   //D or re  
# define  Dsh_o4   311
# define  E_o4     330
# define  F_o4     349//2148       
# define  Fsh_o4   370
# define  G_o4     392//1913       
# define  Gsh_o4   415//1806    
# define  A_o4     440//1705   // A flat
# define  Ash_o4   466
# define  B_o4     494
//OCTAVE 5
# define  C_o5     523
# define  Csh_o5   554   //Db or do diesis//sharp or re bemol/flat 
# define  D_o5     587 //1277   //D or re  
# define  Dsh_o5   622
# define  E_o5     659
# define  F_o5     698       
# define  Fsh_o5   740
# define  G_o5     784       
# define  Gsh_o5   831    
# define  A_o5     881   // A flat
# define  Ash_o5   933
# define  B_o5     988 
//OCTAVE 6
# define  C_o6     1047
# define  Csh_o6   1109
# define  D_o6     1175
# define  Dsh_o6   1245
# define  E_o6     1319
# define  F_o6     1397
# define  Fsh_o6   1480
# define  G_o6     1568
# define  Gsh_o6   1661
# define  A_o6     1760
# define  Ash_o6   1865
# define  B_o6     1976 
# define  E_o7     2637
# define  C_o7     2093

//Global Music Variables
static unsigned long period, dutyCycle;
static unsigned int note;
static uint16_t SineTable[NUM_STEPS];
static uint16_t * Reload_DMA_Source=0;
static uint32_t Reload_DMA_Byte_Count=0;

// "Montagues and Capulets" a.k.a "Dance of the Knights" by Sergei Prokofiev
static unsigned int Knights[] = { //80 notes total
	B_o3, E_o4,G_o4,B_o4,E_o5,B_o4,G_o4,E_o4,B_o3, //9
	E_o4,G_o4,B_o4,E_o5,B_o4,D_o5,Fsh_o5,B_o5,D_o6, //9
	D_o5,Fsh_o5,D_o5,B_o4,D_o5,Fsh_o5,B_o5,D_o6,Dsh_o6, //9
	D_o6,Csh_o6,D_o5,D_o6,D_o5,Dsh_o5,D_o5,Csh_o5,Csh_o5, //9
	Csh_o5,Csh_o5,D_o4,D_o4,Dsh_o4,D_o4,Csh_o4,C_o4,D_o4, //9
	C_o4,Fsh_o4,B_o3,E_o4,G_o4,B_o4,E_o5,B_o4,G_o4,E_o4, //10
	B_o3,E_o4,G_o4,B_o4,E_o5,B_o4,Fsh_o5,B_o5,D_o6,Fsh_o5, //10
	D_o4,B_o4,Fsh_o5,Fsh_o5,G_o5,Fsh_o5,F_o5,F_o5,F_o5,F_o5,Fsh_o4, 0, 0, 0, 0}; 

// "Megalovania"
static unsigned int Megalovania[] = {//70 notes
  D_o4, D_o4, D_o5, 0, A_o4, 0, 0, Gsh_o4,0, G_o4, 0, F_o4, 0,D_o4,F_o4,G_o4,C_o4, C_o4, //18
  D_o5,0, A_o4, 0,0,Gsh_o4,0,G_o4,0,F_o4, 0, D_o4, F_o4, G_o4, B_o3, B_o3, D_o5, 0, A_o4, //19 
  0, 0, Gsh_o4, 0, G_o4, 0, F_o4, 0, D_o4, F_o4, G_o4, A_o3, A_o3, D_o5, 0, A_o4, 0, 0,  //18
  Gsh_o4, 0,  G_o4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   //15
};

//"Hedwig" theme from Harry Potter
static unsigned int Hedwig[]={ //140 notes total
D_o4,0,G_o4,0,0,Ash_o4,A_o4,G_o4,0, //9
0,0,D_o5,C_o5,0,0,0,A_o4, //8
0,0,0,0,G_o4,0,0,Ash_o4,  //8
A_o4,0,Fsh_o4,0,0,0,Gsh_o4,0, //8
D_o4,0,0,0,0,D_o4,0,G_o4, //8
Ash_o4,A_o4,0,G_o4,D_o5,F_o5,0,0, //8
E_o5,0,Dsh_o5,0,0,B_o4,0,Dsh_o5, //8
0,D_o5,Csh_o5,0,Csh_o4,0,0,Ash_o4, //8
G_o4,0,0,0,Ash_o4,0,D_o5,0,  //8
Ash_o4,0,D_o5,0,0,Ash_o4,Dsh_o5,0, //8
0,D_o5,Csh_o5,0,0,A_o4,Ash_o4,0, //8
D_o5,Csh_o5,0,Csh_o4,0,0,D_o4,D_o5, //8
0,0,0,0,Ash_o4,0,D_o4,0,  //8
0,Ash_o4,0,D_o4,0,Ash_o4,0,F_o5, //8
0,0,E_o5,0,Dsh_o5,0,0,B_o4,  //8
0,Dsh_o5,0,D_o5,Csh_o5,0,Csh_o4,0, //8
0,Ash_o4,0,G_o4,0,0,0,0,0,0, 0}; //11

//"Waltz in A minor by Chopin"
static unsigned int Amin[] = {//563 notes  
  E_o4,0,0,0,0,0, A_o4, 0,0, B_o4, 0, 0, C_o5, 0,0,0,0,0,C_o5,0,			 //20 1
	0,0,0,0,D_o5, 0,0,E_o5, 0,0, F_o5, 0,0,0,0,0,0,0,0,0, 							 //20 2
	0,0, B_o4,0,0,C_o5, 0,0,D_o5, 0,0, A_o5,0,0, G_o5, 0,0, F_o5, 0,0, 	 //20 3
	E_o5,F_o5,E_o5, Dsh_o5,0,0,E_o5,0,0,0,0,0,0,0,0,0,0,0,A_o4,0,  			 //20 4
	0,B_o4,0,0,C_o5,0,0,0,0,0,C_o5, 0,0,0,0,0,D_o5,0,0,E_o5, 						 //20 5
	0,0, F_o5, 0,0,0,0,0,0,0,0,0,0,0, B_o4, 0,0,C_o5, 0,0,  						 //20 6
	D_o5,0,0,A_o5, 0,0,G_o5,0,0,B_o4, 0,0, C_o5, 0,0,0,0,0,0,0, 				 //20 7
	0,0,0,0, E_o4, 0,0,0,0,0,A_o4, 0,0,B_o4,0,0,C_o5,0,0,0,  						 //20 8
	0,0,C_o5,0,0,0,0,0,D_o5,0,0,E_o5,0,0,F_o5,0,0,0,0,0, 								 //20 9
	0,0,0,0,0,0,B_o4,0,0,C_o5,0,0,D_o5, 0,0,A_o5, 0,0,G_o5, 0, 					 //20 10
	0, F_o5, 0,0, E_o5, F_o5, E_o5, Dsh_o5, 0,0,E_o5,0,0,0,0,0,0, 0, 0,0, //20 11
	0,0,C_o5,0,0,D_o5,0,0,E_o5,0,0,0,0,0,E_o5,0,0,0,0,0,  								//20 12
	F_o5,0,0,G_o5,0,0,A_o5,0,0,0,0,0,0,0,0,0,0,0,G_o5,0,  								//20 13
	0,0,0,0,Fsh_o5,0,0,G_o5,0,0,D_o6,0,0,F_o5,0,0,E_o5, 0,0,0,					  //20 14
	0,0,0,0,0,0,0,0,0,0,0,0,0,0, E_o5,0,0,Fsh_o5,0,0,  										//20 15
	Gsh_o5,0,0,A_o5,0,0,B_o5,0,0,C_o6,0,0,B_o5,C_o6,B_o5,A_o5,0,0,E_o5,0, //20 16
	0,B_o5,0,0,A_o5,0,0,0,0,0,A_o5,B_o5,A_o5,Gsh_o5,0,0,E_o5,0,0,F_o5,	  //20 17
	0,0,E_o5,0,0,0,0,0,E_o5,F_o5,E_o5,C_o5,0,0,A_o4,0,0,B_o4,0,0,				  //20 18
	A_o4,0,0,0,0,0,E_o4,0,Gsh_o4,0,B_o4,0,E_o5,Gsh_o5,B_o5,0,E_o6,Gsh_o6,B_o6,0,  //20 19
	0,0,0,C_o7,B_o6,C_o7,B_o6,A_o6,0,0,E_o6, 0,0, B_o6, 0,0, A_o6, 0,0,0,				  //20 20
	0,0, A_o6,B_o6,A_o6,Gsh_o6,0,0,E_o6,0,0,E_o7,0,0,E_o7,0,0,0,0,0,						  //20 21
	A_o6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,E_o5,0,  															//20 22
	0,Fsh_o5,0,0,Gsh_o5,0,0,A_o5,0,0,B_o5,0,0,C_o6,0,0,B_o5,C_o6,B_o5,A_o5, 		  //20 23
	0,0,A_o5,0,0,0,0,0,A_o5,B_o5,A_o5,Gsh_o5,0,0,E_o5,0,0,F_o5,0,0,  							//20 24
	E_o5,0,0,0,0,0,E_o5,F_o5,E_o5,C_o5,0,0,A_o4,0,0,B_o4,0,0,A_o4,0,  						//20 25
	0,0,0,0,E_o4,0,Gsh_o4,0,B_o4,0,E_o5,Gsh_o5,B_o5,0,E_o6,Gsh_o6,B_o6,0,0,0, 		//20 26
	0,C_o7,B_o6,C_o7,B_o6,A_o6,0,0,B_o6,0,0,A_o6,0,0,0,0,0,A_o6,B_o6,A_o6, 				//20 27
	Gsh_o6,0,0,E_o6,0,0,E_o7,0,0,E_o7,0,0,0,0,0,A_o6,0,0,0,0,0,0,0 								 //23
};
//"Virus by Beethoven" theme from the movie
static unsigned int Virus [] = {  //640
  E_o4,0,A_o4,0,B_o4,0,C_o5,0,0,0,0,0,D_o5,0,B_o4,0,0,0,0,0,  //20								1
	C_o5,0,A_o4,0,0,0,0,0,0,0,0,0,A_o4,Gsh_o4,A_o4,B_o4,C_o5,D_o5,E_o5,0,					//2
	0,0,E_o5,0,0,0,E_o5,0,0,0,E_o5,0,0,0,E_o5,0,0,0,0,0,													//3
	0,0,0,0,0,0,D_o5,0,E_o5,0,F_o5,0,0,0,0,0,0,0,B_o4,0,													//4
	0,0,C_o5,0,D_o5,0,E_o5,0,0,0,0,0,0,0,0,A_o4,0,0,0,A_o4,												//5
	0,B_o4,0,C_o5,0,0,0,0,0,D_o5,0,B_o4,0,0,0,0,0,C_o5,0,A_o4,										//6
	0,0,0,0,0,0,0,0,0,E_o4,0,A_o4,0,B_o4,0,C_o5,0,0,0,0,													//7
	0,D_o5,0,B_o4,0,0,0,0,0,C_o5,0,A_o4,0,0,0,0,0,0,0,0,													//8
	0,A_o4,Gsh_o4,A_o4,B_o4,C_o5,D_o5,E_o5,0,0,0,E_o5,0,0,0,E_o5,0,0,0,E_o5,			//9
	0,0,0,E_o5,0,0,0,0,0,0,0,0,0,0,0,D_o5,0,E_o5,0,F_o5,													//10
	0,0,0,0,0,0,0,B_o4,0,0,0,C_o5,0,D_o5,0,E_o5,0,0,0,0,													//11
	0,0,0,A_o4,0,0,0,A_o4,0,B_o4,0,C_o5,0,0,0,0,0,D_o5,B_o4,0,										//12
	0,0,0,0,C_o5,0,A_o4,0,0,0,0,0,0,0,0,0,E_o4,0,A_o4,0,													//13
	B_o4,0,C_o5,0,0,0,0,0,D_o5,0,B_o4,0,0,0,0,0,C_o5,0,A_o4,0,										//14
	0,0,0,0,0,0,0,0,A_o4,Gsh_o4,A_o4,B_o4, C_o5,D_o5,E_o5,0,0,0,E_o5,0,						//15
	0,0,E_o5,0,0,0,E_o5,0,0,0,0,0,0,0,0,0,0,0,D_o5,0,															//16
	E_o5,0,F_o5,0,0,0,0,0,0,0,B_o4,0,0,0,C_o5,0,D_o5,0,E_o5,0,										//17
	0,0,0,0,0,0,A_o4,0,0,0,A_o4,0,B_o4,0,C_o5,0,0,0,0,0,													//18
	D_o5,0,B_o4,0,0,0,0,0,C_o5,0,A_o4,0,0,0,0,0,0,0,0,0,													//19
	A_o4,0,Gsh_o4,0,A_o4,0,B_o4,0,E_o4,0,Dsh_o4,0,E_o4,0,B_o4,0,E_o4,0,E_o5,0,		//20
	D_o5,0,D_o5,0,C_o5,0,B_o4,0,A_o4,0,A_o4,0,B_o4,0,C_o5,0,A_o4,0,B_o4,0,				//21
	G_o4,0,Fsh_o4,0,G_o4,0,B_o4,0,G_o4,0, G_o5,0,G_o5,0,F_o5,0,F_o5,0,E_o5,0,			//22
	D_o5,0,C_o5,0,C_o5,0,D_o5,0,E_o5,0,C_o5,0,Csh_o5,0,A_o4,0,Csh_o5,0,E_o5,0,		//23
	A_o5,0,0,0,G_o5,0,0,0,F_o5,0,E_o5,0,D_o5,0,Csh_o5,0,D_o5,0,A_o4,0,						//24
	Gsh_o4,0,A_o4,0,Dsh_o5,0,B_o4,0,Dsh_o5,0,Fsh_o5,0,C_o6,0,0,0,Dsh_o5,0,B_o5,0,	//25
	A_o5,0,A_o5,0,D_o5,0,A_o5,0,Gsh_o5,0, A_o5,0,B_o5,0,E_o5,0,E_o5,0,0,0,				//26
	0,0, D_o5, C_o5,B_o4,0,C_o5,0,D_o5,0,E_o4,0,C_o5,0,B_o4,0,A_o4,0,Gsh_o4,0,		//27
	A_o4,0,B_o4,0,C_o5,0,A_o4,0,B_o4,0,0,0,0,G_o4,0,B_o4,0,G_o5,0,0,							//28
	0,0,F_o5,0,0,0,E_o5,0,D_o5,E_o5,D_o5,0,C_o5,0,B_o4,0,C_o5,0,D_o5,0,						//29
	C_o5,0,Ash_o5,0,0,0,0,G_o5,Ash_o5,0,G_o5,0,Ash_o5,0,E_o5,0,A_o5,0,G_o5,0,			//30
	A_o5,0,F_o5,0,A_o5,0,E_o5,0,A_o5,0,Dsh_o5,0,0,0,0,0,B_o4,0,Dsh_o5,0,					//31
	C_o6,0,0,0,Dsh_o5,0,0,0,B_o5,0,0,0,D_o4,0,E_o4,0,F_o4,0,G_o4,0								//32
};
unsigned int play_music (unsigned int song);
void buzz(unsigned long dutyc, int length); 
void delayMicroseconds(unsigned long val);
void TPM_setup (void);
void Init_DAC(void);
void Init_SineTable(void);
void Init_DMA_For_Playback(uint16_t * source, uint32_t count);
void Start_DMA_Playback(void);
void Delay_us(volatile unsigned int time_del);
int main (void) {
	SIM ->SCGC5 |= (SIM_SCGC5_PORTA_MASK) | (SIM_SCGC5_PORTD_MASK) | (SIM_SCGC5_PORTE_MASK) | SIM_SCGC5_PORTC_MASK;  //enable clock for port B
						//uint8_t count = 0;
						//uint8_t dec_hex=16; //change to 10 for decimal
						//short seg7 = noswitch;
/* This function shows value of count on display the decimal point is 
displayed if dp=1
count must be less than 10 for decimal, or less than 16 for Hex. */
		PORTA->PCR[SW1] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
		PORTD->PCR[SW2] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
		PORTA->PCR[SW3] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
		PORTA->PCR[SW4] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
	
		PORTC->PCR[SOUND_OUT] &= ~PORT_PCR_MUX_MASK; //Select GPIO
		PORTC->PCR[SOUND_OUT] |= PORT_PCR_MUX(4);
		PTA->PDDR |= MASK(SOUND_OUT); //Declaring button for SW1 state as INPUT
	
		PTA->PDDR &= ~MASK(SW1); //Declaring button for SW1 state as INPUT
    PTD->PDDR &= ~MASK(SW2); //Declaring button for SW2 state as INPUT
		PTA->PDDR &= ~MASK(SW3); //Declaring button for SW3 state as INPUT
    PTA->PDDR &= ~MASK(SW4); //Declaring button for SW4 state as INPUT	
	
		Init_DAC();
		Init_SineTable ();
		Init_DMA_For_Playback(SineTable, NUM_STEPS);
		TPM_setup ();
		Start_DMA_Playback();
while(1){
	/*
	if (PORTA->ISFR & MASK(SW1)) {
			play_music (1);
		PORTA->ISFR &= 0xffffffff;
							}
	else if (PORTD->ISFR & MASK(SW2)) {
			play_music (2);
			PORTD->ISFR &= 0xffffffff;
							}
	else if (PORTA->ISFR & MASK(SW3)) {
			play_music (3);
			PORTA->ISFR &= 0xffffffff;
							}
	else if (PORTA->ISFR & MASK(SW4)) {
			play_music (4);
			PORTA->ISFR &= 0xffffffff;
							}
	else  {
			play_music (3);
			//PORTD->ISFR &= 0xffffffff;
							}	*/
							play_music (2);
						}
}
unsigned int play_music (unsigned int song) {
	//unsigned int size;
	//uint16_t i=0;
	uint16_t delay = 0;
switch(song){	
	case 0: //Wait for the first song to initialize
    period = 0;  //Play nothing while we wait
    dutyCycle = 0;
		break;
  case 1:
		//size = sizeof(Knights)/sizeof(unsigned int);
    // "Montagues and Capulets" a.k.a "Dance of the Knights" by Sergei Prokofiev
	  period = 24000000/(Knights[note]*64); //Period = Clock/Frequency
    dutyCycle = period/2;               //50% duty cycle
	 //delayMicroseconds(dutyCycle); // wait for the calculated delay value 
    if (note<80) //If there are more notes in the array, go to the next note
      note++;
    if (note>=80){//If there are no more notes in the array, start the song over
      note = 0;
    } 
		
			break;
  case 2:
				period = 24000000/(Hedwig[note]*64);      //Period = Clock/Frequency
				dutyCycle = period/2;               //50% duty cycle
			//int delnote = 1000/Hedwig[note];
	// for (delay = 0;delay<1000; delay++) {	
		// delay++;
	if (note<140) {//If there are more notes in the array, go to the next note
			if (delay<10000)
				delay++;
      note++;
			TPM0->CONTROLS[1].CnV=  dutyCycle;
		  TPM0->MOD = period;
			//	for (delay=0; delay<10000; delay++)
								//		;} 
	} 
    if (note>=140){//If there are no more notes in the array, start the song over
      note = 0;
    }	
		
	//} 
		break;
  case 3:
	period = 24000000/(Megalovania[note]*64);        //Period = Clock/Frequency
    dutyCycle = period/2;               //50% duty cycle
   /* if (note<70) {//If there are more notes in the array, go to the next note
			note++;
		}
    if (note>=70){//If there are no more notes in the array, start the song over
      note = 0;
    }
		delayMicroseconds(1000); */
	
		for (note=0; note<70; note++) {
			TPM0->CONTROLS[1].CnV=  dutyCycle;
		  TPM0->MOD = period;
				for (delay=0; delay<10; delay++)
										;
		}
	
			break;
	case 4:
				period = 24000000/(Virus[note]*64);      //Period = Clock/Frequency
				dutyCycle = period/2;               //50% duty cycle
		if (note<640) {//If there are more notes in the array, go to the next note
      note++; }
    if (note>=640){//If there are no more notes in the array, start the song over
      note = 0;
    }	
		
    
		break;
	}
    TPM0->CONTROLS[1].CnV=  dutyCycle;
		TPM0->MOD = period;
	return song;
}	


void TPM_setup (void) {

SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

SIM->SOPT2 &= ~SIM_SOPT2_PLLFLLSEL_MASK;
SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL(0);

//Load the counter and the mod
// TPM0->MOD = 6;

//Set TPM Count direction to up with a divide by prescaler
TPM0->SC &= ~((TPM_SC_CPWMS_MASK) | (TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
TPM0->SC  = TPM_SC_CMOD(1) | TPM_SC_PS(6) | TPM_SC_TOIE_MASK | TPM_SC_CPWMS_MASK;

//Continue operation in debug mode
TPM0->CONF |= TPM_CONF_DBGMODE(3);

//Set channel 1 to edge-aligned low-true PWM
TPM0->CONTROLS[1].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK;

//Set channel TPM0 Channel 0 to edge-aligned low true PWM
TPM0_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK));
TPM0_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0) |  TPM_CnSC_MSB(1)  | TPM_CnSC_MSA(0));

//Set initial duty cycle
 TPM0->CONTROLS[1].CnV = 4;

//Start TPM
TPM0->SC |= TPM_SC_CMOD(1);


}

void Init_DAC(void) {
  // Init DAC output
	
	SIM->SCGC6 |= (1UL << SIM_SCGC6_DAC0_SHIFT); 
	SIM->SCGC5 |= (1UL << SIM_SCGC5_PORTD_SHIFT); 
	
	PORTD->PCR[SOUND_OUT] &= ~(PORT_PCR_MUX(7));	// Select analog 
	PORTD->PCR[SOUND_OUT] |= PORT_PCR_MUX(4);
	//PTD->PTOR = MASK(SOUND_OUT);
		
	// Disable buffer mode
	DAC0->C1 = 0;
	DAC0->C2 = 0;
	
	// Enable DAC, select VDDA as reference voltage
	DAC0->C0 = (1 << DAC_C0_DACEN_SHIFT) | (1 << DAC_C0_DACRFS_SHIFT);

}

void Init_SineTable(void) {
	unsigned int n;
	
	for (n=0; n<NUM_STEPS; n++) {
		SineTable[n] = ((MAX_DAC_CODE/2)*(1+sin(n*2*3.1415927/NUM_STEPS)));
	}
}
	
	void Init_DMA_For_Playback(uint16_t * source, uint32_t count) {
	// Save reload information
	Reload_DMA_Source = source;
	Reload_DMA_Byte_Count = count*2;
	
	// Gate clocks to DMA and DMAMUX
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

	// Disable DMA channel to allow configuration
	DMAMUX0->CHCFG[0] = 0;
	
	// Generate DMA interrupt when done
	// Increment source, transfer words (16 bits)
	// Enable peripheral request
	DMA0->DMA[0].DCR = DMA_DCR_EINT_MASK | DMA_DCR_SINC_MASK | 
											DMA_DCR_SSIZE(2) | DMA_DCR_DSIZE(2) |
											DMA_DCR_ERQ_MASK | DMA_DCR_CS_MASK;
	
	// Configure NVIC for DMA ISR
	NVIC_SetPriority(DMA0_IRQn, 2);
	NVIC_ClearPendingIRQ(DMA0_IRQn); 
	NVIC_EnableIRQ(DMA0_IRQn);	

	// Enable DMA MUX channel with TPM0 overflow as trigger
	DMAMUX0->CHCFG[0] = DMAMUX_CHCFG_SOURCE(54);   
}
	void Start_DMA_Playback() {
	// initialize source and destination pointers
	DMA0->DMA[0].SAR = DMA_SAR_SAR((uint32_t) Reload_DMA_Source);
	DMA0->DMA[0].DAR = DMA_DAR_DAR((uint32_t) (&(DAC0->DAT[0])));
	// byte count
	DMA0->DMA[0].DSR_BCR = DMA_DSR_BCR_BCR(Reload_DMA_Byte_Count);
	// clear done flag 
	DMA0->DMA[0].DSR_BCR &= ~DMA_DSR_BCR_DONE_MASK; 
	// set enable flag
	DMAMUX0->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK;

}


	void Delay_us(volatile unsigned int time_del) {
	// This is a very imprecise and fragile implementation!
	time_del = 9*time_del + time_del/2; 
	while (time_del--) {
		;
	}
}