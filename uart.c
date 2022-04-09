#include "MKL25Z4.h"                    // Device header
#include <math.h>

#define SOUND_OUT (20) // Sound Output in PORTE
#define SW1 (12) // Buttons to choose music
#define SW2 (4) // Buttons to choose music
#define SW3 (2) // Buttons to choose music
#define SW4 (1) // Buttons to choose music

#define clock (12000000U) // current clock for TPM
#define secondScalar (1000000) // counter to get time in seconds
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

// "Montagues and Capulets" a.k.a "Dance of the Knights" by Sergei Prokofiev
static uint16_t Knights[] = { //80 notes total
	B_o3, E_o4,G_o4,B_o4,E_o5,B_o4,G_o4,E_o4,B_o3, //9
	E_o4,G_o4,B_o4,E_o5,B_o4,D_o5,Fsh_o5,B_o5,D_o6, //9
	D_o5,Fsh_o5,D_o5,B_o4,D_o5,Fsh_o5,B_o5,D_o6,Dsh_o6, //9
	D_o6,Csh_o6,D_o5,D_o6,D_o5,Dsh_o5,D_o5,Csh_o5,Csh_o5, //9
	Csh_o5,Csh_o5,D_o4,D_o4,Dsh_o4,D_o4,Csh_o4,C_o4,D_o4, //9
	C_o4,Fsh_o4,B_o3,E_o4,G_o4,B_o4,E_o5,B_o4,G_o4,E_o4, //10
	B_o3,E_o4,G_o4,B_o4,E_o5,B_o4,Fsh_o5,B_o5,D_o6,Fsh_o5, //10
	D_o4,B_o4,Fsh_o5,Fsh_o5,G_o5,Fsh_o5,F_o5,F_o5,F_o5,F_o5,Fsh_o4, 0, 0, 0, 0}; 

//"Hedwig" theme from Harry Potter
static uint16_t Hedwig[]={ //140 notes total
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
static uint16_t Amin[] = {//563 notes  
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
static uint16_t Virus [] = {  //640
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


//Global Music Variables
static uint8_t currentSong = 0; // get the current song
static uint8_t prevSong = 0; // Ensure songs have changed if button pressed?
static uint16_t currentNote = 0; // Get the current note to be used as TPM mod
static uint16_t nextNote = 0; // get the next note pre-emptively
static uint16_t noteIndex = 0; // store the current note
static uint16_t *songPtr = 0; // grab the ptr for faster note preparation
static uint16_t songSize = 0; // gets the song size of calculation
static uint16_t songTempo = 80; // Beats per minute
static float noteDelay = .25; // roughly .25 seconds
static uint32_t cntOverflow = 0; // counts the overflows
static uint32_t songCnt = 0; // Counts the total time spent

enum songChoice
{
	none = 0,
	knights,
	hedwig,
	amin,
	virus
};

uint16_t calcNoteDelay(void); // calculates the delay between notes
void play_music (uint8_t song); // selects the song to be played and set defaults
void TPM_setup (void); // inits the TPM to output to PORTE pin 20
void initSoundSwPins (void);

int main (void) {
	
	initSoundSwPins();
	TPM_setup();
	currentSong = knights;
	
	while(1){
			
		if (PORTA->ISFR & MASK(SW1))
		{
			currentSong = knights;
			PORTA->ISFR &= 0xffffffff;
		}
		else if (PORTD->ISFR & MASK(SW2))
		{
			currentSong = hedwig;
			PORTD->ISFR &= 0xffffffff;
		}
		else if (PORTA->ISFR & MASK(SW3))
		{
			currentSong = amin;
			PORTA->ISFR &= 0xffffffff;
		}
		else if (PORTA->ISFR & MASK(SW4))
		{
			currentSong = virus;
			PORTA->ISFR &= 0xffffffff;
		}
		else  
		{
			currentSong = none;
		}
		if(currentSong != prevSong)
		{
			noteIndex = 0; // reset the song index
			play_music(currentSong);	
			prevSong = currentSong;
		}
	}
}

void play_music(uint8_t song) {
	
	uint8_t playsong = 0;
	
	switch(song){		
		case 0: //Wait for the first song to initialize
			TPM1->SC &= TPM_SC_CMOD(0); // turn off the TPM
			playsong = 0;
		break;
		
		case 1:
			// "Montagues and Capulets" a.k.a "Dance of the Knights" by Sergei Prokofiev
			songPtr = &Knights[0];
			songSize = sizeof(Knights)/sizeof(uint16_t);
			playsong = 1;
			break;
		
		case 2:
			songPtr = &Hedwig[0];
			songSize = sizeof(Hedwig)/sizeof(uint16_t);
			playsong = 1;
			break;
		
		case 3:
			songPtr = &Amin[0];
			songSize = sizeof(Amin)/sizeof(uint16_t);
			playsong = 1;
			break;
		
		case 4:
			songPtr = &Virus[0];
			songSize = sizeof(Virus)/sizeof(uint16_t);
			playsong = 1;
			break;
		
		default:
			TPM1->SC &= TPM_SC_CMOD(0); // turn off the TPM
			playsong = 0;
			break;
	}	
	
	if(playsong == 1)
	{
		currentNote = songPtr[noteIndex];
		nextNote = songPtr[++noteIndex];
		TPM1->MOD = currentNote - 1;
		TPM1->SC |= TPM_SC_CMOD(1); // turn on the tmp
	}
	else
	{
		// do nothing
	}
}

void TPM_setup (void) {

	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

	SIM->SOPT2 &= ~SIM_SOPT2_PLLFLLSEL_MASK;
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL(0);

	//Set TPM Count direction to up with a divide by prescaler
	TPM1->SC &= ~((TPM_SC_CPWMS_MASK) | (TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM1->SC  = TPM_SC_CMOD(1) | TPM_SC_PS(2) | TPM_SC_TOIE_MASK | TPM_SC_CPWMS_MASK;

	//Continue operation in debug mode
	TPM1->CONF |= TPM_CONF_DBGMODE(3);

	//Set channel TPM0 Channel 0 to edge-aligned low true PWM
	TPM1->CONTROLS[0].CnSC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK));
	TPM1->CONTROLS[0].CnSC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0) |  TPM_CnSC_MSB(1)  | TPM_CnSC_MSA(0));

	//TPM is off initially
	TPM1->SC &= TPM_SC_CMOD(0);
}

void initSoundSwPins()
{
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK) | (SIM_SCGC5_PORTD_MASK) | (SIM_SCGC5_PORTE_MASK);  //enable clock for port B

	PORTE->PCR[SOUND_OUT] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SOUND_OUT] |= PORT_PCR_MUX(1); // set to GPIO pin
	
	PTE->PDDR |= MASK(SOUND_OUT);
	
	PORTA->PCR[SW1] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
	PORTD->PCR[SW2] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
	PORTA->PCR[SW3] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
	PORTA->PCR[SW4] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);

	PTA->PDDR &= ~MASK(SW1); //Declaring button for SW1 state as INPUT
	PTD->PDDR &= ~MASK(SW2); //Declaring button for SW2 state as INPUT
	PTA->PDDR &= ~MASK(SW3); //Declaring button for SW3 state as INPUT
	PTA->PDDR &= ~MASK(SW4); //Declaring button for SW4 state as INPUT	
}

void TPM1_IRQHandler()
{
	// On overflow, change the note to the next note, and get the next note
	currentNote = nextNote;
	TPM1->MOD = currentNote;
	if(noteIndex < songSize)
	{
		nextNote = clock/songPtr[noteIndex];
	}
	else
	{
		// Song ends next time
		// set current song back to 0
		currentSong = none;
	}
}
