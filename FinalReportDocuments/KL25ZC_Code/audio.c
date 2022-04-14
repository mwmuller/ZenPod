#include "MKL25Z4.h"                    // Device header
#include <math.h>

#define CLOCK_AUDIO (24000000)
#define SOUND_OUT (2) // Sound Output in PORTC
#define SW1 (12) // Buttons to choose music
#define SW2 (4) // Buttons to choose music
#define SW3 (2) // Buttons to choose music
#define SW4 (1) // Buttons to choose music

#define EN       4 
#define RW   		 2  
#define RS       1 


#define MAX_DAC_CODE (4095)
#define NUM_STEPS (512)

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
static uint16_t noteIndex; // get the note index
static unsigned int * songPtr; // get the ptr to the song
static uint8_t curSong = 0; // stores the current song
static uint16_t songLength = 0; // holds the length of the song

// "Hungarian Dance No.5 by Johannes Brahms"
static unsigned int Hungarian_Dance[] = { //80 notes total
	D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,G_o4,G_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,G_o4,G_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,G_o4,A_o4,G_o4,G_o4,  //26
	G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,C_o4,C_o4,C_o4,C_o4,C_o4,C_o4,D_o4,Dsh_o4,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,C_o4,Ash_o3,10,Ash_o3,A_o3,
	10,A_o3,A_o3,A_o3,D_o4,G_o3,G_o3,G_o3,G_o3,G_o3,G_o3,G_o3,G_o3,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,G_o4,Ash_o4,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
	Ash_o4,Ash_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,Ash_o4,C_o5,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Dsh_o5,F_o5,G_o5,Dsh_o5,D_o5,Dsh_o5,F_o5,D_o5,
	C_o5,D_o5,Dsh_o5,C_o5,Ash_o4,C_o5,D_o5,Ash_o4,C_o5,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Dsh_o5,F_o5,G_o5,Dsh_o5,D_o5,Dsh_o5,F_o5,D_o5,
	C_o5,D_o5,Dsh_o5,C_o5,Ash_o4,C_o5,D_o5,Ash_o4,C_o5,Ash_o4,10,Ash_o4,A_o4,10,A_o4,A_o4,A_o4,Ash_o4,G_o4,G_o4,G_o4,G_o4,G_o5,G_o5,G_o5,G_o5,D_o4,D_o4,
	D_o4,D_o4,D_o4,D_o4,G_o4,G_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,G_o4,G_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,G_o4,A_o4,G_o4,G_o4,G_o4,G_o4,
	G_o4,G_o4,G_o4,G_o4,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,Dsh_o5,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,C_o5,Ash_o4,10,Ash_o4,A_o4,10,A_o4,A_o4,
	A_o4,D_o5,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,D_o3,D_o3,D_o3,G_o3,Ash_o3,D_o4,G_o4,Ash_o4,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,Ash_o4,Ash_o4,
	A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,Ash_o4,C_o5,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Dsh_o4,Dsh_o4,G_o4,G_o4,D_o4,D_o4,F_o4,F_o4,C_o4,C_o4,
	Dsh_o4,Dsh_o4,Ash_o3,Ash_o3,D_o4,D_o4,C_o4,C_o4,Ash_o3,Ash_o3,A_o3,A_o3,D_o4,D_o4,G_o3,G_o3,G_o3,G_o3,G_o4,G_o4,G_o4,G_o4,D_o4,D_o4,D_o4,D_o4,10,
	D_o4,D_o4,D_o4,D_o4,Dsh_o4,Dsh_o4,Dsh_o4,Dsh_o4,Dsh_o4,Dsh_o4,D_o4,D_o4,D_o4,D_o4,C_o4,C_o4,C_o4,C_o4,B_o3,C_o4,D_o4,C_o4,B_o3,D_o4,C_o4,C_o4,
	C_o4,C_o4,10,C_o4,C_o4,C_o4,C_o4,10,C_o4,C_o4,C_o4,C_o4,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,C_o4,C_o4,C_o4,C_o4,Ash_o3,Ash_o3,Ash_o3,Ash_o3,A_o3,Ash_o3,
	C_o4,Ash_o3,A_o3,C_o4,Ash_o3,Ash_o3,Ash_o3,Ash_o3,C_o4,C_o4,C_o4,C_o4,10,C_o4,C_o4,C_o4,C_o4,Dsh_o4,Dsh_o4,D_o4,D_o4,D_o4,D_o4,C_o4,C_o4,C_o4,C_o4,
	Ash_o3,Ash_o3,Ash_o3,Ash_o3,A_o3,Ash_o3,C_o4,Ash_o3,A_o3,C_o4,Ash_o3,Ash_o3,Ash_o3,Ash_o3,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,10,D_o4,E_o4,E_o4,E_o4,E_o4,
	Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,G_o4,10,G_o4,G_o4,G_o4,G_o4,Fsh_o4,G_o4,A_o4,G_o4,Fsh_o4,A_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,B_o3,10,B_o3,10,
	B_o3,10,B_o3,B_o3,A_o3,A_o3,G_o3,G_o3,A_o3,A_o3,A_o3,A_o3,B_o3,B_o3,B_o3,10,B_o3,B_o3,B_o3,B_o3,D_o4,10,D_o4,10,D_o4,10,
	D_o4,D_o4,C_o4,C_o4,B_o3,B_o3,A_o3,A_o3,A_o3,A_o3,D_o4,D_o4,D_o4,10,D_o4,10,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,G_o4,G_o4,Ash_o4,Ash_o4,
	Ash_o4,Ash_o4,Ash_o4,Ash_o4,G_o4,G_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,Fsh_o4,G_o4,A_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,C_o4,C_o4,C_o4,C_o4,
	C_o4,C_o4,D_o4,Dsh_o4,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,Ash_o3,C_o4,Ash_o3,10,Ash_o3,A_o3,10,A_o3,A_o3,A_o3,D_o4,G_o3,G_o3,G_o3,G_o3,G_o3,G_o3,
	G_o3,G_o3,D_o3,D_o3,D_o3,G_o3,Ash_o3,D_o4,G_o4,Ash_o4,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,Ash_o4,Ash_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,Ash_o4,C_o5,
	Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Ash_o4,Dsh_o4,Dsh_o4,G_o4,G_o4,D_o4,D_o4,F_o4,F_o4,C_o4,C_o4,Dsh_o4,Dsh_o4,Ash_o3,Ash_o3,D_o4,D_o4,C_o4,C_o4,
	Ash_o3,Ash_o3,A_o3,A_o3,D_o4,D_o4,G_o3,G_o3,G_o3,G_o3,G_o4,G_o4,G_o4,G_o4,D_o4,D_o4,D_o4,10,D_o4,D_o4,D_o4,10,Dsh_o4,Dsh_o4,Dsh_o4,Dsh_o4}; 


// "Lacrimosa by Wolfgang Amadeus Mozart"
static unsigned int Lacrimosa[]={ //140 notes total
Csh_o5,Csh_o5,Csh_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,D_o5,D_o5,D_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,
C_o6,C_o6,C_o6,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,A_o5,A_o5,A_o5,D_o6,D_o6,D_o6,D_o6,D_o6,D_o6,Ash_o5,Ash_o5,Ash_o5,G_o5,G_o5,G_o5,G_o5,
G_o5,G_o5,E_o5,E_o5,E_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,A_o5,A_o5,A_o5,Csh_o5,Csh_o5,Csh_o5,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,
A_o4,A_o4,A_o4,F_o5,F_o5,F_o5,D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,
Csh_o5,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,F_o5,F_o5,F_o5,D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,D_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,D_o4,E_o4,E_o4,E_o4,E_o4,E_o4,E_o4,
E_o4,E_o4,E_o4,F_o4,F_o4,F_o4,F_o4,F_o4,F_o4,F_o4,F_o4,F_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,A_o4,A_o4,A_o4,A_o4,A_o4,
A_o4,A_o4,A_o4,A_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,D_o5,D_o5,D_o5,D_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,F_o5,F_o5,F_o5,
F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,Gsh_o5,Gsh_o5,
Gsh_o5,Gsh_o5,Gsh_o5,Gsh_o5,Gsh_o5,Gsh_o5,Gsh_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,A_o4,
A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,Ash_o4,Ash_o4,Ash_o4,A_o4,A_o4,10,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,
A_o4,A_o4,A_o4,F_o5,F_o5,F_o5,D_o5,D_o5,D_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,10,Csh_o5,Csh_o5,Csh_o5,E_o5,E_o5,
10,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,
Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,G_o5,G_o5,G_o5,Dsh_o5,Dsh_o5,Dsh_o5,10,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,F_o5,F_o5,F_o5,E_o5,E_o5,E_o5,10,E_o5,E_o5,E_o5,E_o5,E_o5,
E_o5,E_o5,E_o5,E_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,G_o5,
G_o5,G_o5,F_o5,F_o5,F_o5,10,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,10,E_o5,E_o5,E_o5,
E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,A_o4,A_o4,A_o4,G_o4,G_o4,G_o4,10,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,G_o4,F_o4,F_o4,
F_o4,F_o4,F_o4,F_o4,F_o4,F_o4,F_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,
D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,10,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,
B_o4,10,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,D_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,F_o5,F_o5,F_o5,10,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,D_o5,D_o5,D_o5,
B_o4,B_o4,B_o4,C_o5,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,Ash_o4,Ash_o4,Ash_o4,G_o4,G_o4,G_o4,F_o4,F_o4,F_o4,10,F_o4,F_o4,
F_o4,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o4,F_o4,F_o4,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o4,F_o4,F_o4,F_o5,F_o5,F_o5,A_o4,A_o4,A_o4,C_o5,
C_o5,C_o5,F_o5,F_o5,F_o5,10,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,D_o5,D_o5,D_o5,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,
C_o6,C_o6,C_o6,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,Ash_o5,Ash_o5,Ash_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,E_o5,E_o5,E_o5,F_o5,F_o5,F_o5,F_o5,F_o5,
F_o5,Csh_o6,Csh_o6,Csh_o6,D_o6,D_o6,D_o6,D_o6,D_o6,D_o6,D_o5,D_o5,D_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,A_o5,A_o5,A_o5,A_o4,A_o4,A_o4,10,A_o4,
A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,F_o5,F_o5,F_o5,D_o5,D_o5,D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,F_o5,F_o5,F_o5,D_o5,D_o5,
D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,Csh_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,D_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o6,D_o6,D_o6,D_o6,D_o6,D_o6,D_o6,D_o6,D_o6,C_o6,C_o6,C_o6,C_o6,C_o6,C_o6,
C_o6,C_o6,C_o6,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,G_o5,G_o5,G_o5,G_o5,G_o5,
G_o5,G_o5,G_o5,G_o5,Fsh_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,G_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,D_o5,D_o5,D_o5,D_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,E_o5,E_o5,E_o5,
E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,10,E_o5,E_o5,
E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,Csh_o5,Csh_o5,Csh_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,G_o5,G_o5,G_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,Fsh_o5,
Fsh_o5,Fsh_o5,Ash_o5,Ash_o5,Ash_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,D_o6,D_o6,D_o6,C_o6,C_o6,C_o6,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,
D_o5,10,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5,D_o5}; //11

//"Waltz in A minor by Chopin"
static unsigned int Amin[] = {//563 notes  
  E_o4,E_o4,E_o4,E_o4,E_o4,E_o4, A_o4, A_o4,A_o4, B_o4, B_o4, B_o4, C_o5, C_o5,C_o5,C_o5,C_o5, 10,C_o5,C_o5,			 //20 1
	C_o5,C_o5,C_o5,10,D_o5, D_o5,D_o5,E_o5, E_o5,E_o5, F_o5, F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,
	B_o4,B_o4,B_o4,C_o5, C_o5,C_o5,D_o5, D_o5,D_o5, A_o5,A_o5,A_o5, G_o5, G_o5,G_o5, F_o5, F_o5,F_o5, 	 //20 3
	E_o5,F_o5,E_o5, Dsh_o5,Dsh_o5,Dsh_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,A_o4,A_o4,  			 //20 4
	A_o4,B_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5, 10,C_o5, C_o5,C_o5,C_o5,C_o5,10,D_o5,D_o5,D_o5,E_o5, 						 //20 5
	E_o5,E_o5, F_o5, F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5, B_o4, B_o4,B_o4,C_o5, C_o5,C_o5,  						 //20 6
	D_o5,D_o5,D_o5,A_o5, A_o5,A_o5,G_o5,G_o5,G_o5,B_o4, B_o4,B_o4, C_o5, C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,10,
	E_o4, E_o4,E_o4,E_o4,E_o4,E_o4,A_o4, A_o4,A_o4,B_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,  						 //20 8
	C_o5,10,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,D_o5,D_o5,E_o5,E_o5,E_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5, 								 //20 9
	F_o5,F_o5,B_o4,B_o4,B_o4,C_o5,C_o5,C_o5,D_o5, D_o5,D_o5,A_o5, A_o5,A_o5,G_o5, G_o5, 					 //20 10
	G_o5, F_o5, F_o5,F_o5, E_o5, F_o5, E_o5, Dsh_o5, Dsh_o5,Dsh_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5, E_o5, 
	C_o5,C_o5,C_o5,D_o5,D_o5,D_o5,E_o5,E_o5,E_o5,E_o5,E_o5,10,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,  								//20 12
	F_o5,F_o5,F_o5,G_o5,G_o5,G_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,A_o5,G_o5,G_o5,  								//20 13
	G_o5,G_o5,G_o5,10,Fsh_o5,Fsh_o5,Fsh_o5,G_o5,G_o5,G_o5,D_o6,D_o6,D_o6,F_o5,F_o5,F_o5,E_o5, E_o5,E_o5,E_o5,					  //20 14
	E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5, E_o5,E_o5,E_o5,Fsh_o5,Fsh_o5,Fsh_o5,  										//20 15
	Gsh_o5,Gsh_o5,Gsh_o5,A_o5,A_o5,A_o5,B_o5,B_o5,B_o5,C_o6,C_o6,C_o6,B_o5,C_o6,B_o5,A_o5,A_o5,A_o5,E_o5,E_o5, //20 16
	E_o5,B_o5,B_o5,B_o5,A_o5,A_o5,A_o5,A_o5,A_o5,10,A_o5,B_o5,A_o5,Gsh_o5,Gsh_o5,Gsh_o5,E_o5,E_o5,E_o5,F_o5,	  //20 17
	F_o5,F_o5,E_o5,E_o5,E_o5,E_o5,E_o5,10,E_o5,F_o5,E_o5,C_o5,C_o5,C_o5,A_o4,A_o4,A_o4,B_o4,B_o4,B_o4,				  //20 18
	A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,E_o4,E_o4,Gsh_o4,Gsh_o4,B_o4,B_o4,E_o5,Gsh_o5,B_o5,B_o5,E_o6,Gsh_o6,B_o6,B_o6,  //20 19
	B_o6,B_o6,B_o6,C_o7,B_o6,C_o7,B_o6,A_o6,A_o6,A_o6,E_o6, E_o6,E_o6, B_o6, B_o6,B_o6, A_o6, A_o6,A_o6,A_o6,				  //20 20
	A_o6,10, A_o6,B_o6,A_o6,Gsh_o6,Gsh_o6,Gsh_o6,E_o6,E_o6,E_o6,E_o7,E_o7,10,E_o7,E_o7,E_o7,E_o7,E_o7,E_o7,						  //20 21
	A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,E_o5,E_o5,  															//20 22
	E_o5,Fsh_o5,Fsh_o5,Fsh_o5,Gsh_o5,Gsh_o5,Gsh_o5,A_o5,A_o5,A_o5,B_o5,B_o5,B_o5,C_o6,C_o6,C_o6,B_o5,C_o6,B_o5,A_o5, 		  //20 23
	A_o5,10,A_o5,A_o5,A_o5,A_o5,A_o5,10,A_o5,B_o5,A_o5,Gsh_o5,Gsh_o5,Gsh_o5,E_o5,E_o5,E_o5,F_o5,F_o5,F_o5,  							//20 24
	E_o5,E_o5,E_o5,E_o5,E_o5,10,E_o5,F_o5,E_o5,C_o5,C_o5,C_o5,A_o4,A_o4,A_o4,B_o4,B_o4,B_o4,A_o4,A_o4,  						//20 25
	A_o4,A_o4,A_o4,A_o4,E_o4,E_o4,Gsh_o4,Gsh_o4,B_o4,B_o4,E_o5,Gsh_o5,B_o5,B_o5,E_o6,Gsh_o6,B_o6,B_o6,B_o6,B_o6, 		//20 26
	B_o6,C_o7,B_o6,C_o7,B_o6,A_o6,A_o6,A_o6,B_o6,B_o6,B_o6,A_o6,A_o6,A_o6,A_o6,A_o6,10,A_o6,B_o6,A_o6, 				//20 27
	Gsh_o6,Gsh_o6,Gsh_o6,E_o6,E_o6,E_o6,E_o7,E_o7,E_o7,10,E_o7,E_o7,E_o7,E_o7,E_o7,E_o7,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6,A_o6 								 //23
};
//"Virus by Beethoven" theme from the movie
static unsigned int Virus [] = {  //640
  E_o4,E_o4,A_o4,A_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5,D_o5,D_o5,B_o4,B_o4,B_o4,B_o4,B_o4,  //20								1
	C_o5,C_o5,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,10,A_o4,Gsh_o4,A_o4,B_o4,C_o5,D_o5,E_o5,E_o5,					//2
	E_o5,10,E_o5,E_o5,E_o5,10,E_o5,E_o5,E_o5,10,E_o5,E_o5,E_o5,10,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,													//3
	E_o5,E_o5,E_o5,E_o5,D_o5,D_o5,E_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,B_o4,B_o4,													//4
	B_o4,C_o5,C_o5,D_o5,D_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,A_o4,A_o4,A_o4,0,A_o4,												//5
	A_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5, C_o5, D_o5,D_o5,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5,C_o5,A_o4,										//6
	A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,E_o4,E_o4,A_o4,A_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5,													//7
	C_o5,D_o5,D_o5,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5,C_o5,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,													//8
	0,A_o4,Gsh_o4,A_o4,B_o4,C_o5,D_o5,E_o5,E_o5,E_o5,E_o5, 0,E_o5,E_o5,E_o5,E_o5,0,E_o5,E_o5,E_o5,E_o5,0,E_o5,			//9
	E_o5,E_o5,E_o5, 0,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5, E_o5, 0,D_o5,D_o5, 0,E_o5,E_o5, 0,F_o5,													//10
	F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,0,B_o4,B_o4,B_o4, B_o4,C_o5, C_o5,D_o5, D_o5,E_o5,E_o5,E_o5,E_o5,E_o5,													//11
	E_o5,E_o5, E_o5,A_o4,A_o4,A_o4, A_o4, 0,A_o4, A_o4,B_o4, B_o4,C_o5,C_o5,C_o5,C_o5,C_o5, C_o5,D_o5,B_o4,B_o4,										//12
	B_o4,B_o4,B_o4, B_o4,0,C_o5, C_o5,0,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4, A_o4,E_o4,E_o4,A_o4, A_o4,													//13
	B_o4, B_o4, 0,C_o5,C_o5,C_o5,C_o5,C_o5, C_o5,D_o5, D_o5,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5, C_o5,A_o4,A_o4,										//14
	A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4, A_o4,0,A_o4,Gsh_o4,A_o4,B_o4, C_o5,D_o5,E_o5,E_o5,E_o5, E_o5,0,E_o5,E_o5,						//15
	E_o5, E_o5,0,E_o5,E_o5,E_o5, E_o5,0,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,E_o5, E_o5,0,D_o5, D_o5,															//16
	E_o5, E_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5,F_o5, F_o5,B_o4,B_o4,B_o4, B_o4,C_o5,C_o5,D_o5,D_o5,E_o5,E_o5,										//17
	E_o5,E_o5,E_o5,E_o5,E_o5,E_o5,A_o4,A_o4,A_o4, A_o4,0,A_o4,A_o4,B_o4,B_o4,C_o5,C_o5,C_o5,C_o5,C_o5,C_o5,													//18
	D_o5,D_o5,B_o4,B_o4,B_o4,B_o4,B_o4,B_o4,C_o5,C_o5,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4,A_o4, 0,													//19
	A_o4,A_o4,Gsh_o4,Gsh_o4,A_o4,A_o4,B_o4,B_o4,E_o4,E_o4,Dsh_o4,Dsh_o4,E_o4,E_o4,B_o4,B_o4,E_o4,E_o4,E_o5,E_o5,		//20
	D_o5, D_o5,0,D_o5,D_o5,C_o5,C_o5,B_o4,B_o4,A_o4, A_o4,0,A_o4,A_o4,B_o4,B_o4,C_o5,C_o5,A_o4,A_o4,B_o4,B_o4,				//21
	G_o4,G_o4,Fsh_o4,Fsh_o4,G_o4,G_o4,B_o4,B_o4,G_o4,G_o4, G_o5, G_o5,0,G_o5,G_o5,F_o5, F_o5,0,F_o5,F_o5,E_o5,E_o5,			//22
	D_o5,D_o5,C_o5, C_o5,0,C_o5,C_o5,D_o5,D_o5,E_o5,E_o5,C_o5,C_o5,Csh_o5,Csh_o5,A_o4,A_o4,Csh_o5,Csh_o5,E_o5,E_o5,		//23
	A_o5,A_o5,A_o5,A_o5,G_o5,G_o5,G_o5,G_o5,F_o5,F_o5,E_o5,E_o5,D_o5,D_o5,Csh_o5,Csh_o5,D_o5,D_o5,A_o4,A_o4,						//24
	Gsh_o4,Gsh_o4,A_o4,A_o4,Dsh_o5,Dsh_o5,B_o4,B_o4,Dsh_o5,Dsh_o5,Fsh_o5,Fsh_o5,C_o6,C_o6,C_o6,C_o6,Dsh_o5,Dsh_o5,B_o5,B_o5,	//25
	A_o5, A_o5,0,A_o5, A_o5,0,D_o5,D_o5,A_o5,A_o5,Gsh_o5,Gsh_o5, A_o5,A_o5,B_o5,B_o5,E_o5, E_o5,0,E_o5,E_o5,E_o5,E_o5,				//26
	E_o5,E_o5, D_o5, C_o5,B_o4,B_o4,C_o5,C_o5,D_o5,D_o5,E_o4,E_o4,C_o5,C_o5,B_o4,B_o4,A_o4,A_o4,Gsh_o4,Gsh_o4,		//27
	A_o4,A_o4,B_o4,B_o4,C_o5,C_o5,A_o4,A_o4,B_o4,B_o4,B_o4,B_o4,B_o4,G_o4,G_o4,B_o4,B_o4,G_o5,G_o5,G_o5,							//28
	G_o5,G_o5,F_o5,F_o5,F_o5,F_o5,E_o5,E_o5,D_o5,E_o5,D_o5,D_o5,C_o5,C_o5,B_o4,B_o4,C_o5,C_o5,D_o5,D_o5,						//29
	C_o5,C_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,Ash_o5,G_o5,Ash_o5,Ash_o5,G_o5,G_o5,Ash_o5,Ash_o5,E_o5,E_o5,A_o5,A_o5,G_o5,G_o5,			//30
	A_o5,A_o5,F_o5,F_o5,A_o5,A_o5,E_o5,E_o5,A_o5,A_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,B_o4,B_o4,Dsh_o5,Dsh_o5,					//31
	C_o6,C_o6,C_o6,C_o6,Dsh_o5,Dsh_o5,Dsh_o5,Dsh_o5,B_o5,B_o5,B_o5,B_o5,D_o4,D_o4,E_o4,E_o4,F_o4,F_o4,G_o4,G_o4								//32
};
static void play_music (unsigned int);
static void tpmSetupAudio(void);
static void initPinsAudio(void);
static void initSongInfo(unsigned int);

//void lcd_data_write(char *data, lcd_line, line, lcd_scrolling, scroll_type);
//static char lcd_scroll_msg[50]; // msg to be scrolled
static uint8_t songPlaying = 0; // are we playing a song
static uint8_t prevSong = 0; // rpeviously no song
static void initPinsAudio()
{
	PORTC->PCR[SOUND_OUT] &= ~PORT_PCR_MUX_MASK; // clear
	PORTC->PCR[SOUND_OUT] |= PORT_PCR_MUX(4); // set to TPM0_CH1
	
	PTC->PDDR |= (1UL << SOUND_OUT);
}

enum songSelect
{
	dance = 0,
	lac,
	amin,
	virus
};

static void play_music (unsigned int song) {
	uint8_t songPicked = 0;
	
	if(prevSong != song)
	{
		switch(song){	
			case 0: //Wait for the first song to initialize
				period = 0;  //Play nothing while we wait
				dutyCycle = 0;
				break;
			case 1:
				songPtr = Hungarian_Dance;
				songLength = sizeof(Hungarian_Dance)/sizeof(unsigned int); // get the number of notes
				break;
			case 2:
				songPtr = Lacrimosa;
				songLength = sizeof(Lacrimosa)/sizeof(unsigned int); // get the number of notes
				break;
			case 3:
				songPtr = Amin;
				songLength = sizeof(Amin)/sizeof(unsigned int); // get the number of notes
				break;
			case 4:
				songPtr = Virus;
				songLength = sizeof(Virus)/sizeof(unsigned int); // get the number of notes
				break;
			}
			initSongInfo(song);
			TPM0->SC |= TPM_SC_CMOD(1); // start the song
			TPM0->CONTROLS[1].CnV= dutyCycle; // set the duty cycle
			TPM0->MOD = period; // set the period
			prevSong = song;
	}
	else
	{
		// playing a song, update the notes
		uint8_t nextNote = *(songPtr + noteIndex);
		
		if(nextNote != 10)
		{
			period = CLOCK_AUDIO/(*(songPtr + noteIndex)*96);      //Period = Clock/Frequency
			dutyCycle = period/2;
			TPM0->CONTROLS[1].CnV= dutyCycle; // set the duty cycle
			TPM0->MOD = period; // set the period
		}
	}
}	

// Removed redundant duplicate code
static void initSongInfo(unsigned int song)
{
		period = CLOCK_AUDIO/(songPtr[0]*96);        //Period = Clock/Frequency
		dutyCycle = period/2; //50% duty cycle
		curSong = song;
		songPlaying = 1;
		noteIndex = 0;
}

static void tpmSetupAudio (void) {

	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

	SIM->SOPT2 &= ~SIM_SOPT2_PLLFLLSEL_MASK;
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL(0);

	//Load the counter and the mod
	// TPM0->MOD = 6;

	//Set TPM Count direction to up with a divide by prescaler
	TPM0->SC &= ~((TPM_SC_CPWMS_MASK) | (TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM0->SC  = TPM_SC_CMOD(1) | TPM_SC_PS(4) | TPM_SC_TOIE_MASK | TPM_SC_CPWMS_MASK;

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

static void stopMusic(uint8_t end)
{
	if(end == 1)
	{
		noteIndex = 0; // reset the note index
	}
	songPlaying = 0;
	TPM0->SC &= TPM_SC_CMOD(0); // stop the tpm. when resumed, 
}

static void resumeMusic()
{
	songPlaying = 1; // song is playing, do not change any current values
	TPM0->SC |= TPM_SC_CMOD(1); // resume the song
}

static void handlerMusicTick()
{
	// move on to the next note in the song
	if(songPlaying == 1)
	{
		noteIndex++;
		if(noteIndex <= songLength)
		{
			play_music(curSong);
		}
		else
		{
			// stop the song
			stopMusic(1); // end the song and reset all the counters
		}
	}
	else
	{
		// do nothing
	}
}
		