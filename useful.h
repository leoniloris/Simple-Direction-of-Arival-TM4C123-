/*
 * useful.h
 *
 *  Created on: 25 de mai de 2017
 *      Author: leoni
 */

#ifndef USEFUL_H_
#define USEFUL_H_
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })
//TENHO QUE COLOCAR O RESULT NOS ARGUMENTOS
void matrixXmatrix(float *M1,float *M2,float *resultado){
    int j,k;
    float sum=0.0;
    //int TILE_WIDTH=16;
    for(int i= 0;i<P; i++){
            for(j =0;j<P;j++){
                for(k =0;k<P;k++){
                    sum+=M1[i*P+k]*M2[k*P+j];
                }
                resultado[i*P+j]=sum;
                sum=0.0;
            }
    }


//    for(int a=0;a<P*P;a++) resultado[a]=0.0;
//        for ( int i=0; i<P; i+=TILE_WIDTH )
//            for (int j=0; j<P; j+=TILE_WIDTH )
//                for (int k=0; k<P; k+=TILE_WIDTH )
//                    for (int y=i; y<i+TILE_WIDTH; y++)
//                        for (int x=j; x=j+TILE_WIDTH; x++)
//                            for (int z=k; z<k+TILE_WIDTH;z++)
//                                resultado[y*P+x] += M1[y*P+z]*M2[z*P+x];






}

void vectorXmatrix(float *V1,float *M1,float *resultado){
    float sum=0.0;

    int i,j;
    for(i= 0;i<P; i++){
        for(j =0;j<P;j++){
            sum+= V1[j]*M1[j*P+i];
        }
        resultado[i]=sum;
        sum=0.0;
    }
}
void matrixXvector(float *M1,float *V1,float *resultado){
    float sum=0.0;


    int i,j;
    for(i= 0;i<P; i++){
        for(j =0;j<P;j++){
            sum+= M1[i*P+j]*V1[j];
        }
        resultado[i]=sum;
        sum=0.0;
    }


}
float vectorXvector(float *V1,float *V2){
    float sum=0.0;

    int j;
    for(j =0;j<P;j++){
        sum+= V1[j]*V1[j];
    }

    return sum;
}
void vectorDOTvector(float *V1,float *V2,float *resultado){
    int i,j;
    for(i=0;i<P;i++)
        for(j =0;j<P;j++)
            resultado[i*P+j] = V1[i]*V2[j];

}
void maxvector(float *h,float *m,int* index){
    int ind=0,check;
    float max=0;
    for(int i=0;i<P;i++){
        check=(double)h[i]*(double)h[i] > max*max  ;
        max=(check)?h[i]:max;
        ind=(check)?i:ind;
    }
    *m = max;
    *index =ind;
}


#endif /* USEFUL_H_ */
