/**@brief: Este programa muestra los bloques de un 
 * programa en C embebido para el DSPIC, los bloques son:
 * BLOQUE 1. OPCIONES DE CONFIGURACION DEL DSC: OSCILADOR, WATCHDOG,
 * BROWN OUT RESET, POWER ON RESET Y CODIGO DE PROTECCION
 * BLOQUE 2. EQUIVALENCIAS Y DECLARACIONES GLOBALES
 * BLOQUE 3. ESPACIOS DE MEMORIA: PROGRAMA, DATOS X, DATOS Y, DATOS NEAR
 * BLOQUE 4. CÓDIGO DE APLICACIÓN
 * @device: DSPIC30F4013
 * @oscillator: FRC, 7.3728MHz
 */
#include <xc.h>
#include <stdio.h>
#include <math.h>

/********************************************************************************/
/* 						BITS DE CONFIGURACIÓN									*/	
/********************************************************************************/
/* SE DESACTIVA EL CLOCK SWITCHING Y EL FAIL-SAFE CLOCK MONITOR (FSCM) Y SE 	*/
/* ACTIVA EL OSCILADOR INTERNO (FAST RC) PARA TRABAJAR							*/
/* FSCM: PERMITE AL DISPOSITIVO CONTINUAR OPERANDO AUN CUANDO OCURRA UNA FALLA 	*/
/* EN EL OSCILADOR. CUANDO OCURRE UNA FALLA EN EL OSCILADOR SE GENERA UNA 		*/
/* TRAMPA Y SE CAMBIA EL RELOJ AL OSCILADOR FRC  								*/
/********************************************************************************/
//_FOSC(CSW_FSCM_OFF & FRC); 
#pragma config FOSFPR = FRC             // Oscillator (Internal Fast RC (No change to Primary Osc Mode bits))
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)/********************************************************************************/
/* SE DESACTIVA EL WATCHDOG														*/
/********************************************************************************/
//_FWDT(WDT_OFF); 
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)
/********************************************************************************/
/* SE ACTIVA EL POWER ON RESET (POR), BROWN OUT RESET (BOR), 					*/	
/* POWER UP TIMER (PWRT) Y EL MASTER CLEAR (MCLR)								*/
/* POR: AL MOMENTO DE ALIMENTAR EL DSPIC OCURRE UN RESET CUANDO EL VOLTAJE DE 	*/	
/* ALIMENTACIÓN ALCANZA UN VOLTAJE DE UMBRAL (VPOR), EL CUAL ES 1.85V			*/
/* BOR: ESTE MODULO GENERA UN RESET CUANDO EL VOLTAJE DE ALIMENTACIÓN DECAE		*/
/* POR DEBAJO DE UN CIERTO UMBRAL ESTABLECIDO (2.7V) 							*/
/* PWRT: MANTIENE AL DSPIC EN RESET POR UN CIERTO TIEMPO ESTABLECIDO, ESTO 		*/
/* AYUDA A ASEGURAR QUE EL VOLTAJE DE ALIMENTACIÓN SE HA ESTABILIZADO (16ms) 	*/
/********************************************************************************/
//_FBORPOR( PBOR_ON & BORV27 & PWRT_16 & MCLR_EN ); 
// FBORPOR
#pragma config FPWRT  = PWRT_16          // POR Timer Value (16ms)
#pragma config BODENV = BORV20           // Brown Out Voltage (2.7V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)
/********************************************************************************/
/*SE DESACTIVA EL CÓDIGO DE PROTECCIÓN											*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCIÓN DE DECLARACIÓN DE CONSTANTES CON DEFINE								*/
/********************************************************************************/
#define float double

#define muestras 1024
#define n 36
#define FS 16
#define PI 3.1415926535897931159979634685441851615905761718750 /* double */

/********************************************************************************/
/* DECLARACIONES GLOBALES														*/
/********************************************************************************/
/*DECLARACIÓN DE LA ISR DEL TIMER 1 USANDO __attribute__						*/
/********************************************************************************/
void __attribute__((__interrupt__)) _T3Interrupt( void );

/********************************************************************************/
/* CONSTANTES ALMACENADAS EN EL ESPACIO DE LA MEMORIA DE PROGRAMA				*/
/********************************************************************************/
int ps_coeff __attribute__ ((aligned (2), space(prog)));
/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO X DE LA MEMORIA DE DATOS			*/
/********************************************************************************/
//int x_input[MUESTRAS] __attribute__ ((space(xmemory)));
/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO Y DE LA MEMORIA DE DATOS			*/
/********************************************************************************/
//int y_input[MUESTRAS] __attribute__ ((space(ymemory)));
/********************************************************************************/
/* VARIABLES NO INICIALIZADAS LA MEMORIA DE DATOS CERCANA (NEAR), LOCALIZADA	*/
/* EN LOS PRIMEROS 8KB DE RAM													*/
/********************************************************************************/

void iniPerifericos();
void iniInterrupciones();
void configurarUART1();
void configurarADC();
void configurarT3();

float alpha = 2*PI/muestras;
float valor;
float x[n];
float Cxx[n];

int main() {
    iniPerifericos();
    configurarUART1();
    configurarADC();
    configurarT3();
    iniInterrupciones();
    
    printf("Iniciando... \n\r");
    
    int i;
    int j;
    int m;
    float ventana;
    
    for(i = 0; i < muestras; i++) {
        while (!IFS0bits.ADIF); //Mientras no termine la conversión va a esperar
        
        valor = (float)ADCBUF0;
        valor -= (float)2048;
        ventana = 0.54 - (0.46 * cosf(alpha*i));
        valor *= ventana;
        
        x[0] = valor;
        printf("Muestra %d: %f \n\r", i, valor);
        
        for(j = 0; j < n; j++){
            Cxx[j] += x[0] * x[j];
        }
            
        //Se mueven los datos del arreglo x[] una posición a la derecha
        for(m = n-2; m >= 0 ; m--){
            x[m+1] = x[m];
        }
        
        IFS0bits.ADIF = 0;
    }
    
    //Busca el valor máximo de la autocorrelación
    float max = 0;
    int pos = 0;
    
    for(i = 0; i < n; i++){
        Cxx[i] = Cxx[i]/muestras;

        printf("Cxx[%d] = %f \n\r", i, Cxx[i]);

        if(i-2 >= 0){
            if(Cxx[i-1] > Cxx[i] && Cxx[i-1] > Cxx[i-2] && Cxx[i-1] > 0) {
                max = Cxx[i-1];
                pos = i-1;

                printf("-----------------------------------------------\n\r");
                printf("\tValor máximo = %f en i=%d\n\r", max, pos);
                printf("\tFrecuencia = %f \n\r", (float) FS/pos);
                printf("\tlpm = %f \n\r", (float) FS/pos*60);
                printf("-----------------------------------------------\n\r");
                break;
            }
        }
    }
    
    while(1) {
        Nop();
    }
    
    return 0;
}


/****************************************************************************/
/* DESCRICION:	ESTA RUTINA INICIALIZA LAS INTERRPCIONES    				*/
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO															*/
/****************************************************************************/
void iniInterrupciones() {
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    
    IFS0bits.ADIF = 0;
    //IEC0bits.ADIE = 1;    No descomentar
    
    //Para UART1
	U1MODEbits.UARTEN=1;    //UART1 Enable bit: 1 para habilitar
    U1STAbits.UTXEN=1;      //Transmit Enable bit: 1 para habilitar
    
    //Encender el timer 3
    T3CONbits.TON = 1;
    
    //Encender el ADC
    ADCON1bits.ADON = 1;
    //ADCON1bits.ASAM = 1;
}

void configurarUART1() {
    //Inicializar el UART1
    U1MODE = 0X0420;
    U1STA = 0X8000;
    U1BRG = 11;  //9600 baudios
}

void configurarADC() {
    ADPCFG = 0xFFFB;
    ADCSSL = 0x0000;
    ADCHS = 0x0002;
    ADCON1 = 0x0044; //Puede ser 0x0040 pero después hay que poner ASAM = 1
    ADCON2 = 0x0000;
    ADCON3 = 0x0000;
}

void configurarT3() {
    TMR3 = 0x0000;
    PR3 = 0x3840;     //Frecuencia de 16Hz
    T3CON = 0x0010;
}

/****************************************************************************/
/* DESCRICION:	ESTA RUTINA INICIALIZA LOS PERIFERICOS						*/
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO															*/
/****************************************************************************/
void iniPerifericos() {
    PORTB = 0;
    Nop();
    LATB = 0;
    Nop();
    TRISB = 0;
    Nop();  
    TRISBbits.TRISB2=1; //AN2 : PMOD1
    Nop();
    
    // Para el UART1, transmite a PC
    PORTC=0;
    Nop();
    TRISCbits.TRISC13=0;    //U1ATX
    Nop();
    TRISCbits.TRISC14=1;    //U1ARX
    Nop();   
}
    
    
    
/********************************************************************************/
/* DESCRIPCIÓN:	ISR (INTERRUPT SERVICE ROUTINE) DEL TIMER 3						*/
/* LA RUTINA TIENE QUE SER GLOBAL PARA SER UNA ISR								*/	
/* SE USA PUSH.S PARA GUARDAR LOS REGISTROS W0, W1, W2, W3, C, Z, N Y DC EN LOS */
/* REGISTROS SOMBRA																*/
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt( void ) {
        IFS0bits.T3IF = 0;    //SE LIMPIA LA BANDERA DE INTERRUPCION DEL TIMER 3
}
