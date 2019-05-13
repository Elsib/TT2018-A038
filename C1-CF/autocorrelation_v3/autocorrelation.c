#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 512
#define FS 512

FILE *fp;

int adc = 0;
float valor;
float x[N];
float Cxx_result[3];
float Cxx;

int main(){

    fp = fopen("pruebaPulseSensor512.txt", "r");

    if(fp == NULL){
        perror("Error al abrir el arhivo. \n");
        exit(EXIT_FAILURE);
    }

    int i;
    int n;
    int m;
    int j = 0;
    float ventana;
    
    i = 0;
    while(fscanf(fp, "%d", &adc)!=EOF){
        valor = (float)adc;
        valor -= (float)2048;
        ventana = 0.54 - (0.46 * cosf(2*M_PI*i/N));
        valor = valor * ventana;

        x[i] = valor;
        i++;

        if(i > N-1)
            break;
    }
    
    Cxx_result[0] = 0;
    Cxx_result[1] = 0;
    Cxx_result[2] = 0;

    for(n = 0; n < N; n++){

        Cxx = 0;
        for(m = 0; m <= N-1-n; m++){
            Cxx += x[m] * x[m+n];
        }

        Cxx = Cxx/N;

        printf("Cxx[%d] = %f \n", n, Cxx);

        Cxx_result[0] = Cxx_result[1];
        Cxx_result[1] = Cxx_result[2];
        Cxx_result[2] = Cxx;
        j++;

        if(j>2){
            if(Cxx_result[1] > Cxx_result[0] && Cxx_result[1] > Cxx_result[2] && Cxx_result[1] > 0){
                printf("-----------------------------------------------\n");
                printf("\tValor máximo=%f en i=%d \n", Cxx_result[1], n-1);
                printf("\tFrecuencia = %f \n", (float)FS/(n-1));
                printf("\tlpm = %f \n", (float)FS/(n-1)*60);
                printf("-----------------------------------------------\n");
                break;
            }
        }
    }

    fclose(fp);

    return 0;
}
