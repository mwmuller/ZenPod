#include "MKL25Z4.h"                    // Device header
#include "math.h"

#define ADC_POS (20)

static void Init_ADCTemp (void) 
{
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	//select analog for pin
	PORTE->PCR[ADC_POS] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[ADC_POS] |= PORT_PCR_MUX (0);
	
	//LOW power configuration, Long sample time, 16 bit single-ended conversion
	// Bus clock input
	ADC0->CFG1 = ADC_CFG1_ADLPC_MASK | ADC_CFG1_ADLSMP_MASK | ADC_CFG1_MODE (3) | ADC_CFG1_ADICLK (0) ;
	
	//software trigger, compare function disabled, DMA disabled 
	//voltage references VREFH and VREFL
	
	ADC0->SC2 = ADC_SC2_REFSEL (0) ;

}

static double Measure_Temperature_Heater (void) 
{
	double dblADCRead;
	double dblTemp;
	
	// start conversion on channel 0
	ADC0->SC1[0] = 0x00; 
	
	// Wait for conversion to finish
	while (! (ADC0->SC1[0] & ADC_SC1_COCO_MASK) )
		;
	
	// Read result, convert to floating-point
	dblADCRead = (double) ADC0->R[0];
	
	//Calculate temperature (Celsius) using polynomial equation
	// Assumes ADC is in 16-bit mode, has VRef = 3.3 V
	
	dblTemp = -36.9861 + dblADCRead * (0.0155762 + dblADCRead * (-0.00000143216 + dblADCRead * (0.0000000000718641 + dblADCRead * (-0.00000000000000184630 + dblADCRead * (0.0000000000000000000232656 + dblADCRead
	* (-0.000000000000000000000000113090)) )) ));
	


	return dblTemp;
}
