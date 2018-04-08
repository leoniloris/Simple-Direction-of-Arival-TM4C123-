/*
 * rls.c
 *
 *  Created on: 25 de mai de 2017
 *      Author: leoni
 */
#include "rls.h"
#include "useful.h"
static float input_delayed[N_samples];

void lms(float *input,float *destination,float *W){
    int a=0,aaaaaa;
    float y;
    float error;
    float squared_norm;
    for(int i=DELAY;i<N_samples;i++)
        input_delayed[i]=input[i-DELAY]; // displace input avoiding negative delays

    for(int ind=P-1;ind<N_samples;ind+=1){


        y=0.0;
        squared_norm=0.0;
        for(int i=ind;i>ind-P;i--)
            y+=W[i-a]*input_delayed[i];// calculate output

        error=y-destination[ind];

        for(int i=0;i<N_samples;i++)
            squared_norm+=input_delayed[i]*input_delayed[i];

        for(int i=ind;i>ind-P;i--)
            W[i-a]=W[i-a] - mu *error * input_delayed[i]/(squared_norm+0.0001);

        a+=1;
    }
}









void correlation(float *input1,float *input2,float *corr){
    float sum;
    for(int ind=-P/2;ind<0;ind++){ // fist half
        sum=0.0;
        //input1[MAX_X/2:end]'*input2[1:MAX_X/2]
        for(int indind=0;indind<N_samples+ind;indind++)
            sum+=input2[indind]*input1[indind-ind];
        corr[ind+P/2]=0.5*corr[ind+P/2]+sum;
    }
    for(int ind=0;ind<P/2;ind++){ // second half
        sum=0.0;
        for(int indind=0;indind<N_samples-ind;indind++)
            sum+=input1[indind]*input2[indind-ind];
        corr[ind+P/2]=0.5*corr[ind+P/2]+sum;
    }


}
