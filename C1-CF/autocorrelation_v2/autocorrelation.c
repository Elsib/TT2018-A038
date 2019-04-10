/* 
 * V2. Propuesto por el profesor Víctor Hugo
 * 
 * Programa para calcular la autocorrelación.
 * Los datos se ingresan mediante un archivo que contiene 4096 muestras.
 * 
 * Se leen y agregan datos en la posición 0 de x[], posteriormente se realiza
 * el cálculo de la autocorrelación para Cxx[n] += x[0] * x[n].
 * Al finalizar se recorren los datos del arreglo x[] una posición a la derecha.
 * Se realiza la lectura nuevamente.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#define N 4096
#define FS 512

FILE *fp;

int adc = 0;
float valor;
float x[N];
float Cxx[N];

int main(){

    fp = fopen("pruebaPulseSensor512.txt", "r");

    if(fp == NULL){
        perror("Error al abrir el arhivo. \n");
        exit(EXIT_FAILURE);
    }
    
    int i;
    int j;
    float ventana;

    for(i = 0; i < N; i++){

        fscanf(fp, "%d", &adc);
        valor = (float)adc;
        valor -= (float)2048;   //Se aplica el offset a la muestra. Offset de la mitad del rango máximo del ADC (4096).

        ventana = 0.54 - (0.46 * cosf(2*M_PI*i/N)); //Se calcula el coeficiente correspondiente al índice (i) de la ventana de Hamming.
        valor = valor * ventana;
        x[0] = valor;

        for(j = 0; j < N; j++){
            Cxx[j] += x[0] * x[j];
        }

        //Se mueven los datos del arreglo x[] una posición a la derecha
        for(int m = N-2; m >= 0 ; m--){
            x[m+1] = x[m];
        }
    }

    //Busca el valor máximo de la autocorrelación
    float max = 0;
    int pos = 0;
    for(int i = 0; i < N; i++){
        Cxx[i] = Cxx[i]/N;

        printf("Cxx[%d] = %f \n", i, Cxx[i]);

        if(i-2>=0){
            if(Cxx[i-1] > Cxx[i] && Cxx[i-1] > Cxx[i-2] && Cxx[i-1] > 0) {
                max = Cxx[i-1];
                pos = i-1;

                printf("-----------------------------------------------\n");
                printf("\tValor máximo = %f en i=%d\n", max, pos);
                printf("\tFrecuencia = %f \n", (float) FS/pos);
                printf("\tlpm = %f \n", (float) FS/pos*60);
                printf("-----------------------------------------------\n");
                break;
            }
        }
    }

    fclose(fp);

    return 0;
}
