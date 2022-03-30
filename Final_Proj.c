#include <MKL25Z4.h> 
#include "MKL25Z4.h"                    // Device header
#include "system_MKL25Z4.h"             // Keil::Device:Startup
#include "RTE_Components.h"             // Component selection
#include "pitches.h"
#define I2C_M_START  I2C0->C1 |= I2C_C1_MST_MASK
#define I2C_M_STOP  I2C0->C1 &= ~I2C_C1_MST_MASK
#define I2C_M_RSTART  I2C0->C1 |= I2C_C1_RSTA_MASK

#define I2C_TRAN  I2C0->C1 |= I2C_C1_TX_MASK
#define I2C_REC   I2C0->C1 &= ~I2C_C1_TX_MASK

#define I2C_WAIT   while((I2C0->S & I2C_S_IICIF_MASK)==0) {} \
													I2C0->S |= I2C_S_IICIF_MASK;
#define NACK       I2C0->C1 |= I2C_C1_TXAK_MASK
#define ACK        I2C0->C1 &= ~I2C_C1_TXAK_MASK

#define MAX_DAC_CODE (4095)
#define NUM (512) 

#define SD_CS (4)
#define SD_SCK (2)
#define SD_MISO (1)
#define SD_MOSI (3)	
/* static int arr[] = {
NOTE_C4, NOTE_G3,NOTE_G3, NOTE_GS3, NOTE_G3,0, NOTE_B3, NOTE_C4}; */

//Global varaibles
static 	uint16_t Reload_DMA_Source;
static uint32_t Reload_DMA_Byte;
	
uint16_t TriangleTable[NUM];
void init_Triangle(void);
void Init_DMA(uint16_t *source, uint32_t count);
void i2c_init (void);
void start_DMA (void);
void init_SPI (void);
int i2c_read(uint8_t dev, uint8_t reg, uint8_t *arr, int8_t count);

int main (void) {
	uint8_t dev; uint8_t reg; uint8_t arr[] = {};
		int8_t count1 = 0;
		uint32_t count2 = 0;
	//Make the connection with the SD module
	init_SPI ();
	//Read data from SD module
  i2c_init ();
	i2c_read(dev,  reg,  *arr, count1);
	//Store value in DMA
	Init_DMA(*arr, count2);
	init_Triangle();
	//Convert .wav to DAC
		start_DMA ();
}
	
void i2c_init (void) {

	//clock i2c peripheral and port E
			SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;
			SIM ->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	//set pins to I2C function
	//baud = bus freq/(scl_div+mul)
	//24MHz/400kHz = 60; icr=0x11 sets scl_div to 56
	I2C0->F = I2C_F_ICR(0x11) | I2C_F_MULT(0);
	
	//enable I2C and set to master mode
	I2C0->C1 |= (I2C_C1_IICEN_MASK);
	
	//select high drive mode
	I2C0->C2 |= (I2C_C2_HDRS_MASK);
}

int i2c_read(uint8_t dev, uint8_t reg, uint8_t *arr, int8_t count) {

uint8_t data;
	int8_t num = 0;

	
	I2C_TRAN;      /*set to transmit mode */
	I2C_M_START;
	I2C0->D = dev;
	I2C_WAIT
	
	I2C0->D = reg;
	I2C_WAIT
	
	I2C_M_RSTART;
	I2C0->D = (dev|0x1);
	I2C_WAIT
	
	I2C_REC;
	ACK;
	
	data = I2C0->D;
	I2C_WAIT
		do{
			ACK;
			arr[num++] = I2C0->D;
			I2C_WAIT
			} while (num < count - 2);
	
	NACK;
	arr[num++] = I2C0->D;
	I2C_WAIT
	I2C_M_STOP;
	data = I2C0->D;
	
	return 1;

}

void init_Triangle(void) {
unsigned n, sample;
	
	for (n=0; n<NUM/2; n++) {
		sample = (n*(MAX_DAC_CODE+1)/(NUM/2));
		TriangleTable[n] = sample;  //Fill in form front
	  TriangleTable[NUM-1-n] = sample;
		
	}
}
void Init_DMA(uint16_t *source, uint32_t count) {
	Reload_DMA_Source = source;
	Reload_DMA_Byte = count*2;
	
	//Gate Clocks to DMA and DMAMUX
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
	
	//Disable DMA channel to allow configuration
	DMAMUX0->CHCFG[0] = 0;
	//Generate DMA interrupt when done
	//Increment source, transfer words (16 bits)
	//Enable peripheral request
	
	DMA0->DMA[0].DCR = DMA_DCR_EINT_MASK | DMA_DCR_SINC_MASK | DMA_DCR_SSIZE(2) | DMA_DCR_DSIZE(2) | DMA_DCR_ERQ_MASK | DMA_DCR_CS_MASK;
	DMA_DCR_SSIZE(2) | DMA_DCR_DSIZE(2) | DMA_DCR_ERQ_MASK | DMA_DCR_CS_MASK;
	
	//Configure NVIC for DMA ISR
	NVIC_SetPriority(DMA0_IRQn, 2);
	NVIC_ClearPendingIRQ(DMA0_IRQn);
	NVIC_EnableIRQ(DMA0_IRQn);
}
void start_DMA (void) {
//Initialize source and destination pointers
	DMA0->DMA[0].SAR = DMA_SAR_SAR((uint32_t) Reload_DMA_Source);
	DMA0->DMA[0].DAR = DMA_DAR_DAR((uint32_t) (&(DAC0->DAT[0])));
	
	//byte count
	DMA0->DMA[0].DSR_BCR = DMA_DSR_BCR_BCR(Reload_DMA_Byte);
	
	//clear done flag
	DMA0->DMA[0].DSR_BCR &= DMA_DSR_BCR_DONE_MASK;
	
	//set enable flag
	DMAMUX0->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK;
}
void init_SPI(void) {
	//enable clock
	SIM->SCGC4 |= SIM_SCGC4_SPI1_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	//set PTE2 as SPI1_SCK --- ALT2
	PORTE->PCR[SD_SCK] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SD_SCK] |= PORT_PCR_MUX(2);
	
	//Set PTE3 as SPI1_MOSI -- ALT5
	PORTE->PCR[SD_MOSI] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SD_MOSI] |= PORT_PCR_MUX(5);
  
	//Set PTE1 as SPI1_MISO -- ALT5
	PORTE->PCR[SD_MISO] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SD_MISO] |= PORT_PCR_MUX(5);
	
	//Set PTE4 as SPI1_PCS0 -- ALT2
	PORTE->PCR[SD_CS] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[SD_CS] |= PORT_PCR_MUX(5);
	
	//Select master mode, enable SS output
	SPI1->C1 = SPI_C1_MSTR_MASK | SPI_C1_SSOE_MASK;
	SPI1->C2 = SPI_C2_MODFEN_MASK;
	
	//Select active high clock, first edge sample
	SPI1->C1 &= ~SPI_C1_CPHA_MASK;
	SPI1->C1 &= ~SPI_C1_CPOL_MASK;
	
	//BaudRate = BusClock / ((SPPR+1)*2^(SPR+1))
	SPI1->BR = SPI_BR_SPPR(2) | SPI_BR_SPPR(1);
	
	//enable SPI1
	SPI1 ->C1 |= SPI_C1_SPE_MASK;
}
