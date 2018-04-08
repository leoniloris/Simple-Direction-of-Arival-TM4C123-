#include <stdint.h>
#ifndef adad
#define adad
//#define CH8 (8*4)
//#define CH8ALT (8*4+128)
//#define BIT8 0x00000100
#define CH30 (30*4)
#define BIT30 0x40000000
#define N_samples 256  // Frame size
#define Micro 2 // microphones



uint16_t write_ind;
uint16_t X[Micro*N_samples]; // Buffer
uint16_t X_proc[Micro*N_samples]; //Buffer already initialized due to the heap limitation
float X_float[Micro*N_samples]; //Buffer already initialized due to the heap limitation


uint32_t ucControlTable[256] __attribute__ ((aligned(1024)));
void Timer0A_Handler(void);
void Double_ADC_init(void);
void uDMA_init(void);
void DMA_Transfer(void);
#endif

