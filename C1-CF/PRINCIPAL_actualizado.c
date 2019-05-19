#include "xc.h"
#include <stdio.h>
#include <libpic30.h>
#include <math.h>
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
/* SECCI?N DE DECLARACI?N DE CONSTANTES CON DEFINE								*/
/********************************************************************************/
#define float double
#define PI 3.1415926535897931159979634685441851615905761718750 /* double */
#define muestras 1600
#define n 10
#define FS 128

//__attribute__ ((aligned (2), space(prog)));

/********************************************************************************/
/*DECLARACI?N DE LA ISR DEL TIMER 1 USANDO __attribute__						*/
/********************************************************************************/
void __attribute__((__interrupt__)) _T3Interrupt(void);
void __attribute__((__interrupt__)) _U2RXInterrupt(void);
void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void);


/********************************************************************************/
/* Para el m?dulo 4G    														*/
/********************************************************************************/
char CMD_AT[] = "AT\r";
char CMD_ATE0[] = "ATE0\r";
char CMD_AT_CMGF[] = "AT+CMGF=1\r";
char CMD_AT_CMGS[] = "AT+CMGS=\"+525543612094\"\r";
char CMD_AT_CCLK[] = "AT+CCLK?\r";
char CMD_MSG[60];

char respuestaGSM[40];
unsigned char j;
char count;
char value = 0;

/********************************************************************************/
/* Banderas de control          												*/
/********************************************************************************/
unsigned char flag_auto = 0;
unsigned char flag_temp = 0;
int timer = 7680;   //completar esa cantidad de ciclos para haber tenido aproximadamente 60 seg

/********************************************************************************/
/* Variables para autocorrelaci?n												*/
/********************************************************************************/
//float alpha = 2*PI/muestras;
long x[n];
long Cxx[n];
int index_muestras = 0;
int index_autocor;
int index_recorrer;
int p_ant;
int p_act;
int aux;
int valor;
int lpm;
float max = 0;
int pos = 0;
unsigned char lpm_max = 90;
unsigned char lpm_min = 70;

/********************************************************************************/
/* Varibles para temperatura                                                    */
/********************************************************************************/
unsigned short int temp_msb;
unsigned short int temp_lsb;
float temperatura;
float temp_max = 36.8;
float temp_min = 36.3;

/********************************************************************************/
/* Funciones            														*/
/********************************************************************************/
void iniPerifericos();
void iniInterrupciones();
void configurarADC();
void configurarI2C();
void configurarTimer3();
void configurarUART1();
void configurarUART2();

unsigned char comunicacionMAX();

void iniGSM();
void enviarComandoGSM(char comando[]);

extern void START_I2C();
extern void ENVIA_DATO_I2C(unsigned char dato);
extern void RESTART_I2C();
extern unsigned short int RECIBE_DATO_I2C();
extern void ACK_MST_I2C();
extern void NACK_MST_I2C();
extern void STOP_I2C();
extern void WREG_INIT();

extern void RETARDO_50us();
extern void RETARDO_300ms();
extern void RETARDO_1s();

void printUART1(char* cadena);

int main(void) {

    iniPerifericos();
    configurarUART1();
    configurarUART2();
    configurarTimer3();
    configurarADC();
    configurarI2C();
    iniInterrupciones();
    
    printf("\n\r Iniciando... \n\r");
    
    while(index_muestras < muestras);
    
    ADCON1bits.ADON = 0;
    
    //Busca el valor m�ximo de la autocorrelaci�n
    int i;
    for(i = 0; i < n; i++) {
        //Cxx[i] = Cxx[i] / muestras;

        printf("Cxx[%d] = %ld \n\r", i, Cxx[i]);

        if(i-2 >= 0) {
            if(Cxx[i-1] > Cxx[i] && Cxx[i-1] > Cxx[i-2] && Cxx[i-1] > 0) {
                max = Cxx[i-1];
                pos = i-1;

                printf("-----------------------------------------------\n\r");
                printf("\tValor m?ximo = %f en i=%d\n\r", max, pos);
                printf("\tFrecuencia = %f \n\r", (float) FS/pos);
                printf("\tlpm = %f \n\r", (float) FS/pos*60);
                printf("-----------------------------------------------\n\r");
                
                lpm = FS/pos*60;
                flag_auto = 1;
                break;
            }
        }
    }
    
    if(flag_auto) {
        flag_temp = comunicacionMAX();
        
        if(flag_temp) {
            temperatura = 0.0;
            temperatura += temp_msb;
            float pb = 0.00390625 * temp_lsb;
            temperatura += pb;
            
            iniGSM();

            enviarComandoGSM(CMD_AT);
            enviarComandoGSM(CMD_ATE0);
            enviarComandoGSM(CMD_AT_CMGF);
            //enviarComandoGSM(CMD_AT_CCLK);
            
            //+CCLK: "14/07/01,15:00:00+01"
//            char date[15];
//            int o = 0;
//            while (o <= 13) {
//                date[o] = respuestaGSM[10+o];
//                o++;
//            }
             
//            sprintf(CMD_MSG, "lpm: %d \n\rTemperatura: %.2f \n\rFecha: %s \x1A\r", lpm, temperatura, date);
            sprintf(CMD_MSG, "lpm: %d \n\rTemperatura: %.2f \n\r \x1A\r", lpm, temperatura);

            enviarComandoGSM(CMD_AT_CMGS);
            enviarComandoGSM(CMD_MSG);
            
            
//            if((timer <= 0) || (lpm <= lpm_min) || (lpm >= lpm_max) || (temperatura <= temp_min) || (temperatura >= temp_max)) {
//                timer = 7680;
//                
//            }
        }
        else {
            printf("ERROR: I2C");
        }
    }
    else {
        printf("ERROR: Autocorrelacion");
    }
    
    ADCON1bits.ADON = 1;
    flag_auto = 0;
    flag_temp = 0;
    
    return 0;
}


/****************************************************************************/
/* DESCRICION:	ESTA RUTINA INICIALIZA LOS PERIFERICOS						*/
/* PARAMETROS:  NINGUNO                                                     */
/* RETORNO:     NINGUNO                                                     */
/*                                                                          */
/*  TRISx:  Registro de control que determina si el pin asociado al puerto es una entrada o salida -> 0 (Output) ; 1 (Input)
 *          -Est?n configurados como entrada despu?s de un reset-
 * 
 *  PORTx:  Para leer o escribir valores en los pines de un puerto
 * 
 *  LATx:   Elimina los problemas que podr?an ocurrir con las instrucciones de lectura-modificaci?n-escritura:
 *          Una escritura en el registro PORTX escribe el valor de los datos en el cierre (latch) del puerto.
            Una escritura en el registro LATx escribe el valor de los datos en el cierre (latch) del puerto.
            Una lectura del registro PORTx lee el valor de los datos en el pin de I/O.
            Una lectura del registro LATx lee el valor de los datos retenidos en el cierre (latch) del puerto.
****************************************************************************/
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

/******************************************************************************
* CONFIGURACION DEL UART 1. EL UART 1 CONFIGURA EL FT232 PARA ENVIO DE DATOS A LA PC
* VELOCIDAD: 19200 BAUDIOS
* TRAMA: 8 BITS X DATO, SIN PARIDAD, 1 BIT DE PARO
******************************************************************************/
void configurarUART1() {
    U1MODE = 0X0420;    //4 para poner ALTIO en 1: usar UxATX and UxARX I/O pins; 2 para Auto Baud Enable bit
    U1STA = 0X8000;     //8 para UTXISEL: Transmission Interrupt Mode Selection bit; 
                                        //1 = Interrupt when a character is transferred to the Transmit Shift register and as result, the transmit buffer becomes empty
    //U1BRG = 11;          //9600 baudios
    U1BRG = 5;     //19200 baudios
}

/******************************************************************************
* CONFIGURACION DEL UART 2. EL UART 2 CONFIGURA EL MODEM GSM PARA ENVIO DE
* COMANDOS AT Y RECEPCION DE RESPUESTAS DEL MODEM
* VELOCIDAD: 9600 BAUDIOS
* TRAMA: 8 BITS X DATO, SIN PARIDAD, 1 BIT DE PARO
******************************************************************************/
void configurarUART2() {
    U2MODE = 0X0020;    //No se utiliza ALTIO
    U2STA = 0X8000;
    //U2BRG = 11;     //9600 baudios
    U2BRG = 5;     //19200 baudios
}

void configurarI2C() {
    I2CBRG = 2;     //Configura la velocidad de transmisi?n a 400KHZ soportado por el MAX30205
}

void configurarADC() {
    ADPCFG = 0xFFFB;      //;1 Analog Input in Digital Mode; 0 Analog Input in Analog Mode
    //ADPCFGbits.PCFG2 = 0;
    ADCSSL = 0x0000;    //Skip ANx for input scan
    ADCHS = 0x0002;     //2 para AN3 como entrada del canal 0
    
    ADCON1 = 0x0044;    //4 para SSRC seleccionar el Timer3; 4 para ASAM: Sampling begins immediately after last conversion completes.
    ADCON2 = 0x0000;    //
    ADCON3 = 0x0000;    //Era 0x0F02: F para 10000 = 16TAD, 2 para ADCS (A/D Conversion Clock Select bits): 000010 = TCY/2*(ADCS<5:0> + 1) = TCY/2
}

void configurarTimer3() {
    PR3 = 0x0708;//Frecuencia de 128Hz
    TMR3 = 0;
    T3CON = 0x0010;
}

//Habilitar comunicaciones cuando se inicializan las interrupciones o cuando se configura?
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
/* DESCRICION:	ESTA RUTINA REALIZA LA COMUNICACI?N CON EL MAX30205 PARA OBTENER LA TEMPERATURA ENVIADA EN 16 BITS    */
/* PARAMETROS:  NINGUNO                                                     */
/* RETORNO:     EXITO O NANCK                                               */
/****************************************************************************/
unsigned char comunicacionMAX() {
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
    
    printf("Temp: %d.%d", temp_msb, temp_lsb);
    
    return 1;
}


void iniGSM() {
    LATDbits.LATD1 = 1;  //PWK
    RETARDO_1s();
    
    LATDbits.LATD1 = 0;  //PWK
    RETARDO_50us();
    LATDbits.LATD1 = 1;  //PWK
    Nop();
    RETARDO_1s();
    RETARDO_1s();
    RETARDO_1s();
    RETARDO_1s();
    RETARDO_1s();
}


void enviarComandoGSM(char comando[]) {
    count = 2;
    j = 0;
    IFS1bits.U2TXIF = 0;
    
    __C30_UART=1;
    printf("\r\n");
    printf(comando);
    printf("\r\n");
    
    __C30_UART=2;
    printf(comando);
    
    //Espera respuesta
    while(count > 0){   //Disminuye con la interrupci?n U2RXInterrupt
        U1TXREG = 'x';
        RETARDO_1s();
    }
    
}


/********************************************************************************/
/* DESCRICION:	ISR (INTERRUPT SERVICE ROUTINE) DEL UART 2						*/
/* Esta rutina cuenta los caracteres <LF> recibidos para determinar la respuesta*/
/*                                                                              */
/* En general la respuesta del m?dulo GSM es: <CR><LF>OK<CR><LF>                */
/* Para el mensaje: <CR><LF><greater_than><space>                               */
/********************************************************************************/

void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt(void) {
    char resp;

    resp = U2RXREG;
    respuestaGSM[j] = resp;
    
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

    U1TXREG = resp;
    //U1TXREG = count+48; //Para ver el car?cter
    
    j++;
    
    IFS1bits.U2RXIF = 0;
}

/********************************************************************************/
/* DESCRIPCI?N:	ISR (INTERRUPT SERVICE ROUTINE) DEL TIMER 3						*/
/* LA RUTINA TIENE QUE SER GLOBAL PARA SER UNA ISR								*/	
/* SE USA PUSH.S PARA GUARDAR LOS REGISTROS W0, W1, W2, W3, C, Z, N Y DC EN LOS */
/* REGISTROS SOMBRA																*/
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void) {
    value = !value;
    LATFbits.LATF6 = value;
    timer--;    //cada 7.8125ms
    
    IFS0bits.T3IF = 0;    //SE LIMPIA LA BANDERA DE INTERRUPCION DEL TIMER 3
}



void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void){
    valor = ADCBUF0;
    valor -= 2048;

    x[0] = valor * ventana[index_muestras];
    x[0] = x[0] >> 15;      //para regresar el punto flotante a su lugar original modificado por la ventana
    printf("Muestra %d: %d \n\r", index_muestras, valor);

    //autocorrelaci�n
    for(index_autocor = 0; index_autocor < n; index_autocor++) {
        Cxx[index_autocor] += x[0] * x[index_autocor];
    }

    //Se mueven los datos del arreglo x[] una posici�n a la derecha
    for(index_recorrer = n-2; index_recorrer >= 0 ; index_recorrer--) {
        x[index_recorrer+1] = x[index_recorrer];
    }
    
    index_muestras++;

    IFS0bits.ADIF = 0;
}

//void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void){
//    valor = ADCBUF0;
//    valor -= 2048;
//
//    x[p_act] = valor * ventana[index_muestras];
//    //printf("Muestra %d: %d \n\r", index_muestras, valor);
//
//    p_ant = p_act;
//    
//    //autocorrelaci�n
//    for(aux = 0; aux < n; aux++) {
//        Cxx[aux] += x[p_act] * x[p_ant];
//        p_ant = (n-1+p_act)%n;
//    }
//    
//    p_act = (p_act+1)%n;
//    
//    index_muestras++;   //se verifica desde fuera de la interrupci�n
//
//    IFS0bits.ADIF = 0;
//}

void printUART1(char* cadena) {
    int i;
    for(i = 0; cadena[i] != '\0'; i++) {
        U1TXREG = cadena[i];
        
        //Mientras no se genere la interrupci?n, va a esperar
        while(!IFS0bits.U1TXIF);
        IFS0bits.U1TXIF = 0;
    }
}