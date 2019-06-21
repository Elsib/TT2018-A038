#include <xc.h>
#include <stdio.h>
#include <libpic30.h>
#include <math.h>
#include <string.h>
#include "hamming.h"


//_FOSC(CSW_FSCM_OFF & FRC);
#pragma config FOSFPR = FRC             // Oscillator (Internal Fast RC (No change to Primary Osc Mode bits))
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)
/********************************************************************************/
/* SE DESACTIVA EL WATCHDOG														*/
//_FWDT(WDT_OFF); 
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)
/********************************************************************************/
//_FBORPOR( PBOR_ON & BORV27 & PWRT_16 & MCLR_EN ); 
// FBORPOR
#pragma config FPWRT  = PWRT_16          // POR Timer Value (16ms)
#pragma config BODENV = BORV20           // Brown Out Voltage (2.7V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)
/********************************************************************************/
/*SE DESACTIVA EL C?DIGO DE PROTECCI?N											*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);
// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCION DE DECLARACION DE CONSTANTES CON DEFINE								*/
/********************************************************************************/
#define float double

#define muestras 1600
#define n 150
#define FS 128

#define minutos 1

/********************************************************************************/
/* DECLARACION DE ISRs                                   						*/
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt( void );
void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt( void );
void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void);

/********************************************************************************/
/* COMANDOS Y VARIABLES PARA EL MODULO 4G                                       */
/********************************************************************************/
char CMD_AT[] = "AT\r";
char CMD_ATE0[] = "ATE0\r";
char CMD_AT_CMGF[] = "AT+CMGF=1\r";
char CMD_AT_CCLK[] = "AT+CCLK?\r";
char CMD_AT_CMGS[] = "AT+CMGS=\"+525543612094\"\r";   //elsi
//char CMD_AT_CMGS[] = "AT+CMGS=\"+525532768660\"\r"; //elena
//char CMD_AT_CMGS[] = "AT+CMGS=\"+525564211272\"\r"; //carlos
//char CMD_AT_CMGS[] = "AT+CMGS=\"+525519705183\"\r";   //katia
//char CMD_AT_CMGS[] = "AT+CMGS=\"+525531475618\"\r";   //naye
//char CMD_AT_CMGS[] = "AT+CMGS=\"+525518036525\"\r";   //profesora odette

char CMD_MSG[42];

char *comandos[] = {
	CMD_AT,
	CMD_ATE0,
	CMD_AT_CMGF,
    CMD_AT_CCLK,
    CMD_AT_CMGS,
    CMD_MSG
};

char respuestaGSM[40];
unsigned char j;
char count;

/********************************************************************************/
/* VARIABLES PARA AUTOCORRELACION												*/
/********************************************************************************/
int valor;
long x[n];
long Cxx[n];
int lpm;
int index_muestras;

/********************************************************************************/
/* VARIABLES PARA TEMPERATURA													*/
/********************************************************************************/
unsigned short int temp_msb;
unsigned short int temp_lsb;
float temperatura;

/********************************************************************************/
/* VARIABLES DE UMBRALES      													*/
/********************************************************************************/
unsigned char lpm_max = 100;
unsigned char lpm_min = 60;
float temp_max = 37.0;
float temp_min = 36.0;

/********************************************************************************/
/* BANDERAS DE CONTROL          												*/
/********************************************************************************/
unsigned char flag_auto = 0;
unsigned char flag_temp = 0;
int timer = minutos*60*FS;
unsigned int periodo = 1843200/8/FS;
int i;

/********************************************************************************/
/* DECLARACION DE FUNCIONES                                                     */
/********************************************************************************/

/*          CONFIGURACION DEL DSPIC           */
void iniPerifericos();
void iniInterrupciones();
void configurarADC();
void configurarI2C();
void configurarTimer3();
void configurarUART1();
void configurarUART2();

/*      COMUNICACION CON EL MODULO MAX232     */
unsigned char comunicacionMAX();
extern void START_I2C();
extern void ENVIA_DATO_I2C(unsigned char dato);
extern void RESTART_I2C();
extern unsigned short int RECIBE_DATO_I2C();
extern void ACK_MST_I2C();
extern void NACK_MST_I2C();
extern void STOP_I2C();

/*        COMUNICACION CON EL MODULO 4G       */
void ini4G();
unsigned char enviarComando(char comando[]);

/*               GENERALES                    */
extern void RETARDO_50us();
extern void RETARDO_300ms();
extern void RETARDO_1s();

void printUART1(char* cadena);
void printUART2(char* cadena);
void resetTimer();
void clearArray();

int main(void) {
    iniPerifericos();
    configurarUART1();
    configurarUART2();
    configurarTimer3();
    configurarADC();
    configurarI2C();
    iniInterrupciones();
    
    while(1) {
        clearArray();

        printf("\n\r iniciando... \n\r");
        printf("timer: %d \n\r", timer);

        //Espera hasta completar el numero de muestras requeridas
        while(index_muestras < muestras);
        index_muestras = 0;
        ADCON1bits.ADON = 0;    //Apaga el ADC para evitar la interrupción mientras se realiza el envio el SMS
        IEC0bits.ADIE = 0;
        //Busca el valor maximo de la autocorrelacion
        int pos = 0;

        for(i = 0; i < n; i++) {
            Cxx[i] = Cxx[i]/muestras;

            //printf("Cxx[%d] = %ld \n\r", i, Cxx[i]);

            if(i-2 >= 0) {
                if(Cxx[i-1] > Cxx[i] && Cxx[i-1] > Cxx[i-2] && Cxx[i-1] > 0) {
                    pos = i-1;
                    float freq = (float) FS/pos;
                    lpm = freq*60;

                    printf("-----------------------------------------------\n\r");
                    printf("\tValor maximo = %ld en i = %d \n\r", Cxx[pos], pos);
                    printf("\tFrecuencia = %f \n\r", freq);
                    printf("\tlpm = %d \n\r", lpm);
                    printf("-----------------------------------------------\n\r");

                    flag_auto = 1;
                    break;
                }
            }
        }

        //Si encuentra la frecuencia fundamental continua...
        if(flag_auto) {
            flag_temp = comunicacionMAX();
            //flag_temp = 1;

            //Si recibio el valor de temperatura continua...
            if(flag_temp) {
                temperatura += temp_msb;
                float pb = (float) 0.00390625 * temp_lsb;
                temperatura += pb;

                printf("temp: %f \n\r", temperatura);

                //si pasa algun umbral o el tiempo se agota, continua para enviar el SMS
                if((temperatura < temp_min) || (temperatura > temp_max) || (lpm < lpm_min) || (lpm > lpm_max) || (timer <= 0)) {
                    unsigned char index_comando = 0;
                    unsigned char try = 1;
                    char date[16];

                    while(index_comando <= 5) {
                        if(try) {
                            ini4G();
                            RETARDO_1s();
                            RETARDO_1s();
                            RETARDO_1s();
                            RETARDO_1s();
                            RETARDO_1s();
                        }

                        if(enviarComando(comandos[index_comando])) {
                            try = 0;

                            if(index_comando == 3) {
                                date[0] = '2';
                                date[1] = '0';

                                int o = 0;
                                while (o <= 13) {
                                    date[o+2] = respuestaGSM[7+o];
                                    o++;
                                }

                                date[10] = ' ';
                            } else if(index_comando == 4) {
                                sprintf(comandos[5], "lpm:%d,temp:%.2f,fecha:%s \x1A\r", lpm, temperatura, date);
                            }

                            index_comando++;
                        } else {
                            try = 1;
                            index_comando = 0;
                        }
                    }

                    resetTimer();
                } else {
                    printf("No se cumple la condicion de envio \n");
                }

            } else {
                printf("ERROR: I2C \n");
            }
        } else {
            printf("ERROR: Autocorrelacion \n");
        }

        //reset flags
        flag_auto = 0;
        flag_temp = 0;
        ADCON1bits.ADON = 1;    //Enciende nuevamente el ADC
        IEC0bits.ADIE = 1;
        temperatura = 0;
    }
    
    return 0;
}

/****************************************************************************/
/* FUNCION PARA INICIALIZAR LOS PERIFERICOS DEL DSPIC						*/
/****************************************************************************/
void iniPerifericos() {
    PORTB = 0;
    Nop();
    LATB = 0;
    Nop();
    TRISB = 0;
    Nop();
    
    PORTC = 0;
    Nop();
    LATC = 0;
    Nop();
    TRISC = 0;
    Nop();
    
    PORTD = 0;
    Nop();
    LATD = 0;
    Nop();
    TRISD = 0;
    Nop();
    
    PORTF = 0;
    Nop();
    LATF = 0;
    Nop();
    TRISF = 0;
    Nop();
    
    TRISBbits.TRISB2=1;     //AN2 : PMOD1
    Nop();
    
    // Para el UART1, transmite a PC
    TRISCbits.TRISC13=0;    //U1ATX
    Nop();
    TRISCbits.TRISC14=1;    //U1ARX
    Nop();
    
    //Para I2C en mikroBUS1
    TRISFbits.TRISF3 = 0;   //RF3, se configura como salida para el SCL
    Nop();
    TRISFbits.TRISF2 = 1;   //RF2, se configura como entrada para el SDA
    Nop();
    
    //Para UART2 (4G) en mikroBUS2
    TRISFbits.TRISF4 = 1;    //U2ARX
    Nop();
    TRISFbits.TRISF5 = 0;    //U2ATX
    Nop();
    
    //inicializaci?n del mikroBUS2 para el m?dulo 4G
    TRISBbits.TRISB5 = 1;   //STA
    TRISDbits.TRISD1 = 0;   //PWK
    TRISBbits.TRISB8 = 0;   //RTS
    TRISDbits.TRISD3 = 1;   //RI
    TRISDbits.TRISD9 = 1;   //CTS
}

/******************************************************************************/
/* CONFIGURACION DEL UART 1. EL UART 1 CONFIGURA EL FT232 PARA ENVIO DE DATOS */
/* A LA PC                                                                    */
/* TRAMA: 8 BITS X DATO, SIN PARIDAD, 1 BIT DE PARO                           */
/******************************************************************************/
void configurarUART1() {
    U1MODE = 0X0420;    //4 para poner ALTIO en 1: usar UxATX and UxARX I/O pins; 2 para Auto Baud Enable bit
    U1STA = 0X8000;     //8 para UTXISEL: Transmission Interrupt Mode Selection bit; 
                                        //1 = Interrupt when a character is transferred to the Transmit Shift register and as result, the transmit buffer becomes empty
    //U1BRG = 11;          //9600 baudios
    U1BRG = 5;          //19200 baudios
}

/******************************************************************************
* CONFIGURACION DEL UART 2. EL UART 2 CONFIGURA EL MODEM GSM PARA ENVIO DE
* COMANDOS AT Y RECEPCION DE RESPUESTAS DEL MODEM
* TRAMA: 8 BITS X DATO, SIN PARIDAD, 1 BIT DE PARO
******************************************************************************/
void configurarUART2() {
    U2MODE = 0X0020;    //No se utiliza ALTIO
    U2STA = 0X8000;
    //U2BRG = 11;     //9600 baudios
    U2BRG = 5;     //19200 baudios
}

/********************************************************************************/
/* CONFIGURACION DE LA INTERFAZ I2C PARA EL MAX232                              */
/********************************************************************************/
void configurarI2C() {
    I2CBRG = 2;     //Configura la velocidad de transmision a 400KHZ soportado por el MAX30205
}

/********************************************************************************/
/* CONFIGURACION DEL ADC                                                        */
/********************************************************************************/
void configurarADC() {
    ADPCFG = 0xFFFB;      //;1 Analog Input in Digital Mode; 0 Analog Input in Analog Mode
    //ADPCFGbits.PCFG2 = 0;
    ADCSSL = 0x0000;    //Skip ANx for input scan
    ADCHS = 0x0002;     //2 para AN3 como entrada del canal 0
    
    ADCON1 = 0x0044;    //4 para SSRC seleccionar el Timer3; 4 para ASAM: Sampling begins immediately after last conversion completes.
    ADCON2 = 0x0000;    //
    ADCON3 = 0x0000;    //Era 0x0F02: F para 10000 = 16TAD, 2 para ADCS (A/D Conversion Clock Select bits): 000010 = TCY/2*(ADCS<5:0> + 1) = TCY/2
}

/********************************************************************************/
/* CONFIGURACION DEL TIMER3 DE ACUERDO A LA FRECUENCIA DE MUESTREO INDICADA     */
/********************************************************************************/
void configurarTimer3() {
    TMR3 = 0x0000;         //Inicializa el registro. ?ste guarda la Most Significant Word del valor de 32bits del timer
    //PR3=0x0E10;     //Valor que se compara con el contador para lanzar la interrupci?n. Establece la frecuencia de 512Hz
    //T3CON=0x000;    //Contiene el preescalador en <5:4> con TCKPS<1:0> con 1:1, 1:8, 1:64, 1:256
    //PR3 = 0x3840;     //Frecuencia de 16Hz
    PR3 = periodo;
    T3CON = 0x010;
}

/********************************************************************************/
/* CONFIGURACION DE LAS INTERRUPCIONES Y HABILITACION DE LAS INTERFACES DE COMUNICACION */
/********************************************************************************/
void iniInterrupciones() {
    IFS0bits.T3IF=0;    //Timer3 Interrupt Flag Status bit
    IEC0bits.T3IE=1;    //Timer3 Interrupt Enable bit (Interrupt request enabled)
    
    IFS0bits.ADIF=0;    //A/D Conversion Complete Interrupt Flag Status bit
    IEC0bits.ADIE=1;    //A/D Conversion Complete Interrupt Enable bit (Interrupt request enabled)
    
    IFS1bits.U2RXIF=0;    //UART2 Receiver Interrupt Flag Status bit
    IEC1bits.U2RXIE=1;    //UART2 Receiver Interrupt Enable bit (Interrupt request enabled)
    
    //----- Habilitar ------
    T3CONbits.TON=1;    //Activar el Timer3
    
    //Para UART1
	U1MODEbits.UARTEN=1;    //UART1 Enable bit: 1 para habilitar
    U1STAbits.UTXEN=1;      //Transmit Enable bit: 1 para habilitar
    
    //Para UART2
    U2MODEbits.UARTEN=1;    //UART2 Enable bit: 1 para habilitar
    U2STAbits.UTXEN=1;      //Transmit Enable bit: 1 para habilitar
    
    //Para ADC
    ADCON1bits.ADON=1;      //A/D Operating Mode bit: 1 para activar
    
    //Para I2C
    I2CCONbits.I2CEN = 1;   //I2C Enable bit: 1 para habilitar y configurar SDA y SCL como puertos seriales
}

/****************************************************************************/
/* ESTA RUTINA REALIZA LA COMUNICACION CON EL MAX30205 PARA OBTENER LA TEMPERATURA EN 16 BITS */
/****************************************************************************/
unsigned char comunicacionMAX() {
    printf("cominicacionMAX \n");
    temp_msb = 0;
    temp_lsb = 0;

    START_I2C();
    
    ENVIA_DATO_I2C(0X90);   //direccci?n del sensor+RW
    
    if(I2CSTATbits.ACKSTAT == 1) { //Preguntando por un ACK
        return 0;   //NANCK
    }
    
    ENVIA_DATO_I2C(0X00);   //selecci?n del registro de temperatura
    
    if(I2CSTATbits.ACKSTAT == 1) { //Preguntando por un ACK
        return 0;   //NANCK
    }
    
    RESTART_I2C();

    ENVIA_DATO_I2C(0X91);   //direccci?n del sensor+RW
    
    if(I2CSTATbits.ACKSTAT == 1) { //Preguntando por un ACK
        return 0;   //NANCK
    }
    
    temp_msb = RECIBE_DATO_I2C();  //Recibe MSB
    
    ACK_MST_I2C();  //Genera ACK
    
    temp_lsb = RECIBE_DATO_I2C();  //Recibe LSB
    
    NACK_MST_I2C();  //Genera NACK
    
    STOP_I2C();
    
    //printf("temp: %d.%d \n", temp_msb, temp_lsb);
    
    return 1;
}

/********************************************************************************/
/* FUNCION PARA REALIZAR LA INICIALIZACION DEL MODULO 4G                        */
/********************************************************************************/
void ini4G() {
    printf("ini4G \n");
    LATDbits.LATD1 = 1;  //PWK
    RETARDO_1s();
    
    LATDbits.LATD1 = 0;  //PWK
    RETARDO_50us();
    LATDbits.LATD1 = 1;  //PWK
    Nop();
}

/********************************************************************************/
/* FUNCION PARA ENVIAR COMANDOS AT AL MODULO 4G Y ESPERAR LA RESPUESTA  		*/
/********************************************************************************/
unsigned char enviarComando(char comando[]) {
    IFS1bits.U2TXIF = 0;
    
    printUART1(comando);
    printUART1("\n");
    
    printUART2(comando);
    
    char try = 0;
    count = 2;
    j = 0;
    
    //Espera respuesta del modulo 4G
    while(count > 0){   //count disminuye con la ISR U2RXInterrupt
        U1TXREG = 'x';
        RETARDO_1s();
        try++;
        
        if(try > 4) {   //Si no hay respuesta en 5 seg devuelve un 0 
            return 0;
        }
    }
    
    return 1;
}

/********************************************************************************/
/* ISR DEL UART 2                                          						*/
/* Esta rutina cuenta los caracteres <LF> recibidos para determinar la respuesta*/
/* del modulo 4G                                                                */
/* En general la respuesta del m?dulo GSM es: <CR><LF>OK<CR><LF>                */
/* Para el mensaje: <CR><LF><greater_than><space>                               */
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt(void) {
    char resp;

    resp = U2RXREG;
    U1TXREG = resp;
    
    if(resp == 13){     //<CR>
        resp = '.';
    }
    else if(resp == 10){     //<LF>
        resp = '_';
        count--;
    }
    else if(resp == 62){    //'>'
        count--;
    }
    else if(resp == 32){   //' '
        resp = '-';
    }
    else {
        respuestaGSM[j] = resp;
        j++;
    }
    
    IFS1bits.U2RXIF = 0;
}

/********************************************************************************/
/* ISR DEL ADC EN DONDE SE REALIZA EL PROCESAMIENTO DE LOS VALORES DIGITALES    */
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void) {
    valor = ADCBUF0;
    valor -= 2048;
    
    x[0] = valor * ventana[index_muestras];
    x[0] = x[0] >> 15;      //para regresar el punto flotante a su lugar original modificado por la ventana
    
    //printf("Muestra %d: %d \n\r", index_muestras, valor);
    int i;
    //autocorrelacion
    for(i = 0; i < n; i++) {
        Cxx[i] += x[0] * x[i];
    }
    
    //Se mueven los datos del arreglo x[] una posicion a la derecha
    for(i = n-2; i >= 0 ; i--) {
        x[i+1] = x[i];
    }
    
    index_muestras++;
    IFS0bits.ADIF = 0;
}

/********************************************************************************/
/* ISR DEL TIMER3 EN DONDE SE DECREMENTA UN CONTADOR PARA EL ENVIO DEL SMS		*/
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt( void ) {
    timer--;
    IFS0bits.T3IF = 0;
}

/********************************************************************************/
/* FUNCION PARA ENVIAR UNA CADENA AL UART1                              		*/
/********************************************************************************/
void printUART1(char* cadena) {
    int i;
    for(i = 0; cadena[i] != '\0'; i++) {
        U1TXREG = cadena[i];
        
        //Mientras no se genere la interrupción, va a esperar
        while(!IFS0bits.U1TXIF);
        IFS0bits.U1TXIF = 0;
    }
}

/********************************************************************************/
/* FUNCION PARA ENVIAR UNA CADENA AL UART2                              		*/
/********************************************************************************/
void printUART2(char* cadena) {
    int i;
    for(i = 0; cadena[i] != '\0'; i++) {
        U2TXREG = cadena[i];
        
        //Mientras no se genere la interrupción, va a esperar
        while(!IFS1bits.U2TXIF);
        IFS1bits.U2TXIF = 0;
    }
}

/********************************************************************************/
/* FUNCION PARA ESTABLECER EL VALOR DEL CONTADOR PARA EL ENVIO DEL SMS    		*/
/********************************************************************************/
void resetTimer() {
    timer = minutos*60*FS;
}

void clearArray() {
    int i;
    for(i = 0; i < n; i++) {
        Cxx[i] = 0;
        x[i] = 0;
    }
}