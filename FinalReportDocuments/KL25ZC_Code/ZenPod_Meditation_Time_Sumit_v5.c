#include <stdio.h>
#include <MKL25Z4.h>
#include "MKL25Z4.h" 

// Device header
//#define 4 LEDS on PORT A
#define LED1 (1) //PTA1
#define LED2 (2) //PTA2
#define LED3 (4) //PTA4
#define LED4 (5) //PTA5

//#define 4 pin Switches on PORT D
#define SW1 (4) //PTD4
#define SW2 (5) //PTD4
#define MASK(x) (1UL << (x))



int main () { //Main function starts here
	//Enable clock to  PORT A and PORT D
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	
	//Make GPIO pins for ZenPod timer
	PORTA->PCR[LED1]&= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[LED1] |= PORT_PCR_MUX(1);
	PORTA->PCR[LED2]&= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[LED2] |= PORT_PCR_MUX(1);
	PORTA->PCR[LED3]&= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[LED3] |= PORT_PCR_MUX(1);
	PORTA->PCR[LED4]&= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[LED4] |= PORT_PCR_MUX(1);
		
	//Set ports to Inputs and Outputs
	PTA->PDDR |= MASK(LED1)|MASK(LED2)|MASK(LED3)|MASK(LED4);
	PTD->PDDR &= ~MASK(SW1);
	PTD->PDDR &= ~MASK(SW2);
	
	// Configure port peripheral. Select GPIO and Interrupts on raising edge
	//PORTE->PCR[SW2] = PORT_PCR_MUX(1)|PORT_PCR_IRQC(9)|PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	PORTD->PCR[SW1] = PORT_PCR_MUX(1)|PORT_PCR_IRQC(9);
	PORTD->PCR[SW2] = PORT_PCR_MUX(1)|PORT_PCR_IRQC(9);

	//Clear all the fields of LED's
	PTA->PCOR = MASK(LED1)|MASK(LED2)|MASK(LED3)|MASK(LED4);
	
	while(1)
	{
		//assigning count to d1 variable based on switch press
		if (PORTD->ISFR & MASK(SW1))
		{
			PORTD->ISFR = 0xffffffff;
		}
		else if (PORTD->ISFR & MASK(SW2))
		{	
			PORTD->ISFR = 0xffffffff;
		}
		else
		{
			// do nothing
		}		 
		switch(Final)
        {   
					  //FSM based on switch pressed, this shows time oncrement or decrement 
            case 0:
							  //Time = 0mins
						    PTA->PCOR = MASK(LED1)| MASK(LED2)|MASK(LED3)|MASK(LED4);
                break;
			      case 1:
							  //Time = 5mins
			          PTA->PSOR = MASK(LED1);
						    PTA->PCOR = MASK(LED2)|MASK(LED3)|MASK(LED4);
                break;
            case 2: 
							  //Time = 10mins
			          PTA->PSOR = MASK(LED1)|MASK(LED2);
						    PTA->PCOR = MASK(LED3)|MASK(LED4);
                break;
						case 3: 
							  //Time = 15mins
			          PTA->PSOR = MASK(LED1)|MASK(LED2)|MASK(LED3);
						    PTA->PCOR = MASK(LED4);
                break;
						case 4: 
							  //Time = 10mins
						    PTA->PSOR = MASK(LED1)|MASK(LED2)|MASK(LED3)|MASK(LED4);
						    break;

        }
	}
}
