#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 4096

FILE *fp;
int val = 0;
float vector[N];
float ventana[N];
float vectorVentana[N];
float vectorAC[N];

void leerArchivo();
void calcularVentana();

int main(){
    
    leerArchivo();
    calcularVentana();
    
    //ventaneo
    for(int i = 0; i < N; i++){
        vectorVentana[i] = vector[i] * ventana[i];
    }

    //autocorrelación
    float precalc = 0;
    for(int n = 0; n < N; n++){
        precalc = 0;

        for(int m = 0; m <= N-1-n; m++){
            precalc += vectorVentana[m] * vectorVentana[m+n];
        }

        vectorAC[n] = precalc/(float)N;
        
        printf("Cxx[%d] = %f \n", n, vectorAC[n]);
    }

    return 0;
}

void leerArchivo(){
    fp = fopen("pruebaPulseSensor512.txt", "r");

    if(fp == NULL){
        perror("Error al abrir el arhivo. \n");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while((fscanf(fp, "%d", &val)) != EOF){
        printf("%d \n", val);
        vector[i] = (float)val;
        i++;
    }

    fclose(fp);
}

void calcularVentana(){
    for(int i = 0; i<N; i++){
        ventana[i] = 0.54 - (0.46 * cosf(2*M_PI*i/N));
    }
}

