#include "xc.h"
#include <stdio.h>
#include <libpic30.h>
#include <string.h>
#include <p30F4013.h>


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
//_FGS(CODE_PROT_OFF)
// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCI?N DE DECLARACI?N DE CONSTANTES CON DEFINE								*/
/********************************************************************************/
#define EVER 1
//#define MUESTRAS 64

/********************************************************************************/
/* DECLARACIONES GLOBALES														*/
/********************************************************************************/
/*DECLARACI?N DE LA ISR DEL TIMER 1 USANDO __attribute__						*/
/********************************************************************************/
void __attribute__((__interrupt__)) _U2RXInterrupt( void );

/********************************************************************************/
/* CONSTANTES ALMACENADAS EN EL ESPACIO DE LA MEMORIA DE PROGRAMA				*/
/********************************************************************************/
//int ps_coeff __attribute__ ((aligned (2), space(prog)));
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

//****Comandos AT****
//Para configurar el envío de un mensaje de bienvenida cuando esté operacional
//char CMD_CSGT[] = "1,\"ready\"\r";

//char CMD_CREG[] = "AT+CREG?\r";
char CMD_AT[] = "AT\r";
char CMD_ATE0[] = "ATE0\r";
char CMD_AT_CMGF[] = "AT+CMGF=1\r";
char CMD_AT_CMGS[] = "AT+CMGS=\"+525543612094\"\r";
char CMD_MENSAJE[] = "Temperatura: \x1A\r";

//char respuestaGSM[40];
unsigned char j;
unsigned short int x[480];
unsigned short int y[480];

char count;

void iniPerifericos();  
void iniInterrupciones();
void configurarUART1();
void configurarUART2();

void iniGSM();
void enviarComandoGSM(char comando[]);

void printUART1(char* cadena);
void printUART2(char* cadena);

extern void RETARDO_300ms();
extern void RETARDO_1s();
extern void RETARDO_50us();

int main(void) {
    
    iniPerifericos();
    configurarUART1();
    configurarUART2();
    iniInterrupciones();
    
    printUART1("\r\n inicializando... \r\n");
    
    iniGSM();
    
    RETARDO_1s();
    RETARDO_1s();
    RETARDO_1s();
    RETARDO_1s();
    RETARDO_1s();
    
    //enviarComandoGSM(CMD_AT);
    //enviarComandoGSM(CMD_ATE0);
    //enviarComandoGSM(CMD_AT_CMGF);
    //enviarComandoGSM(CMD_AT_CMGS);
    //enviarComandoGSM(CMD_MENSAJE);
    
    while(EVER){
        Nop();
    }
    
    return 0;
}


void iniPerifericos(){
    PORTD = 0;
    Nop();
    LATD = 0;
    Nop();
    TRISD = 0;
    
    PORTB = 0;
    Nop();
    LATB = 0;
    Nop();
    TRISB = 0;
    Nop();
    ADPCFG = 0xFFFF;
    
    // Para el UART1, transmite a PC
    PORTC=0;
    Nop();
    LATC = 0;
    Nop();
    TRISC = 0;
    Nop();
    TRISCbits.TRISC13=1;    //U1ATX
    Nop();
    TRISCbits.TRISC14=1;    //U1ARX
    Nop();
    
    //Para el UART2, trasmite al módulo 4G
    PORTF = 0;
    Nop();
    LATF = 0;
    Nop();
    TRISF = 0;
    Nop();
    TRISFbits.TRISF4 = 1;    //U2ARX
    Nop();
    TRISFbits.TRISF5 = 1;    //U2ATX
    Nop();
    
    
    //inicialización del mikroBUS2 para el módulo 4G
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
void configurarUART1(){
    U1MODE = 0X0420;    //4 para poner ALTIO en 1: usar UxATX and UxARX I/O pins; 2 para Auto Baud Enable bit
    U1STA = 0X8000;     //8 para UTXISEL: Transmission Interrupt Mode Selection bit; 
                                        //1 = Interrupt when a character is transferred to the Transmit Shift register and as result, the transmit buffer becomes empty
    U1BRG = 11;          //11 para 9600 baudios
}

/******************************************************************************
* CONFIGURACION DEL UART 2. EL UART 2 CONFIGURA EL MODEM GSM PARA ENVIO DE
* COMANDOS AT Y RECEPCION DE RESPUESTAS DEL MODEM
* VELOCIDAD: 9600 BAUDIOS
* TRAMA: 8 BITS X DATO, SIN PARIDAD, 1 BIT DE PARO
******************************************************************************/
void configurarUART2(){
    U2MODE = 0X0020;    //No se utiliza ALTIO
    U2STA = 0X8000;
    U2BRG = 11;     //11 para 9600 baudios
}


void iniInterrupciones(){    
    IFS1bits.U2RXIF=0;    //UART2 Receiver Interrupt Flag Status bit
    IEC1bits.U2RXIE=1;    //UART2 Receiver Interrupt Enable bit (Interrupt request enabled)
    
    //----- Habilitar ------
    
    //Para UART1
	U1MODEbits.UARTEN=1;    //UART1 Enable bit: 1 para habilitar
    U1STAbits.UTXEN=1;      //Transmit Enable bit: 1 para habilitar
    Nop();
    //Para UART2
    U2MODEbits.UARTEN=1;    //UART2 Enable bit: 1 para habilitar
    U2STAbits.UTXEN=1;      //Transmit Enable bit: 1 para habilitar
}


void iniGSM(){
    LATBbits.LATB0 = 1;     //LED
    LATDbits.LATD1 = 1;  //PWK
    RETARDO_1s();
    
    LATDbits.LATD1 = 0;  //PWK
    RETARDO_50us();
    LATDbits.LATD1 = 1;  //PWK
    Nop();
    
    LATBbits.LATB0 = 0;     //LED
}


void enviarComandoGSM(char comando[]){
    IFS1bits.U2TXIF = 0;
    
    printUART1(comando);
    printUART1("\n");
    
    printUART2(comando);
    
    //count = 2;
    //j = 0;
    //Espera respuesta
    //while(count > 0){
//        U1TXREG = 'x';
  //      RETARDO_1s();
    //}   //count disminuye con la interrupci?n U2RXInterrupt
    
}

/********************************************************************************/
/* DESCRICION:	ISR (INTERRUPT SERVICE ROUTINE) DEL TIMER 1						*/
/* LA RUTINA TIENE QUE SER GLOBAL PARA SER UNA ISR								*/	
/* SE USA PUSH.S PARA GUARDAR LOS REGISTROS W0, W1, W2, W3, C, Z, N Y DC EN LOS */
/* REGISTROS SOMBRA																*/
/********************************************************************************/
void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt( void )
{
    LATBbits.LATB0 = 1;     //LED
    char resp;
    
    resp = U2RXREG;
    U1TXREG = resp;
    
    //respuestaGSM[j] = resp;
    j++;
    
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
    
    LATBbits.LATB0 = 0;     //LED
    IFS1bits.U2RXIF = 0;
}

void printUART1(char* cadena) {
    int i;
    for(i = 0; cadena[i] != '\0'; i++) {
        U1TXREG = cadena[i];
        
        //Mientras no se genere la interrupción, va a esperar
        while(!IFS0bits.U1TXIF);
        IFS0bits.U1TXIF = 0;
    }
}

void printUART2(char* cadena) {
    int i;
    for(i = 0; cadena[i] != '\0'; i++) {
        U2TXREG = cadena[i];
        
        //Mientras no se genere la interrupción, va a esperar
        while(!IFS1bits.U2TXIF);
        IFS1bits.U2TXIF = 0;
    }
}
