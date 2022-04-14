#include "MKL25Z4.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "system_MKL25Z4.h"             // Keil::Device:Startup
#include "math.h"
#include "tempFiles.c"
#include "audio.c"

#define LEDHeater (20)			//Simulating Heater as LED on E PIN 20, This requires TPM0 and Channel 0 "Pavan"
#define PWM_PERIOD (24000) // Period to use for calculations
#define minDutyHeater (0.80F) // % value as default
#define heaterScalar (5)

// PORT C Define breathing
#define SERVO_SHIFT (0) // PORTC relay connected to Servo.
#define LED_MED (3) // PORTC LED is meditation is still active
#define vibrationScalar (10) // scales up the timings

//define 4 LEDS on PORT A
#define LED1 (5) //PTA5
#define LED2 (12) //PTA12
#define LED3 (2) //PTA2
#define LED4 (1) //PTA1

// number of LEDS for med time
#define MED_LED_NUM (4) 
//#define 4 pin Switches on PORT D
#define SW_SWITCH (4) // starts the meditation
#define PLUS_SWITCH (0)
#define MINUS_SWITCH (5)

// PORTE pin 1 is Rx from ESP
#define UART1_RX_ESP (1)
#define breathShift (4)

// Delays for state machine
#define HOLD_DELAY (10) // delay between all states
#define STATE_DELAY (10)

// State machine masks
#define inhaleMask (0x00F0) // grab inhale time
#define exhaleMask (0x000F) // grab exhale time

// Misc timers
#define MED_TIME_DEFAULT (600) // default number of milliseconds
#define MED_TIME_SCALAR (10) // Used to scale up from 5 bits
#define MILLISECONDSCALAR (500) // scales the time into milliseconds. Calibrated taking into account delays


#define MASK(x) (1ul << x)

// Prototypes
void initPins(void);
void initSysTick(void);
void breathStateMachine(void);
void handleLedTimes(uint8_t); // timer, forced plus? forced minus?
void handleMeditationStatus(void);
void setMedOnOffSettings(uint8_t);
void setLedMask(uint8_t);
void handleSwitches(void);
void initHeater (uint16_t);	//"Pavan"
void setHeaterPWMDutyCycle();	//"Pavan"
void handleUartEsp(void);
void initUart1(void);
void initHandler(void); // handles all init calls
// Arrays for init
static uint8_t portCGpio[2] = {SERVO_SHIFT, LED_MED};
static uint8_t portAGpio[4] = {LED1, LED2, LED3, LED4}; // LED1 indicates lowest time remaining
static uint8_t portDGpio[3] = {MINUS_SWITCH, PLUS_SWITCH, SW_SWITCH};

// Global values for breathing
static uint8_t breathTime = 0; // timer for inhaling and exhaling
static uint8_t delayTime = 0; // value for delay between states
static uint8_t inMeditation = 0;
static uint8_t inhaleExhaleSettings[3] = {0x22, 0x32, 0x43}; // Inhale/Exhale timer
static uint8_t breathingSetting = 0; // default
static uint8_t prev_medState = 0; // Have we transistioned states?
static uint8_t InhaleExhaleSetting = 0;	// setting for inhale/exhale  "Pavan"
static uint8_t prev_heaterSetting = 5; // a value that is impossible to set later

// Globals for Meditation Time LEDS.
static uint32_t medTimeCurrent = MED_TIME_DEFAULT; // 20 minute default
static uint32_t medTimeMax = MED_TIME_DEFAULT; // max time 
static uint8_t swDebounce = 0; // if 1, the switch will not be read.
// enums
// State of breathing
enum breathState
{
	noBreath =0,
	inhaling,
	holding,
	exhaling
};	

// Enum state globals
static uint8_t currentBreathState = noBreath; // init the breath state
static uint8_t prevBreathState = noBreath; // Used to determine if we've changed states

// Time remaining in meditation cycle
enum medTimeState
{
	off = 0,
	percent25,
	percent50,
	percent75,
	percent100
};

enum medTransitionType
{
	_timer,
	_plus,
	_minus
};

// Enum state globals
static uint8_t currentMedState = percent100; // init the breath state
static uint8_t prevMedLedState = off; // Used to determine if we've changed states

static uint8_t medLedMask = 0; // contains 4 bits that will determine the LEDs states

// ****** MAIN **********
int main()
{
	initHandler(); // call system init handler
	while(1)
	{
		if(swDebounce == 0)
		{
			handleSwitches(); // check switches
		}
		handleMeditationStatus();
	}
}

// used to init the system.
void initHandler(){
	initPins(); // init speaker port
	Init_ADCTemp(); // inits the temp for heater checking
	initSysTick(); // init pit timer
	initUart1(); // init UART pins, clock, and UART communication for Rx
	initHeater(PWM_PERIOD);	//Initliaze the heater "Pavan"
	setHeaterPWMDutyCycle();	//"Pavan"
	tpmSetupAudio();
	initPinsAudio();
}

/// **********INIT FUNCTION**************
// Inits the pins for the board
void initPins()
{
	// init the clocks for pins
	SIM->SCGC5 |= (SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK);
	
	// configure MED LED and Servo Motor
	for(int i = 0; i < sizeof(portCGpio)/(sizeof(uint8_t)); i++)
	{
		PORTC->PCR[portCGpio[i]] |= PORT_PCR_MUX(1);
		PTC->PDDR |= MASK(portCGpio[i]);
		PTC->PCOR |= MASK(portCGpio[i]);
	}
	// Configuring buttons
	for(int i = 0; i < sizeof(portDGpio)/(sizeof(uint8_t)); i++)
	{
		PORTD->PCR[portDGpio[i]] &= ~(PORT_PCR_MUX_MASK | PORT_PCR_IRQC_MASK);
		PORTD->PCR[portDGpio[i]] |= (PORT_PCR_MUX(1) | PORT_PCR_IRQC(10) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK); // gpio, check falling edge, pullup resistor set
		PTD->PDDR &= (uint8_t)~MASK(portDGpio[i]); // set pin direction as input
	}

	// configuring PORT A GPIO LEDS
	for(int i = 0; i < sizeof(portAGpio)/(sizeof(uint8_t)); i++)
	{
		PORTA->PCR[portAGpio[i]] |= PORT_PCR_MUX(1);
		PTA->PDDR |= MASK(portAGpio[i]);
		PTA->PCOR |= MASK(portAGpio[i]);
	}
}

void initUart1()
{
	// configure UART1 port
	PORTE->PCR[UART1_RX_ESP] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART1_RX_ESP] |= PORT_PCR_MUX(3);
	SIM->SCGC4 |= SIM_SCGC4_UART1_MASK; // set clock for scgc4 uart1
	SIM->SOPT5 |= SIM_SOPT5_UART1RXSRC(0); // Set the src for rx
	
	UART1->C4 |= UARTLP_C4_OSR(16);
	// Init UART settings
	
	// Baud rate --> the baud rate equals baud clock / ((OSR+1) × BR).
	// 9600 = 48Mhz / ((15 + 1) * 9600) => 48,000,000/(16 + 1 * 9600) => 0x138
	UART1->BDL = 0x45;
	UART1->BDH = 0x01;

	// Data communication config
	UART1->C1 = 0x00;
	UART1->C2 |= (UART_C2_TIE(0) | UART_C2_TCIE(0) | UART_C2_RIE(1) | UART_C2_RE(1) | UART_C2_TE(0));

	NVIC_SetPriority(UART1_IRQn, 3);
	NVIC_ClearPendingIRQ(UART1_IRQn);
	NVIC_EnableIRQ(UART1_IRQn);
}

// Inits the Systick to keep track of internal clock (1 second interrupts)
void initSysTick()
{
	SysTick->LOAD = (240000000L/100); // 100ms delays
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk; // Enable interrupts and timer
	
	NVIC_SetPriority(SysTick_IRQn, 2);
	NVIC_ClearPendingIRQ(SysTick_IRQn);
	NVIC_EnableIRQ(SysTick_IRQn);
}

//Function is added by Pavan
void initHeater(uint16_t period)
{
	// enable the clock for PORTD
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;  									
	
	// Init the Heater PWM (Here LED is simulated as Heater)
	PORTE->PCR[LEDHeater] &= ~PORT_PCR_MUX_MASK;
	// Select the PIN for ALT4/TPM functionality
	PORTE->PCR[LEDHeater] |= PORT_PCR_MUX(3);
	// Selecting the LED pins data direction registers is not advised here. 
	// The PWM functionality does not run in thst case
	
	//enable the clock for TPM
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
	//Configure TPM
	
	//Set clock source for tpm : 48 MHz
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	// Selecting the clock from the board, internal clock is set to use
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC (1);

	SIM->SOPT2 &= ~SIM_SOPT2_PLLFLLSEL_MASK;
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL(0);
	
	//Load the counter and MOD, this is active low PWM signal
	TPM1->MOD = period-1;
	
	//Set TPM count direction to up with a divide by 2 prescaler
	TPM1->SC &= ~((TPM_SC_CPWMS_MASK) | (TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM1->SC = TPM_SC_CMOD(0) | TPM_SC_PS(2) | TPM_SC_TOIE_MASK | TPM_SC_CPWMS_MASK;
	
	//Continue operation in debug mode
	TPM1->CONF  |= TPM_CONF_DBGMODE(3);
	
	// Configured as Edge Aligned PWM, 	
	//Set channel TPM0 Channel 2 to edge-aligned low true PWM
	TPM1_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK));
	TPM1_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_ELSA(0) |  TPM_CnSC_MSB(1)  | TPM_CnSC_MSA(0));
	
}	// End of Init Heater function
/// ****************INIT FUNC END**************

// Handles meditation state

void handleMeditationStatus()
{
	if(inMeditation != prev_medState)
	{
		setMedOnOffSettings(inMeditation); 
	}
		prev_medState = inMeditation;
}

void handleSwitches()
{
	// Handles PLUs/MINUS switches
	if(PORTD->ISFR & MASK(PLUS_SWITCH))
	{
		handleLedTimes(_plus);
		PORTD->ISFR &= 0xffffffff; // clear button flag
		swDebounce = 1;
	}
	else if(PORTD->ISFR & MASK(MINUS_SWITCH))
	{
		handleLedTimes(_minus);
		PORTD->ISFR &= 0xffffffff; // clear button flag
		swDebounce = 1;
	}
	else
	{
		// none
	}
	
	// Handles Meditation On/Off switch
	if(PORTD->ISFR & MASK(SW_SWITCH))
	{
		// Toggle meditation
		if(inMeditation)
		{
			inMeditation = 0;
			setMedOnOffSettings(inMeditation);
		}
		else
		{
			inMeditation = 1;
			setMedOnOffSettings(inMeditation);
		}
		swDebounce = 1;
		PORTD->ISFR &= 0xffffffff; // clear button flag
	}
}
// Sets the settings based on meditation on or off

void setMedOnOffSettings(uint8_t state)
{
	if(state == 0)
	{
		breathTime = 0;
		delayTime = 0;
		TPM1->SC  &= (TPM_SC_CMOD(0) & 0xffffffff);
		PTC->PCOR |= (MASK(SERVO_SHIFT) | MASK(LED_MED));
	}
	else
	{
		handleLedTimes(_timer);
		TPM1->SC  |= TPM_SC_CMOD(1);
		PTC->PSOR |= MASK(LED_MED);
	}
}

// determines the states for the breathing statemachine

void breathStateMachine()
{
	switch(currentBreathState)
	{
		case noBreath:
			delayTime++;
			if(delayTime >= STATE_DELAY)
			{
				delayTime = 0;
				currentBreathState = inhaling;
			}
			break;
		case inhaling:
			breathTime++; // increment the seconds
			if(breathTime >= ((inhaleMask & inhaleExhaleSettings[breathingSetting]) >> 4UL) * vibrationScalar)
			{
				currentBreathState = holding;
				breathTime = 0;
			}
			break;
		case holding:
			delayTime++;
			if(delayTime >= HOLD_DELAY)
			{
				delayTime = 0;
				currentBreathState = exhaling;
			}
			break;
		case exhaling:
			breathTime++; // increment the seconds
			if(breathTime >= (exhaleMask & inhaleExhaleSettings[breathingSetting]) * vibrationScalar)
			{
				currentBreathState = noBreath;
				breathTime = 0;
			}
			break;
		default:
			currentBreathState = noBreath;
			delayTime = 0;
			break;
	}
	if((prevBreathState != currentBreathState))
	{
		if((currentBreathState == inhaling || currentBreathState == exhaling))// do we turn on the servo motor? If we are inhaling or exhaling we do
		{
			PTC->PSOR |= MASK(SERVO_SHIFT); // toggle servo motor
		}
		else
		{
			PTC->PCOR |= MASK(SERVO_SHIFT); // when not inhaling or exhaling
		}
	}
	else
	{
		// continue with current outputs
	}
	prevBreathState = currentBreathState;
}


// Contains the logic for Meditation timed leds.

void handleLedTimes(uint8_t transitionType)
{
	// As per default, each LED will represent 100/numLeds percent
	// of total meditation time. 
	// OR 
	// MeditationTime(minutes)/numLEDS (TBD). This is be 20 minutes by default
	// Max time is 3 hours. 
	
	float currentMedRatio = 0;
	
	switch(transitionType)
	{
		case _timer:
			// do not change time calculation
			currentMedRatio = (float)medTimeCurrent / (float)medTimeMax;
			currentMedState = ceil(currentMedRatio *  MED_LED_NUM);
		break;
		case _plus:
			// increment only if at 3 or less
			if(currentMedState < 4)
			{
				currentMedState++;
				medTimeCurrent = medTimeMax * (float)((float)currentMedState/(float)MED_LED_NUM); // set the current time
			}
			// do plus
		break;
		case _minus:
			// decrement only if at greater than 0
			if(currentMedState > 0)
			{
				currentMedState--;
				medTimeCurrent = medTimeMax * (float)((float)currentMedState/(float)MED_LED_NUM);
			}
			// do minus
		break;
		default:
			// do timer
			currentMedRatio = (float)medTimeCurrent / (float)medTimeMax;
			currentMedState = ceil(currentMedRatio *  MED_LED_NUM);
		break;
	}
	
	// LEVELS
	/*
	0.01 -> 0.25 1 LED
	0.26 -> 0.50 2 LED
	0.51 -> 0.75 3 LED
	0.76 -> 1.00 4 LED	
	*/
	// We will know the new current state prior to switch statement
	if(prevMedLedState != currentMedState)
	{
		// Set the appropriate LED bit mask
		switch(currentMedState)
		{
			case off:
				medLedMask = 0; // _ _ _ _
			break;
			case percent25:
				medLedMask = 0x8; // * _ _ _
			break;
			case percent50:
				medLedMask = 0xC; // * * _ _
			break;
			case percent75:
				medLedMask = 0xE; // * * * _ 
			break;
			case percent100:
				medLedMask = 0xF; // * * * * 
			break;
			default:
				medLedMask = 0xF; // * * * *
				// full
			break;
		}
		// Set the LEDs based on the mask
		setLedMask(medLedMask);
		
		// We are out of time. Stop the clock
		if(currentMedState == 0)
		{
			inMeditation = 0;
			medTimeCurrent = medTimeMax; // reset to max
		}
		// update the LEDS
	}
	else
	{
		// Do nothing, LEDs are in correct state
	}
	prevMedLedState = currentMedState; // Save the previous state.
}


// Sets the LEDS to the proper state

void setLedMask(uint8_t mask)
{
	for(int i = 0; i < MED_LED_NUM; i++)
	{
		if(1 & mask)
		{			
			PTA->PSOR |= MASK(portAGpio[(MED_LED_NUM - 1) - i]); // if I = 0, select 3
		}
		else
		{
			PTA->PCOR |= MASK(portAGpio[(MED_LED_NUM - 1) - i]);
		}
		mask = mask >> 1UL; // shift the mask
	}
}

//Function is added by Pavan
void setHeaterPWMDutyCycle (void) 
{

	//detrmine the inhale exhale setting
	//Inhale timinmg could independently determine the setting uniquely 
	uint32_t HeaterPWM = (PWM_PERIOD * minDutyHeater);
	
	if(breathingSetting != prev_heaterSetting)
	{
		switch(breathingSetting)
			{
			case 0: 		//80% PWM
				HeaterPWM = (PWM_PERIOD * minDutyHeater);
			break;
			case 1: 		//85% PWM
				HeaterPWM = (PWM_PERIOD * (minDutyHeater + (((breathingSetting - 1) * heaterScalar) / 100)));
			break;
			case 2: 		//90% PWM
				HeaterPWM = (PWM_PERIOD * (minDutyHeater + (((breathingSetting - 1) * heaterScalar) / 100)));
			break;
			default: 	//Unknown state, use no heat
				HeaterPWM = 0;
			break;
			} //End of the switch case
			TPM1->CONTROLS[0].CnV=HeaterPWM;
	}
	else
	{
		// warnings. 
	}
		prev_heaterSetting = breathingSetting;
} // End of setHeaterPWMDutyCycle function

///*************IRQ HANDLERS****************
// Handles the timer

void SysTick_Handler()
{
	// if we are in an intermmediat state, we delay breathTime
	if(inMeditation)
	{
		medTimeCurrent--;
		breathStateMachine();
		handleLedTimes(_timer);
		Measure_Temperature_Heater();
	}
	handlerMusicTick();
	if(swDebounce >= 2)
	{
		swDebounce = 0; // Allow buttons to be pressed again
	}
	else
	{
		swDebounce++;
	}
}

// Struct for handling Zendata. Readability
struct ZenEspData
{
	uint8_t sysOnOff : 1;
	uint8_t breathSetting : 2;
	uint8_t medTime : 5;
};

// No hardcoded masks
enum espDataMasks
{
	sysOnOffMask = 0x01,
	breathSetMask = 0x30,
	medTimeMask = 0x0F,
};

// On_Off[7:6], breath setting[6:4], meditation total time [4:0]
// On_Off is one bit
// Breath setting is 1-3
// Meditation time is min 20 - max 180, 5 min increments. 20 + (2^5 * 5)

enum espSettingType
{
	onOffMed = 0,
	medTimeBreathing,
	musicSelection,
	onOffMusic
};

// Handles incoming UART communication
void UART1_IRQHandler()
{
	static uint8_t firstByte = 0; // has recieved first byte
	static uint8_t espSettingsUpdate = 0; // A setting to be updated
	// get data
	struct ZenEspData espData;
	uint8_t uart1Data = 0;
	while(!(UART1->S1 & UART_S1_RDRF_MASK))
	{
		// Wait for data
	}
	
	//uart1Data = UART1_D;
	uart1Data = UART1->D;
	firstByte = ~firstByte; // Set first byte received
	if(firstByte & 1)
	{
		espSettingsUpdate = uart1Data;
	}
	else
	{
		switch(espSettingsUpdate)
		{
			case onOffMed:
				inMeditation = uart1Data & 0x01;
				setMedOnOffSettings(inMeditation); // send the first bit
			break;
			
			case medTimeBreathing:
				espData.breathSetting = (uart1Data & breathSetMask) >> breathShift; // get the 6,5 bit
				espData.medTime = (uart1Data & medTimeMask); // get the med time data
				
				// Set the settings
				breathingSetting = espData.breathSetting;
				setHeaterPWMDutyCycle(); // update the heater cycle
				// (Time * 10 min scalar + 20 min) * 60 to convert to seconds 
				medTimeMax = (espData.medTime * MED_TIME_SCALAR) * MILLISECONDSCALAR; // Always 20 minute minimum.
				medTimeCurrent = medTimeMax;
				currentMedState = MED_LED_NUM; // LEDS determine the med state for leds
			// Set prev_data to new data for comparison
			break;
			
			case musicSelection:
				// get the song selection and play the song
				prevSong = 0; // Replay the same song
				
				play_music(uart1Data); // shift bit due to ESP
			break;
			
			case onOffMusic:
				if(uart1Data & 1) // We want to resume playing
				{
					resumeMusic(); // resume the song
				}
				else if(uart1Data == 0)
				{
					// stop the song do not reset the index
					stopMusic(0); 
				}
			default:
				// do not update info
				firstByte = 0; // byte from data was not useable
			break;
		}
	}
}

