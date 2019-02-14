#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 4096
#define FS 512

FILE *fp;
int val = 0;
float vector[N];
float ventana[N];
float vectorVentana[N];
float vectorAC[N];

void leerArchivo();
void aplicarOffset();
void calcularVentana();
void guardarArchivo();
void graficar();

int main(){
    
    leerArchivo();
    aplicarOffset();
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
        
        //printf("Cxx[%d] = %f \n", n, vectorAC[n]);
    }

    //obtener valor más alto
    float max = 0;
    float aux = 0;
    int flag = 0;
    int pos = 0;

    for(int i = 0; i < N; i++){
        if(vectorAC[i] < 0)
            flag = 1;

        if((vectorAC[i] > max) && flag == 1){
            max = vectorAC[i];
            pos = i;
        }
    }

    printf("---------------------------------------------\n");
    printf("\tValor máximo=%f en i=%d \n", max, pos);
    printf("\tFrecuencia = %f \n", (float)FS/pos);
    printf("\tlpm = %f \n", (float)FS/pos*60);
    printf("---------------------------------------------\n");

    guardarArchivo();
    graficar();

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

void aplicarOffset(){
    for(int i = 0; i<N; i++){
        vector[i] = vector[i] - (float)(4096/2);
    }
}

void calcularVentana(){
    for(int i = 0; i<N; i++){
        ventana[i] = 0.54 - (0.46 * cosf(2*M_PI*i/N));
    }
}

void graficar(){
	FILE *pipe_gp = popen("gnuplot -p", "w");

	fputs("set multiplot \n", pipe_gp);
	fputs("set size .5,.5 \n", pipe_gp);

	fputs("set origin 0,.5 \n", pipe_gp);
  	fputs("plot \"in.dat\" w l \n", pipe_gp);

  	fputs("set origin .5,.5 \n", pipe_gp);
  	fputs("plot \"hamming.dat\" w l \n", pipe_gp);

    fputs("set origin 0,0 \n", pipe_gp);
  	fputs("plot \"ventaneo.dat\" w l \n", pipe_gp);

  	fputs("set origin .5,0 \n", pipe_gp);
  	fputs("plot \"out.dat\" w l \n", pipe_gp);

	pclose(pipe_gp);
}

void guardarArchivo(){
    FILE *ap_arch; 
    
    ap_arch = fopen("in.dat", "w");

    for (register int n = 0; n < N; n++){
		fprintf(ap_arch, "%f\n", vector[n]);
	}

	fclose(ap_arch);

    ap_arch = fopen("hamming.dat", "w");

    for (register int n = 0; n < N; n++){
		fprintf(ap_arch, "%f\n", ventana[n]);
	}
    
	fclose(ap_arch);


    ap_arch = fopen("ventaneo.dat", "w");

    for (register int n = 0; n < N; n++){
		fprintf(ap_arch, "%f\n", vectorVentana[n]);
	}
    
	fclose(ap_arch);

    ap_arch = fopen("out.dat", "w");

    for (register int n = 0; n < N; n++){
		fprintf(ap_arch, "%f\n", vectorAC[n]);
	}
    
	fclose(ap_arch);
}