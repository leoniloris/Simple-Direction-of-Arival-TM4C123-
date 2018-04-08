/*
 * rls.h
 *
 *  Created on: 25 de mai de 2017
 *      Author: leoni
 */
#include "double_adc.h"
#include "Nokia5110.h"

#ifndef RLS_H_
#define RLS_H_
#define l 0.2f
#define Fs 8000.0f
#define c 340.29f
#define mu 0.1f             //convergence rate
#define P 16             //order of filter
#define DELAY (int)(l*Fs/c)*2

void lms(float *input,float *destination,float *W);
void correlation(float *input1,float *input2,float *corr);


#endif /* RLS_H_ */
