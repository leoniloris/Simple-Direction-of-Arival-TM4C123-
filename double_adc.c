#include <Nokia5110.h>
#include "double_adc.h"
#include "TM4C123GH6PM.h"
#include <stdint.h>
#include <stdlib.h>
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))  // IRQ 0 to 31 Set Enable Register
#define NVIC_PRI4_R             (*((volatile unsigned long *)0xE000E410))  // IRQ 16 to 19 Priority Register

void Timer0A_Handler(void){
    TIMER0->ICR |= (1<<4);// acknowledge timer0A match interrupt
    X[ write_ind] = ADC0->SSFIFO3; // fill the buffers
    X[ N_samples+(write_ind++)] = ADC1->SSFIFO3;
    write_ind=write_ind&(N_samples-1);
    //GPIOF->DATA ^=(1<<1);//make output
}
void Double_ADC_init(void){
    // Enable clock on two adc's
    SYSCTL->RCGCADC = (1<<0)|(1<<1);
    // use port           E and F (led)
    SYSCTL->RCGCGPIO = (1<<4)|(1<<5);
    GPIOE->DIR &= ~((1<<1)|(1<<2));
    GPIOE->AFSEL=(1<<1)|(1<<2); // use PE1 and PE2 for the analog pins
    GPIOE->DEN&=~((1<<1)|(1<<2)); // disable digital functionalities
    GPIOE->AMSEL=(1<<1)|(1<<2); // use PE1 and PE2
    /////////////////// TIMER LOL
    SYSCTL->RCGCTIMER |= (1<<0); // activate timer0
    uint32_t delay = SYSCTL->RCGCTIMER;   // allow time to finish activating
    TIMER0->CTL &= ~(1<<0);;    // disable timer0A during setup
    TIMER0->CTL |= (1<<5);   // enable timer0A trigger to ADC
    TIMER0->CFG = 0;             // configure for 32-bit timer mode
    TIMER0->TAMR |= 0x2<<0;   // configure for periodic mode
    TIMER0->TAMR &= ~(1<<4);   // default down-count settings
    TIMER0->TAMR |= 1<<5;   // generate interrupt in match mode
    TIMER0->TAMATCHR = 1000; // generate interrupt when it reaches less than a half its counter
    TIMER0->TAILR = 10000-1;    // start value for trigger
    TIMER0->IMR = 0x00000000;TIMER0->IMR |= 1<<4; // enable match interrupt

    // configure sequencer for both ADC's
    ADC1->ACTSS &= ~(1<<3);//first, disable it
    ADC0->ACTSS &= ~(1<<3);
    //ADC0->PC = 7;         // configure for 1Msamples/sec (we don't actually need it, the reset value is 0x7)
    ADC0->PC = 0x1;         // configure for 125Ksamples/sec
    ADC1->PC = 0x1;         // configure for 125Ksamples/sec
    ADC1->SSPRI = (0<<12)|(1<<8)|(2<<4)|(3<<0);    // sequencer 0 is LOWEST!!, sequencer 3 is HIGHEST!!!
    ADC0->SSPRI = 0x00003210;                      // sequencer 0 is highest, sequencer 3 is lowest

    ADC1->EMUX = (0x5<<12);// Timer triggered
    ADC0->EMUX = (0x5<<12);

    ADC1->SSMUX3 = 2;// sample pin PE1
    ADC0->SSMUX3 = 1;// sample pin PE2

    ADC1->SSCTL3 = 0x6;//set bit END0(bit 1), and bit IE0(bit 2) to be used for interrupts
    ADC0->SSCTL3 = 0x6;

    ADC1->IM &= ~(1<<3);// mask interrupt for SS3 (interrupt comes from timer!)
    ADC0->IM &= ~(1<<3);

    ADC1->ACTSS |=(1<<3); // enable the sequencer
    ADC0->ACTSS |=(1<<3);

    ADC1->ISC =(1<<3); // clear interrupt flags
    ADC0->ISC =(1<<3);
    // enable timers and interrupts
    TIMER0->CTL |= 1<<0;   // enable timer0A 32bits, periodic, no interrupts
    NVIC->ISER[0] |= (1<<19); // enable timer0A interrupt
    //NVIC_EnableIRQ(ADC1SS3_IRQn); // enable interrupt for handler functions
    // CONFIG led
    GPIOF->DEN = 0xff;//enable
    GPIOF->AFSEL = 0x00;
    GPIOF->DIR =  0xff;//direction
    GPIOF->DATA =(1<<1);//make output
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
}
void uDMA_init(void){
    int i;
    for(i=0; i<256; i++){
        ucControlTable[i] = 0;
      }
      SYSCTL->RCGCDMA = 0x01;    // ?DMA Module Run Mode Clock Gating Control
      int delay = SYSCTL->RCGCDMA;   // allow time to finish
      UDMA->CFG = 0x01;          // MASTEN Controller Master Enable
      UDMA->CTLBASE = (unsigned long)ucControlTable;
      UDMA->PRIOCLR = BIT30;     // default, not high priority
      UDMA->ALTCLR = BIT30;      // use primary control
      UDMA->USEBURSTCLR = BIT30; // responds to both burst and single requests
      UDMA->REQMASKCLR = BIT30;  // allow the ?DMA controller to recognize requests for this channel

      uint16_t *SourceEndPt = &X[0]+Micro*N_samples-1;
      uint16_t *DestinationEndPt = &X_proc[0]+Micro*N_samples-1;
      ucControlTable[CH30]   = (uint32_t)SourceEndPt;       // last address
      ucControlTable[CH30+1] = (uint32_t)DestinationEndPt;  // last address
      ucControlTable[CH30+2] = 0x55000002+((Micro*N_samples-1)<<4);             // DMA Channel Control Word (DMACHCTL)
      /* DMACHCTL          Bits    Value Description
         DSTINC            31:30   01    16-bit destination address increment
         DSTSIZE           29:28   01    16-bit destination data size
         SRCINC            27:26   01    16-bit source address increment
         SRCSIZE           25:24   01    16-bit source data size
         reserved          23:18   0     Reserved
         ARBSIZE           17:14   0000  Auto mode does not arbitrate
         XFERSIZE          13:4  1024-1 Transfer count items
         NXTUSEBURST       3       0     N/A for this transfer type
         XFERMODE          2:0     010   Use Auto-request transfer mode
          0b 01 01 01 01 000000 0000 1111111111 0 010
          0b 01010101000000000000000000000010
        */
}
void DMA_Transfer(void){
  ucControlTable[CH30+2] = 0x55000002+((Micro*N_samples-1)<<4);             // DMA Channel Control Word (DMACHCTL)
  /* DMACHCTL          Bits    Value Description
     DSTINC            31:30   01    16-bit destination address increment
     DSTSIZE           29:28   01    16-bit destination data size
     SRCINC            27:26   01    16-bit source address increment
     SRCSIZE           25:24   01    16-bit source data size
     reserved          23:18   0     Reserved
     ARBSIZE           17:14   0000  Auto mode does not arbitrate
     XFERSIZE          13:4  1024-1 Transfer count items
     NXTUSEBURST       3       0     N/A for this transfer type
     XFERMODE          2:0     010   Use Auto-request transfer mode
      0b 01 01 01 01 000000 0000 1111111111 0 010
      0b 01010101000000000000000000000010
    */
  UDMA->ENASET = BIT30;  // channel30 is enabled.
  UDMA->SWREQ = BIT30;   // software start,
  // bit 30 in UDMA_ENASET_R become clear when done
  // bits 2:0 ucControlTable[CH30+2] become clear when done
  // vector 62, NVIC interrupt 46, vector address 0x0000.00F8 could be armed or �DMA Software interrupt
  while(UDMA->ENASET&BIT30){};
}

