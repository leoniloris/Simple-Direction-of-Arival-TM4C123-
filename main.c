#include <Nokia5110.h>
#include "TM4C123GH6PM.h"
#include "system_tm4c123.h"
#include "pt.h"
#include "PLL.h"
#include "double_adc.h"
#include "rls.h"
#include "Nokia5110.h"
#include "math.h"
uint8_t repeat=0;
float h[P],corr[P],angle=0,max=0;
int ind=0;
static volatile uint8_t processing = 1;
/* Tasks */
struct pt pt_process, pt_display;// Protothreads context
static PT_THREAD(process(struct pt *pt)) {
    PT_BEGIN(pt);
    while( 1 ) {// wait until the buffer is full so I can begin processing it
        PT_WAIT_UNTIL(pt,(write_ind==(N_samples-1)));
        processing = 1;
        DMA_Transfer();// begin transferring data to auxiliary buffer
        float max1=0,max2=0;
        uint32_t sum1=0,sum2=0;
        for(int i=0;i<N_samples;i++){
            sum1+=X_proc[i];
            sum2+=X_proc[i+N_samples];
            max1=(X_proc[i]>max1)?X_proc[i]:max1;
            max2=(X_proc[i+N_samples]>max2)?X_proc[i+N_samples]:max2;
        }
        for(int i=0;i<N_samples;i++){ // normalize
            X_float[i] = ((float)(X_proc[i] - (uint16_t)(sum1/N_samples)))/max1;
            X_float[i+N_samples] = ((float)(X_proc[i+N_samples] - (uint16_t)(sum2/N_samples)))/max2;
        }

        lms(&X_float[0],&X_float[N_samples],&h[0]);
        maxvector(&h[0],&max,&ind);

//        correlation(&X_float[0],&X_float[N_samples],&corr[0]);
//        maxvector(&corr[0],&max,&ind);

//        int indd=0,check;
//        double maxx=0;
//        for(int i=(DELAY)/2;i<(3*(DELAY+1))/2;i++){
//            check=(double)corr[i]*(double)corr[i] > maxx*maxx  ;
//            maxx=(check)?corr[i]:maxx;
//            indd=(check)?i:indd;
//        }
//        double valcos=(((double)(DELAY-ind)*(double)c/(double)Fs)/(double)l);
//        angle = (float)((double)90-(double)180*acos( valcos>1.0?1.0:(valcos<-1.0?-1.0:valcos)     )/(double)3.14159265359);


        processing = 0;
        repeat++;
    }
    PT_END(pt);
}

static PT_THREAD(display(struct pt *pt)) {

    PT_BEGIN(pt);
    while(1) {
    PT_WAIT_UNTIL(pt,(processing==0)&&(repeat>1));
    repeat=0;
    Nokia5110_Init();
    Nokia5110_ClearBuffer();

    float y1=0,y2=0;
    int ind1=0,next_pixel=0,current_pixel=0;
    h[P-9]=max;
    if(max!=0)
    for(int i=0;i<P-1;i++){
        current_pixel=(int) (    ((float)(MAX_X-1))*(float)(i) / (float)P      );
        next_pixel=(int) (    ((float)(MAX_X-1))*(float)(i+1) / (float)P      );
        y1=(h[i]/max);y1=y1<0?-y1:y1;
        y2=(h[i+1]/max);y2=y2<0?-y2:y2;

//        y1=(corr[i]/max);y1=y1<0?-y1:y1;
//        y2=(corr[i+1]/max);y2=y2<0?-y2:y2;

        for(int pixel=0;pixel<(next_pixel-current_pixel)-1;pixel++){

            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)pixel/((float)(next_pixel-current_pixel)))),current_pixel+pixel );
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.1)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.2)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.3)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.4)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.5)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel+1));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.6)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel+1));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.7)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel+1));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.8)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel+1));
            Nokia5110_SetPxl((int)(47-47*(y1+(y2-y1)*(float)(pixel+0.9)/((float)(next_pixel-current_pixel)))),current_pixel+(pixel+1));
        }
    }
    Nokia5110_DisplayBuffer();
    Nokia5110_SetCursor(0, 0);
//    if(angle<0){Nokia5110_OutChar('-');Nokia5110_SetCursor(1, 0);Nokia5110_OutUDec((uint16_t)(-1.0*angle));}
//    else Nokia5110_OutUDec((uint16_t)angle);
    angle=31  ;
    Nokia5110_OutUDec((uint16_t)angle);

    processing = 1;
    Double_ADC_init();
    uDMA_init();
    }
    PT_END(pt);
}
int main(void) {
    PLL_Init(Bus80MHz);
    Double_ADC_init();
    uDMA_init();

    for(int i=0;i<P;i++){h[i]=0.0;corr[i]=0.0;}
    PT_INIT(&pt_process);
    PT_INIT(&pt_display);

    for (;;) {
        PT_SCHEDULE(process(&pt_process));
        PT_SCHEDULE(display(&pt_display));
    }
}
