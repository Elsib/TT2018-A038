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
 * 
 * La idea es tener un buffer de 512 muestras en las que se cargarán las 4096 muestras necesarias para realizar los cálculos de las 512 Cxx.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#define muestras 256
#define n 1600
#define FS 128
//#define lpm_esperado 57

FILE *fp;

int adc = 0;
int valor;
float x[n];
long xQ15[n];
float Cxx[n];
long CxxQ15[n];
int lpm_esperado = 90;

int main(){

    int i;
    int j;
    float ventana;
    long ventanaQ15;
    int muestras;

    printf("FS = %d ; lpm_esperados = %d \n", FS, lpm_esperado);
    printf("%-4s \t %-10s \t %-4s \t %-10s \t %-10s \t %-7s \n", "N", "Cxx", "pos", "freq", "lpm", "error %");

    for(muestras = 16; muestras <= 4096; muestras+=16) {

        fp = fopen("../Pruebas/128hz_4096_Carlos_90.txt", "r");

        if(fp == NULL) {
            perror("Error al abrir el arhivo. \n");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i < muestras; i++) {

            fscanf(fp, "%d", &adc);
            valor = adc;
            valor -= 2048;   //Se aplica el offset a la muestra. Offset de la mitad del rango máximo del ADC (4096).

            ventana = 0.54 - (0.46 * cosf(2*M_PI*i/muestras)); //Se calcula el coeficiente correspondiente al índice (i) de la ventana de Hamming.
            ventanaQ15 = ventana * 32768;   //2^15

            x[0] = valor * ventana;

            xQ15[0] = valor * ventanaQ15;
            xQ15[0] = xQ15[0] >> 15;

            for(j = 0; j < n; j++) {
                Cxx[j] += x[0] * x[j];
                CxxQ15[j] += xQ15[0] * xQ15[j];
            }

            //Se mueven los datos del arreglo x[] una posición a la derecha
            for(int m = n-2; m >= 0 ; m--) {
                x[m+1] = x[m];
                xQ15[m+1] = xQ15[m];
            }
        }

        //Busca el valor máximo de la autocorrelación
        float max = 0;
        int pos = 0;
        float freq = 0;
        int lpm = 0;
        float error = 0;
        
        for(int i = 0; i < n; i++) {
            Cxx[i] = Cxx[i]/muestras;

            //printf("Cxx[%d] = %f \n", i, Cxx[i]);

            if(i-2 >= 0) {
                if(Cxx[i-1] > Cxx[i] && Cxx[i-1] > Cxx[i-2] && Cxx[i-1] > 0) {
                    max = Cxx[i-1];
                    pos = i-1;

                    // printf("-----------------------------------------------\n");
                    // printf("\tValor máximo = %f en i=%d\n", max, pos);
                    // printf("\tFrecuencia = %f \n", (float) FS/pos);
                    // printf("\tlpm = %f \n", (float) FS/pos*60);
                    // printf("-----------------------------------------------\n");
                    freq = (float) FS/pos;
                    lpm = freq*60;

                    error = (float) (lpm-lpm_esperado)/lpm_esperado*100;

                    if(error < 0) {
                        error *= -1;
                    }

                    printf("%-4d \t %-10f \t %-4d \t %-10f \t %-10d \t %-7f \n", muestras, max, pos, freq, lpm, error);
                    break;
                }
            }
        }

        for(int i = 0; i < n; i++) {
            CxxQ15[i] = CxxQ15[i]/muestras;

            //printf("Cxx[%d] = %f \n", i, Cxx[i]);

            if(i-2 >= 0) {
                if(CxxQ15[i-1] > CxxQ15[i] && CxxQ15[i-1] > CxxQ15[i-2] && CxxQ15[i-1] > 0) {
                    max = CxxQ15[i-1];
                    pos = i-1;

                    // printf("-------------------Q15-------------------------\n");
                    // printf("\tValor máximo = %f en i=%d\n", max, pos);
                    // printf("\tFrecuencia = %f \n", (float) FS/pos);
                    // printf("\tlpm = %f \n", (float) FS/pos*60);
                    // printf("-----------------------------------------------\n");
                    freq = (float) FS/pos;
                    lpm = freq*60;

                    error = (float) (lpm-lpm_esperado)/lpm_esperado*100;

                    if(error < 0) {
                        error *= -1;
                    }

                    printf("%-4d \t %-10f \t %-4d \t %-10f \t %-10d \t %-7f   :Q15: \n", muestras, max, pos, freq, lpm, error);
                    break;
                }
            }
        }

        fclose(fp);
    }

    return 0;
}
